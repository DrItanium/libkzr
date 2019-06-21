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

#include "MessageStream.h"
#include "Exception.h"

namespace kzr {
void
MessageStream::decode(uint8_t& out) {
    char temporaryStorage;
    _storage.get(temporaryStorage);
    out = temporaryStorage;
}

void
MessageStream::encode(uint8_t value) {
    _storage.put(value);
}

void
MessageStream::decode(uint16_t& out) {
    char temporaryStorage[2];
    _storage.read(temporaryStorage, 2);
    out = build(uint8_t(temporaryStorage[0]), uint8_t(temporaryStorage[1]));
}

void
MessageStream::encode(uint16_t value) {
    _storage.put(uint8_t(value));
    _storage.put(uint8_t(value >> 8));
}

void
MessageStream::decode(uint32_t& out) {
    char temporaryStorage[4];
    _storage.read(temporaryStorage, 4);
    out = build(temporaryStorage[0], temporaryStorage[1], temporaryStorage[2], temporaryStorage[3]);
}

void
MessageStream::encode(uint32_t value) {
    encode(uint16_t(value));
    encode(uint16_t(value >> 16));
}

void
MessageStream::decode(uint64_t& out) {
    out = build(decode<uint32_t>(), decode<uint32_t>());
}

void
MessageStream::encode(uint64_t value) {
    encode(uint32_t(value));
    encode(uint32_t(value >> 32));
}

void
MessageStream::decode(std::string& data) {
    auto len = decode<uint16_t>();
    data.reserve(len);
    _storage.read(data.data(), len);
}
void
MessageStream::encode(const std::string& value) {
    if (uint16_t len = value.length(); len != value.length()) {
        throw kzr::Exception("Attempted to encode a string of ", value.length(), " characters when ", ((decltype(len))-1), " is the maximum allowed!");
    } else {
        encode(len);
        for (const auto& c : value) {
            encode(uint8_t(c));
        }
    }
}
std::string
MessageStream::str() const { 
    return _storage.str(); 
}
void MessageStream::str(const std::string& input) { _storage.str(input); }
void MessageStream::reset() { _storage.str(""); }

void 
MessageStream::write(const std::stringstream::char_type* s, std::streamsize count) {
    _storage.write(s, count);
    if (_storage.bad()) {
        throw kzr::Exception("bad bit set during write!");
    }
}
void
MessageStream::write(const std::string& str) {
    write(str.c_str(), str.length());
}
auto 
MessageStream::read(std::stringstream::char_type* s, std::streamsize count) {
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
MessageStream::read(std::string& str) {
    return read(str.data(), str.length());
}
std::optional<uint8_t>
MessageStream::peek() noexcept {
    if (auto result = _storage.peek(); result != _storage.eof()) {
        return std::optional<uint8_t>(uint8_t(result));
    } else {
        return std::nullopt;
    }
}
} // end namespace kzr

kzr::MessageStream& 
operator<<(kzr::MessageStream& msg, const std::string& value) {
    msg.encode(value);
    return msg;
}
kzr::MessageStream& 
operator>>(kzr::MessageStream& msg, std::string& value) {
    msg.decode(value);
    return msg;
}
#define X(type) \
    kzr::MessageStream& \
        operator<<(kzr::MessageStream& msg, type data) { \
            msg.encode(data); \
            return msg;  \
        } \
        kzr::MessageStream& \
        operator>>(kzr::MessageStream& msg, type & data ) { \
            msg.decode(data); \
            return msg; \
        }
        X(uint8_t);
        X(uint16_t);
        X(uint32_t);
        X(uint64_t);
#undef X
