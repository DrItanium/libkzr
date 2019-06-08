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

std::istream& 
decode(std::istream& in, uint8_t& out) {
    char temporaryStorage;
    in.get(temporaryStorage);
    out = temporaryStorage;
    return in;
}

std::ostream& 
encode(std::ostream& out, uint8_t value) {
    return out.put(value);
}

std::istream& 
decode(std::istream& in, uint16_t& out) {
    char temporaryStorage[2];
    in.read(temporaryStorage, 2);
    out = (uint16_t(temporaryStorage[1]) << 8) | (uint16_t(temporaryStorage[0]));
    return in;
}

std::ostream& 
encode(std::ostream& out, uint16_t value) {
    out.put(uint8_t(value));
    out.put(uint8_t(value >> 8));
    return out;
}

std::istream& 
decode(std::istream& in, uint32_t& out) {
    char temporaryStorage[4];
    in.read(temporaryStorage, 4);
    out = (uint32_t(temporaryStorage[3]) << 24) |
        (uint32_t(temporaryStorage[2]) << 16) | 
        (uint32_t(temporaryStorage[1]) << 8) | 
        (uint32_t(temporaryStorage[0]));
    return in;
}

std::ostream& 
encode(std::ostream& out, uint32_t value) {
    out.put(uint8_t(value));
    out.put(uint8_t(value >> 8));
    out.put(uint8_t(value >> 16));
    out.put(uint8_t(value >> 24));
    return out;
}

std::istream& 
decode(std::istream& in, uint64_t& out) {
    uint32_t temporaryStorage[2];
    decode(in, temporaryStorage[0]);
    decode(in, temporaryStorage[1]);
    out = (uint64_t(temporaryStorage[1]) << 32) | 
        (uint64_t(temporaryStorage[0]));
    return in;
}

std::ostream& 
encode(std::ostream& out, uint64_t value) {
    encode(out, uint32_t(value));
    encode(out, uint32_t(value >> 32));
    return out;
}

std::istream& 
decode(std::istream& in, std::string& data) {
    uint16_t len;
    decode(in, len);
    data.reserve(len);
    return in.read(data.data(), len);
}
std::ostream& 
encode(std::ostream& out, const std::string& value) {
    if (uint16_t len = value.length(); len != value.length()) {
        throw kzr::Exception("Attempted to encode a string of ", value.length(), " characters when ", ((decltype(len))-1), " is the maximum allowed!");
    } else {
        encode(out, len);
        for (const auto& c : value) {
            encode(out, uint8_t(c));
        }
        return out;
    }
}

std::istream& 
decode(std::istream& in, std::list<std::string>& collec) {
    uint16_t len;
    decode(in, len); // we can discard this value as it doesn't matter
    for (auto i = 0; i < len; ++i) {
        std::string temporary;
        decode(in, temporary);
        collec.emplace_back(temporary);
    }
    return in;
}
std::ostream&
encode(std::ostream& out, const std::list<std::string>& collec) {
    if (uint16_t len = collec.size(); len != collec.size()) {
        throw kzr::Exception("Attempted to encode a std::list<std::string> of ", collec.size(), " elements when ", ((decltype(len))-1), " is the maximum allowed!");
    } else {
        encode(out, len);
        for (const auto& c : collec) {
            encode(out, c);
        }
        return out;
    }
}
std::istream& 
decode(std::istream& in, std::vector<std::string>& collec) {
    uint16_t len;
    decode(in, len); // we can discard this value as it doesn't matter
    for (auto i = 0; i < len; ++i) {
        std::string temporary;
        decode(in, temporary);
        collec.emplace_back(temporary);
    }
    return in;
}
std::ostream&
encode(std::ostream& out, const std::vector<std::string>& collec) {
    if (uint16_t len = collec.size(); len != collec.size()) {
        throw kzr::Exception("Attempted to encode a std::vector<std::string> of ", collec.size(), " elements when ", ((decltype(len))-1), " is the maximum allowed!");
    } else {
        encode(out, len);
        for (const auto& c : collec) {
            encode(out, c);
        }
        return out;
    }
}
Action::Action(Operation op, uint16_t tag) : _op(op), _tag(tag) { }
Action::Action(Operation op) : Action(op, -1) { }
void 
Action::encode(std::ostream& out) const {
    kzr::encode(out, _op);
    kzr::encode(out, _tag);
}
void 
Action::decode(std::istream& in) {
    kzr::decode(in, _op);
    kzr::decode(in, _tag);
}

void
VersionAction::encode(std::ostream& out) const {
    Parent::encode(out);
    kzr::encode(out, _msize);
    kzr::encode(out, _version);
}

void
VersionAction::decode(std::istream& in) {
    Parent::decode(in);
    kzr::decode(in, _msize);
    kzr::decode(in, _version);
}

} // end namespace kzr
