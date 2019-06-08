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
#ifndef KZR_MESSAGE_H__
#define KZR_MESSAGE_H__
#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <array>
#include <list>
#include <vector>
#include <set>
#include <sstream>
#include "Operations.h"

namespace kzr {
/**
 * A memory stream used to encode and decode messages
 */
class Message {
    public:
        Message() = default;
        auto str() const;
        void str(const std::string& newStr);
        void reset();
        void encode(uint8_t value);
        void encode(uint16_t value);
        void encode(uint32_t value);
        void encode(uint64_t value);
        void encode(const std::string& value);
        void encode(const std::list<std::string>& value);
        void encode(const std::vector<std::string>& value);
        void encode(const std::set<std::string>& value);
        void decode(uint8_t& value);
        void decode(uint16_t& value);
        void decode(uint32_t& value);
        void decode(uint64_t& value);
        void decode(std::string& value);
        void decode(std::list<std::string>& value);
        void decode(std::vector<std::string>& value);
        void decode(std::set<std::string>& value);
        template<typename T>
        void encode(const T& data) {
            data.encode(*this);
        }
        template<typename T>
        void decode(T& data) {
            data.decode(*this);
        }
        template<typename T, size_t capacity>
        void decode(std::array<T, capacity>& collection) {
            for (auto& ptr : collection) {
                decode(ptr);
            }
        }
        template<typename T, size_t capacity>
        void encode(const std::array<T, capacity>& collec) {
            for (const auto& ptr : collec) {
                encode(ptr);
            }
        }
        template<typename ... Types>
        void encodeSequence(Types&& ... args) {
            static_assert((std::is_const_v<std::forward<Types>(args)> && ...), "All types in the encode sequence must be marked const!");
            (encode(std::forward<Types>(args)), ...);
        }
        template<typename ... Types>
        void decodeSequence(Types&& ... args) {
            static_assert((!std::is_const_v<std::forward<Types>(args)> && ...), "All types in the encode sequence must not be const!");
            static_assert((std::is_reference_v<std::forward<Types>(args)> && ...), "All types in the encode sequence must be a reference!");
            (decode(std::forward<Types>(args)), ...);
        }
    private:
        std::stringstream _storage;
};
/**
 * Generic version of decoding messages, it assumes the class/struct has a
 * decode method.
 * @param in the input stream to read from
 * @param data the value to write out to
 * @return the input stream
 */
template<typename T>
std::istream& decode(std::istream& in, T& data) {
    data.decode(in);
    return in;
}
/**
 * Generic version of encoding messages, it assumes the class/struct has an
 * encode method.
 * @param out the stream to write out to
 * @param data the data to write to the stream
 * @return the output stream
 */
template<typename T>
std::ostream& encode(std::ostream& out, const T& data) {
    data.encode(out);
    return out;
}

std::istream& decode(std::istream&, uint8_t&);
std::ostream& encode(std::ostream&, uint8_t);
std::istream& decode(std::istream&, uint16_t&);
std::ostream& encode(std::ostream&, uint16_t);
std::istream& decode(std::istream&, uint32_t&);
std::ostream& encode(std::ostream&, uint32_t);
std::istream& decode(std::istream&, uint64_t&);
std::ostream& encode(std::ostream&, uint64_t);
std::istream& decode(std::istream&, std::string&);
std::ostream& encode(std::ostream&, const std::string&);
std::istream& decode(std::istream&, std::list<std::string>&);
std::ostream& encode(std::ostream&, const std::list<std::string>&);
std::istream& decode(std::istream&, std::vector<std::string>&);
std::ostream& encode(std::ostream&, const std::vector<std::string>&);

template<typename T, size_t capacity>
std::istream& decode(std::istream& in, std::array<T, capacity>& collection) {
    for (auto& ptr : collection) {
        decode(in, ptr);
    }
    return in;
}
template<typename T, size_t capacity>
std::ostream& encode(std::ostream& out, const std::array<T, capacity>& collection) {
    for (const auto& ptr : collection) {
        encode(out, ptr);
    }
    return out;
}
template<typename T>
std::istream& decode(std::istream& in, std::vector<T>& collection) {
    for (auto& ptr : collection) {
        decode(in, ptr);
    }
    return in;
}

template<typename T>
std::istream& decode(std::istream& in, std::list<T>& collection) {
    for (auto& ptr : collection) {
        decode(in, ptr);
    }
    return in;
}

template<typename T>
std::ostream& encode(std::ostream& out, const std::vector<T>& collection) {
    for (const auto& ptr : collection) {
        encode(out, ptr);
    }
    return out;
}

template<typename T>
std::ostream& encode(std::ostream& out, const std::list<T>& collection) {
    for (const auto& ptr : collection) {
        encode(out, ptr);
    }
    return out;
}


/**
 * Holds the arguments of a request by the client or a response by the server;
 * This top level class contains the elements common to all message kinds
 */
class Action {
    public:
        Action() = default;
        Action(Operation op, uint16_t tag);
        explicit Action(Operation op);
        virtual ~Action() = default;
        constexpr auto getOperation() const noexcept { return _op; }
        constexpr auto getTag() const noexcept { return _tag; }
        void setTag(uint16_t value) noexcept { _tag = value; }
        void setOperation(Operation value) noexcept { _op = value; }
        virtual void encode(std::ostream&) const;
        virtual void decode(std::istream&);
    private:
        Operation _op;
        uint16_t _tag;
};

class VersionAction : public Action {
    public:
        using Parent = Action;
    public:
        using Parent::Parent;
        virtual ~VersionAction() = default;
        void encode(std::ostream&) const override;
        void decode(std::istream&) override;
        auto getVersion() const noexcept { return _version; }
        constexpr auto getMsize() const noexcept { return _msize; }
        void setMsize(uint16_t msize) noexcept { _msize = msize; }
        void setVersion(const std::string& value) { _version = value; }
    private:
        std::string _version;
        uint16_t _msize;
};

} // end namespace kzr
#endif // end KZR_MESSAGE_H__

