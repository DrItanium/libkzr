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
    out = build(uint8_t(temporaryStorage[0]), uint8_t(temporaryStorage[1]));
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
    out = build(temporaryStorage[0], temporaryStorage[1], temporaryStorage[2], temporaryStorage[3]);
}

void
Message::encode(uint32_t value) {
    encode(uint16_t(value));
    encode(uint16_t(value >> 16));
}

void
Message::decode(uint64_t& out) {
    out = build(decode<uint32_t>(), decode<uint32_t>());
}

void
Message::encode(uint64_t value) {
    encode(uint32_t(value));
    encode(uint32_t(value >> 32));
}

void
Message::decode(std::string& data) {
    auto len = decode<uint16_t>();
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


ActionHeader::ActionHeader(Operation op, uint16_t tag) : _op(op), _tag(tag) { }
ActionHeader::ActionHeader(Operation op) : ActionHeader(op, -1) { }
void 
ActionHeader::encode(Message& msg) const {
    msg << _op << _tag;
}
void 
ActionHeader::decode(Message& msg) {
    msg >> _op >> _tag;
}

void
VersionRequest::encode(Message& msg) const {
    Parent::encode(msg);
    VersionBody::encode(msg);
}
void
VersionRequest::decode(Message& msg) {
    Parent::decode(msg);
    VersionBody::decode(msg);
}

void
VersionResponse::encode(Message& msg) const {
    Parent::encode(msg);
    VersionBody::encode(msg);
}
void
VersionResponse::decode(Message& msg) {
    Parent::decode(msg);
    VersionBody::decode(msg);
}
void
VersionBody::encode(Message& msg) const {
    msg << _msize << _version;
}

void
VersionBody::decode(Message& msg) {
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

void
Stat::encode(Message& msg) const {
    // need to construct the inner message and then tack the stat onto the front
    Message innerMessage;
    innerMessage << _type << _dev;
    HasQid::encode(innerMessage);
    innerMessage << _mode
        << _atime
        << _mtime
        << _length;
    HasName::encode(innerMessage);
    innerMessage << _uid
        << _gid
        << _muid;
    auto innerString = innerMessage.str(); 
    // we then just send this string to the outer message as it will get its length automatically
    // computed as part of the encoding process
    msg << innerString;
}

void
Stat::decode(Message& msg) {
    uint16_t len;
    msg >> len
        >> _type
        >> _dev;
    HasQid::decode(msg);
    msg >> _mode
        >> _atime
        >> _mtime
        >> _length;
    HasName::decode(msg);
    msg >> _uid
        >> _gid
        >> _muid;
}

void
AuthenticationRequest::encode(Message& msg) const {
    Parent::encode(msg);
    msg << _afid << _uname << _aname;
}

void
AuthenticationRequest::decode(Message& msg) {
    Parent::decode(msg);
    msg >> _afid >> _uname >> _aname;
}

void
AuthenticationResponse::encode(Message& msg) const {
    Parent::encode(msg);
    HasQid::encode(msg);
}
void
AuthenticationResponse::decode(Message& msg) {
    Parent::decode(msg);
    HasQid::decode(msg);
}
void 
HasQid::encode(Message& msg) const { msg << _qid; }
void 
HasQid::decode(Message& msg) { msg >> _qid; }
void 
HasFid::encode(Message& msg) const { msg << _fid; }
void 
HasFid::decode(Message& msg) { msg >> _fid; }

void
FlushRequest::encode(Message& msg) const {
    Parent::encode(msg);
    msg << _oldtag;
}
void
FlushRequest::decode(Message& msg) {
    Parent::decode(msg);
    msg >> _oldtag;
}

void
AttachRequest::encode(Message& msg) const {
    Parent::encode(msg);
    HasFid::encode(msg);
    msg << _afid << _uname << _aname;
}
void
AttachRequest::decode(Message& msg) {
    Parent::decode(msg);
    HasFid::decode(msg);
    msg >> _afid >> _uname >> _aname;
}

void
AttachResponse::encode(Message& msg) const {
    Parent::encode(msg);
    HasQid::encode(msg);
}
void
AttachResponse::decode(Message& msg) {
    Parent::decode(msg);
    HasQid::decode(msg);
}

void
WalkRequest::encode(Message& msg) const {
    Parent::encode(msg);
    HasFid::encode(msg);
    msg << _newfid << _wname;
}

void
WalkRequest::decode(Message& msg) {
    Parent::decode(msg);
    HasFid::decode(msg);
    msg >> _newfid >> _wname;
}

void
WalkResponse::encode(Message& msg) const {
    Parent::encode(msg);
    msg << _wqid;
}

void
WalkResponse::decode(Message& msg) {
    Parent::decode(msg);
    msg >> _wqid;
}

std::optional<uint8_t>
Message::peek() noexcept {
    if (auto result = _storage.peek(); result != _storage.eof()) {
        return std::optional<uint8_t>(uint8_t(result));
    } else {
        return std::nullopt;
    }
}

void
OpenRequest::encode(Message& msg) const {
    Parent::encode(msg);
    HasFid::encode(msg);
    msg << _mode;
}

void
OpenRequest::decode(Message& msg) {
    Parent::decode(msg);
    HasFid::decode(msg);
    msg >> _mode;
}

void
CreateRequest::encode(Message& msg) const {
    Parent::encode(msg);
    HasFid::encode(msg);
    HasName::encode(msg);
    msg << _perm << _mode;
}

void
CreateRequest::decode(Message& msg) {
    Parent::decode(msg);
    HasFid::decode(msg);
    HasName::decode(msg);
    msg >> _perm >> _mode;
}

void
ReadRequest::encode(Message& msg) const {
    Parent::encode(msg);
    HasCount::encode(msg);
}

void
ReadRequest::decode(Message& msg) {
    Parent::decode(msg);
    HasCount::decode(msg);
}

void
ReadResponse::encode(Message& msg) const {
    Parent::encode(msg);
    HasDataStorage::encode(msg);
}

void
ReadResponse::decode(Message& msg) {
    Parent::decode(msg);
    HasDataStorage::decode(msg);
}

void
WriteRequest::encode(Message& msg) const {
    Parent::encode(msg);
    HasDataStorage::encode(msg);
}

void
WriteRequest::decode(Message& msg) {
    Parent::decode(msg);
    HasDataStorage::decode(msg);
}

void
WriteResponse::encode(Message& msg) const {
    Parent::encode(msg);
    HasCount::encode(msg);
}

void
WriteResponse::decode(Message& msg) {
    Parent::decode(msg);
    HasCount::decode(msg);
}

void
StatResponse::encode(Message& msg) const {
    Parent::encode(msg);
    msg << _data;
}

void
StatResponse::decode(Message& msg) {
    Parent::decode(msg);
    msg >> _data;
}

void
WStatRequest::encode(Message& msg) const {
    Parent::encode(msg);
    HasFid::encode(msg);
    msg << _stat;
}
void
WStatRequest::decode(Message& msg) {
    Parent::decode(msg);
    HasFid::decode(msg);
    msg >> _stat;
}

void
HasDataStorage::encode(Message& msg) const {
    if (uint32_t len = size(); len != size()) {
        throw Exception("data storage too large for transmission");
    } else {
        msg << len;
        for (auto i : _data) {
            msg << i;
        }
    }
}

void
HasDataStorage::decode(Message& msg) {
    uint32_t size;
    msg >> size;
    _data.reserve(size);
    for (decltype(size) i = 0; i < size; ++i) {
        _data.emplace_back(msg.decode<uint8_t>());
    }
}

void
HasName::encode(Message& msg) const {
    msg << _name; 
}

void
HasName::decode(Message& msg) {
    msg >> _name; 
}

void
HasCount::encode(Message& msg) const {
    msg << _count; 
}

void
HasCount::decode(Message& msg) {
    msg >> _count; 
}

void
HasOffset::encode(Message& msg) const {
    msg << _offset; 
}

void
HasOffset::decode(Message& msg) {
    msg >> _offset; 
}


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

kzr::Message& 
operator<<(kzr::Message& msg, const kzr::Request& request) {
    std::visit([&msg](auto&& value) { msg << value; }, request);
    return msg;
}
kzr::Message& 
operator<<(kzr::Message& msg, const kzr::Response& request) {
    std::visit([&msg](auto&& value) { msg << value; }, request);
    return msg;
}
kzr::Message& 
operator>>(kzr::Message& msg, kzr::Request& request) {
    // decoding is a bit harder actually, we have to figure out the type first and emplace that into memory
    if (auto op = msg.peek(); op) {
        switch (convert(kzr::Operation(*op))) {
#define X(name, _) \
            case kzr::ConceptualOperation:: name : \
                 request.emplace<kzr::BoundRequestType<kzr::ConceptualOperation:: name>>(); \
                 msg.decode(std::get<kzr::BoundRequestType<kzr::ConceptualOperation:: name>>(request)); \
                break 
            X(Version, 100);
            X(Auth, 102);
            X(Attach, 104);
            X(Error, 106);
            X(Flush, 108);
            X(Walk, 110);
            X(Open, 112);
            X(Create, 114);
            X(Read, 116);
            X(Write, 118);
            X(Clunk, 120);
            X(Remove, 122);
            X(Stat, 124);
            X(WStat, 126);
#undef X
            default:
                throw kzr::Exception("Illegal type found!");
        }
        return msg;
    } else {
        throw kzr::Exception("Cannot deduce type because message is not in a good state!");
    }
}

kzr::Message& 
operator>>(kzr::Message& msg, kzr::Response& request) {
    // decoding is a bit harder actually, we have to figure out the type first and emplace that into memory
    if (auto op = msg.peek(); op) {
        switch (convert(kzr::Operation(*op))) {
#define X(name, _) \
            case kzr::ConceptualOperation:: name : \
                 request.emplace<kzr::BoundResponseType<kzr::ConceptualOperation:: name>>(); \
                 msg.decode(std::get<kzr::BoundResponseType<kzr::ConceptualOperation:: name>>(request)); \
                break 
            X(Version, 100);
            X(Auth, 102);
            X(Attach, 104);
            X(Error, 106);
            X(Flush, 108);
            X(Walk, 110);
            X(Open, 112);
            X(Create, 114);
            X(Read, 116);
            X(Write, 118);
            X(Clunk, 120);
            X(Remove, 122);
            X(Stat, 124);
            X(WStat, 126);
#undef X
            default:
                throw kzr::Exception("Illegal type found!");
        }
        return msg;
    } else {
        throw kzr::Exception("Cannot deduce type because message is not in a good state!");
    }
}

kzr::Message&
operator<<(kzr::Message& msg, const kzr::Interaction& thing) {
    std::visit([&msg](auto&& value) { msg << value; }, thing); 
    return msg;
}
kzr::Message&
operator>>(kzr::Message& msg, kzr::Interaction& thing) {
    if (auto op = msg.peek(); op) {
        if (auto lookup = kzr::Operation(*op); kzr::isRequest(lookup)) {
            thing.emplace<kzr::Request>();
            msg >> std::get<kzr::Request>(thing);
        } else {
            // its a response so act accordingly
            thing.emplace<kzr::Response>();
            msg >> std::get<kzr::Response>(thing);
        }
        return msg;
    } else {
        throw kzr::Exception("Cannot deduce type because message is not in a good state!");
    }
}
