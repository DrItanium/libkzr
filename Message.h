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
#include <tuple>
#include "Operations.h"

namespace kzr {
constexpr uint16_t build(uint8_t lower, uint8_t upper) noexcept {
    return (uint16_t(upper) << 8) | uint16_t(lower);
}
constexpr uint32_t build(uint16_t lower, uint16_t upper) noexcept {
    return (uint32_t(upper) << 16) | uint16_t(lower);
}
constexpr uint32_t build(uint8_t lowest, uint8_t lower, uint8_t high, uint8_t highest) noexcept {
    return build(build(lowest, lower), build(high, highest));
}
/**
 * A memory stream used to encode and decode messages
 */
class Message {
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
        template<typename T>
        void encode(const T& data) {
            data.encode(*this);
        }
        template<typename T>
        void decode(T& data) {
            data.decode(*this);
        }
#define X(type) \
        Message& operator<<(type data); \
        Message& operator>>(type& data)
        X(uint8_t);
        X(uint16_t);
        X(uint32_t);
        X(uint64_t);
#undef X
        Message& operator<<(const std::string& value);
        Message& operator>>(std::string& value);
        auto length() const noexcept { return _storage.str().length(); }
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
class ErrorResponse : public Action {
    public:
        using Parent = Action;
    public:
        using Parent::Parent;
        virtual ~ErrorResponse() = default;
        void encode(Message&) const override;
        void decode(Message&) override;
        auto getName() const noexcept { return _ename; }
        void setName(const std::string& value) noexcept { _ename = value; }
    private:
        std::string _ename;
};

class FlushAction : public Action {
    public:
        using Parent = Action;
    public:
        using Parent::Parent;
        virtual ~FlushAction() = default;
        void encode(Message&) const override;
        void decode(Message&) override;
        constexpr auto getOldTag() const noexcept { return _oldtag; }
        void setOld(uint16_t value) noexcept { _oldtag = value; }
    private:
        uint16_t _oldtag;
};

template<Operation op>
struct OperationToTypeBinding final {
    OperationToTypeBinding() = delete;
    ~OperationToTypeBinding() = delete;
    OperationToTypeBinding(const OperationToTypeBinding&) = delete;
    OperationToTypeBinding(OperationToTypeBinding&&) = delete;
    OperationToTypeBinding& operator=(const OperationToTypeBinding&) = delete;
    OperationToTypeBinding& operator=(OperationToTypeBinding&&) = delete;
    static constexpr auto TargetOperation = op;
};

#define DefTypeBinding(op, type) \
    template<> \
    struct OperationToTypeBinding<Operation:: op > final { \
    OperationToTypeBinding() = delete; \
    ~OperationToTypeBinding() = delete; \
    OperationToTypeBinding(const OperationToTypeBinding&) = delete; \
    OperationToTypeBinding(OperationToTypeBinding&&) = delete; \
    OperationToTypeBinding& operator=(const OperationToTypeBinding&) = delete; \
    OperationToTypeBinding& operator=(OperationToTypeBinding&&) = delete; \
    static constexpr auto TargetOperation = Operation:: op ; \
    using BoundType = type ; \
    }
#define X(op, type) DefTypeBinding(op, type)
X(TVersion, VersionAction);
X(RVersion, VersionAction);
#undef X
#undef DefTypeBinding



} // end namespace kzr

template<typename T>
kzr::Message& operator<<(kzr::Message& msg, const T& value) {
    msg.encode<T>(value);
    return msg;
}
template<typename T>
kzr::Message& operator>>(kzr::Message& msg, T& value) {
    msg.decode<T>(value);
    return msg;
}
kzr::Message& operator<<(kzr::Message&, const std::list<std::string>&);
kzr::Message& operator<<(kzr::Message&, const std::vector<std::string>&);
kzr::Message& operator<<(kzr::Message&, const std::set<std::string>&);
kzr::Message& operator>>(kzr::Message&, std::list<std::string>&);
kzr::Message& operator>>(kzr::Message&, std::vector<std::string>&);
kzr::Message& operator>>(kzr::Message&, std::set<std::string>&);

template<typename T, size_t capacity>
kzr::Message& operator<<(kzr::Message& msg, const std::array<T, capacity>& a) {
    for (const auto& ptr : a) {
        msg << ptr;
    }
    return msg;
}

template<typename T, size_t capacity>
kzr::Message& operator>>(kzr::Message& msg, std::array<T, capacity>& a) {
    for (auto& ptr : a) {
        msg >> ptr;
    }
    return msg;
}
#endif // end KZR_MESSAGE_H__

