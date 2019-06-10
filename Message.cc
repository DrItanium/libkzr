/**
 * @file
 * Routines to convert between raw byte streams and Requests/Responses
 * @copyright
 * libkzr
 * Copyright (c) 2019, Joshua Scoggins 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Message.h"
#include "Exception.h"

namespace kzr {

void
Message::decode(uint8_t& out) {
    char temporaryStorage;
    _storage.get(temporaryStorage);
    out = temporaryStorage;
}

void
Message::encode(uint8_t value) {
    _storage.put(value);
}

void
Message::decode(uint16_t& out) {
    char temporaryStorage[2];
    _storage.read(temporaryStorage, 2);
    out = (uint16_t(temporaryStorage[1]) << 8) | (uint16_t(temporaryStorage[0]));
}

void
Message::encode(uint16_t value) {
    _storage.put(uint8_t(value));
    _storage.put(uint8_t(value >> 8));
}

void
Message::decode(uint32_t& out) {
    char temporaryStorage[4];
    _storage.read(temporaryStorage, 4);
    out = (uint32_t(temporaryStorage[3]) << 24) |
        (uint32_t(temporaryStorage[2]) << 16) | 
        (uint32_t(temporaryStorage[1]) << 8) | 
        (uint32_t(temporaryStorage[0]));
}

void
Message::encode(uint32_t value) {
    _storage.put(uint8_t(value));
    _storage.put(uint8_t(value >> 8));
    _storage.put(uint8_t(value >> 16));
    _storage.put(uint8_t(value >> 24));
}

void
Message::decode(uint64_t& out) {
    uint32_t temporaryStorage[2];
    decode(temporaryStorage[0]);
    decode(temporaryStorage[1]);
    out = (uint64_t(temporaryStorage[1]) << 32) | 
        (uint64_t(temporaryStorage[0]));
}

void
Message::encode(uint64_t value) {
    encode(uint32_t(value));
    encode(uint32_t(value >> 32));
}

void
Message::decode(std::string& data) {
    uint16_t len;
    decode(len);
    data.reserve(len);
    _storage.read(data.data(), len);
}
void
Message::encode(const std::string& value) {
    if (uint16_t len = value.length(); len != value.length()) {
        throw kzr::Exception("Attempted to encode a string of ", value.length(), " characters when ", ((decltype(len))-1), " is the maximum allowed!");
    } else {
        encode(len);
        for (const auto& c : value) {
            encode(uint8_t(c));
        }
    }
}


Action::Action(Operation op, uint16_t tag) : _op(op), _tag(tag) { }
Action::Action(Operation op) : Action(op, -1) { }
void 
Action::encode(Message& msg) const {
    msg << _op << _tag;
}
void 
Action::decode(Message& msg) {
    msg >> _op >> _tag;
}

void
VersionAction::encode(Message& msg) const {
    Parent::encode(msg);
    msg << _msize << _version;
}

void
VersionAction::decode(Message& msg) {
    Parent::decode(msg);
    msg >> _msize >> _version;
}

std::string
Message::str() const { 
    return _storage.str(); 
}
void Message::str(const std::string& input) { _storage.str(input); }
void Message::reset() { _storage.str(""); }
Message& 
Message::operator<<(const std::string& value) {
    encode(value);
    return *this;
}
Message& 
Message::operator>>(std::string& value) {
    decode(value);
    return *this;
}
#define X(type) \
        Message& \
        Message::operator<<(type data) { \
            encode(data); \
            return *this;  \
        } \
        Message& \
        Message::operator>>(type & data ) { \
            decode(data); \
            return *this; \
        }
        X(uint8_t);
        X(uint16_t);
        X(uint32_t);
        X(uint64_t);
#undef X

void 
Message::write(const std::stringstream::char_type* s, std::streamsize count) {
    _storage.write(s, count);
    if (_storage.bad()) {
        throw kzr::Exception("bad bit set during write!");
    }
}
void
Message::write(const std::string& str) {
    write(str.c_str(), str.length());
}
auto 
Message::read(std::stringstream::char_type* s, std::streamsize count) {
    _storage.read(s, count);
    if (_storage.fail() || _storage.eof()) {
        auto total = _storage.gcount();
        _storage.clear(); // clear flags right now
        return total;
    } else {
        return count;
    }
}
auto 
Message::read(std::string& str) {
    return read(str.data(), str.length());
}

ErrorResponse::ErrorResponse() : Parent(Operation::RError) { }
ErrorResponse::ErrorResponse(uint16_t tag) : Parent(Operation::RError, tag) { }

void 
ErrorResponse::encode(Message& msg) const {
    Parent::encode(msg);
    msg << _ename;
}
void
ErrorResponse::decode(Message& msg) {
    Parent::decode(msg);
    msg >> _ename;
}

void
Qid::encode(Message& msg) const {
    msg << _type << _version << _path;
}

void
Qid::decode(Message& msg) {
    msg >> _type >> _version >> _path;
}

Qid::Qid(uint8_t t, uint64_t path, uint32_t version) : _type(t), _version(version), _path(path) { }


} // end namespace kzr
kzr::Message&
operator>>(kzr::Message& msg, std::set<std::string>& collec) {
    uint16_t len;
    msg >> len;
    for (auto i = 0; i < len; ++i) {
        std::string temporary;
        msg >> temporary;
        collec.emplace(temporary);
    }
    return msg;
}
kzr::Message&
operator<<(kzr::Message& msg, const std::set<std::string>& collec) {
    if (uint16_t len = collec.size(); len != collec.size()) {
        throw kzr::Exception("Attempted to encode a std::set<std::string> of ", collec.size(), " elements when ", ((decltype(len))-1), " is the maximum allowed!");
    } else {
        msg << len;
        for (const auto& c : collec) {
            msg << c;
        }
        return msg;
    }
}

kzr::Message&
operator>>(kzr::Message& msg, std::vector<std::string>& collec) {
    uint16_t len;
    msg >> len;
    for (auto i = 0; i < len; ++i) {
        std::string temporary;
        msg >> temporary;
        collec.emplace_back(temporary);
    }
    return msg;
}
kzr::Message&
operator<<(kzr::Message& msg, const std::vector<std::string>& collec) {
    if (uint16_t len = collec.size(); len != collec.size()) {
        throw kzr::Exception("Attempted to encode a std::vector<std::string> of ", collec.size(), " elements when ", ((decltype(len))-1), " is the maximum allowed!");
    } else {
        msg << len;
        for (const auto& c : collec) {
            msg << c;
        }
        return msg;
    }
}
kzr::Message&
operator>>(kzr::Message& msg, std::list<std::string>& collec) {
    uint16_t len;
    msg >> len;
    for (auto i = 0; i < len; ++i) {
        std::string temporary;
        msg >> temporary;
        collec.emplace_back(temporary);
    }
    return msg;
}
kzr::Message&
operator<<(kzr::Message& msg, const std::list<std::string>& collec) {
    if (uint16_t len = collec.size(); len != collec.size()) {
        throw kzr::Exception("Attempted to encode a std::list<std::string> of ", collec.size(), " elements when ", ((decltype(len))-1), " is the maximum allowed!");
    } else {
        msg << len;
        for (const auto& c : collec) {
            msg << c;
        }
        return msg;
    }
}
