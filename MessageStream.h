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
#ifndef KZR_MESSAGE_STREAM_H__
#define KZR_MESSAGE_STREAM_H__
#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <optional>
#include <vector>
#include <sstream>
#include <tuple>
#include <variant>
#ifdef __cpp_lib_concepts
#include <concepts>
#endif 
#include "Operations.h"
#include "Exception.h"

namespace kzr {
constexpr uint16_t notag = uint16_t(~0);
constexpr char version9pString[] = "9P";
constexpr char version9p2000String[] = "9P2000";
constexpr uint16_t build(uint8_t lower, uint8_t upper) noexcept {
    return (uint16_t(upper) << 8) | uint16_t(lower);
}
constexpr uint32_t build(uint16_t lower, uint16_t upper) noexcept {
    return (uint32_t(upper) << 16) | uint16_t(lower);
}
constexpr uint32_t build(uint8_t lowest, uint8_t lower, uint8_t high, uint8_t highest) noexcept {
    return build(build(lowest, lower), build(high, highest));
}
constexpr uint64_t build(uint32_t lower, uint32_t upper) noexcept {
    return (uint64_t(upper) << 32) | uint64_t(lower);
}
#ifdef __cpp_lib_concepts
template<typename T>
concept Encodable = requires(MessageStream& os, const T& a) {
    { a.encode(os); } -> void;
};

template<typename T>
concept Decodeable = requires(MessageStream& is, T& a) {
    { a.encode(is); } -> void;
};
#endif // end __cpp_lib_concepts 

/**
 * A memory stream used to encode and decode messages
 */
class MessageStream {
    public:
        auto read(std::stringstream::char_type* s, std::streamsize count);
        auto read(std::string& str);
        void write(const std::stringstream::char_type* s, std::streamsize count);
        void write(const std::string& str);
        [[nodiscard]] std::string str() const;
        void str(const std::string& newStr);
        void reset();
        void encode(uint8_t value);
        void encode(uint16_t value);
        void encode(uint32_t value);
        void encode(uint64_t value);
        void encode(const std::string& value);
        void decode(uint8_t& value);
        void decode(uint16_t& value);
        void decode(uint32_t& value);
        void decode(uint64_t& value);
        void decode(std::string& value);
        std::optional<uint8_t> peek() noexcept;
#ifdef __cpp_lib_concepts
        void encode(const Encodable& data) {
            data.encode(*this);
        }
        void decode(Encodable& data) {
            data.decode(*this);
        }
#else 
        template<typename T>
        void encode(const T& data) {
            data.encode(*this);
        }
        template<typename T>
        void decode(T& data) {
            data.decode(*this);
        }
#endif
        template<typename T>
        T decode() {
            T value;
            decode(value);
            return value;
        }
        auto length() const noexcept { return _storage.str().length(); }
    private:
        std::stringstream _storage;
};
} // end namespace kzr

kzr::MessageStream& operator<<(kzr::MessageStream&, const std::string&);
kzr::MessageStream& operator>>(kzr::MessageStream&, std::string&);
kzr::MessageStream& operator<<(kzr::MessageStream&, uint8_t);
kzr::MessageStream& operator>>(kzr::MessageStream&, uint8_t&);
kzr::MessageStream& operator<<(kzr::MessageStream&, uint16_t);
kzr::MessageStream& operator>>(kzr::MessageStream&, uint16_t&);
kzr::MessageStream& operator<<(kzr::MessageStream&, uint32_t);
kzr::MessageStream& operator>>(kzr::MessageStream&, uint32_t&);
kzr::MessageStream& operator<<(kzr::MessageStream&, uint64_t);
kzr::MessageStream& operator>>(kzr::MessageStream&, uint64_t&);


template<typename T>
kzr::MessageStream& operator<<(kzr::MessageStream& msg, const T& value) {
    msg.encode<T>(value);
    return msg;
}
template<typename T>
kzr::MessageStream& operator>>(kzr::MessageStream& msg, T& value) {
    msg.decode<T>(value);
    return msg;
}
template<typename T>
kzr::MessageStream& operator<<(kzr::MessageStream& msg, const std::vector<T>& collec) {
    if (uint16_t len = collec.size(); len != collec.size()) {
        throw kzr::Exception("Attempted to encode a std::vector<T> of ", collec.size(), " elements when ", ((decltype(len))-1), " is the maximum allowed!");
    } else {
        msg << len;
        for (const auto& c : collec) {
            msg << c;
        }
        return msg;
    }
}

template<typename T>
kzr::MessageStream& operator>>(kzr::MessageStream& msg, std::vector<T>& collec) {
    auto len = msg.decode<uint16_t>();
    for (auto i = 0; i < len; ++i) {
        collec.emplace_back();
        msg >> collec.back();
    }
    return msg;
}

#endif // end KZR_MESSAGE_STREAM_H__
