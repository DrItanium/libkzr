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
        template<typename T>
        Message& operator<<(const T& data) {
            encode(data);
            return *this;
        }
        template<typename T>
        Message& operator>>(T& data) {
            decode(data);
            return *this;
        }
#define X(type) \
        Message& operator<<(type data) { \
            encode(data); \
            return *this;  \
        } \
        Message& operator>>(type & data ) { \
            decode(data); \
            return *this; \
        }
        X(uint8_t);
        X(uint16_t);
        X(uint32_t);
        X(uint64_t);
#undef X
    private:
        std::stringstream _storage;
};


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
        virtual void encode(Message&) const;
        virtual void decode(Message&);
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
        void encode(Message&) const override;
        void decode(Message&) override;
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

