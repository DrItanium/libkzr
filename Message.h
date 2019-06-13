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
constexpr uint64_t build(uint32_t lower, uint32_t upper) noexcept {
    return (uint64_t(upper) << 32) | uint64_t(lower);
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
        template<typename T>
        T decode() {
            T value;
            decode(value);
            return value;
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
 * A unique identification for the given file being accessed
 */
class Qid  {
    public:
        Qid() = default;
        Qid(uint8_t type, uint64_t path, uint32_t version = 0);
        void setType(uint8_t v) noexcept { _type = v; }
        void setVersion(uint32_t v) noexcept { _version = v; }
        void setPath(uint64_t v) noexcept { _path = v; }
        constexpr auto getType() const noexcept { return _type; }
        constexpr auto getVersion() const noexcept { return _version; }
        constexpr auto getPath() const noexcept { return _path; }
        void encode(Message&) const;
        void decode(Message&);
    private:
        uint8_t _type;
        uint32_t _version;
        uint64_t _path;
};

class Stat {
    public:
        Stat() = default;
        void encode(Message&) const;
        void decode(Message&);
        Qid& getQid() noexcept { return _qid; }
        const Qid& getQid() const noexcept { return _qid; }
        void setQid(const Qid& qid) noexcept { _qid = qid; }
#define X(name, field) \
        auto get ## name () const noexcept { return field ; } \
        void set ## name ( const std::string& value) noexcept { field = value ; }
        X(FileName, _name);
        X(Group, _gid);
        X(Owner, _uid);
        X(UserThatLastModified, _muid);
#undef X
#define X(name, field, type) \
        constexpr auto get  ## name () const noexcept { return field ; } \
        void set ## name (type value) noexcept  { field = value ; } 
        X(Type, _type, uint16_t);
        X(Device, _dev, uint32_t);
        X(Permissions, _mode, uint32_t);
        X(LastAccessTime, _atime, uint32_t);
        X(LastModificationTime, _mtime, uint32_t);
        X(Length, _length, uint64_t);
#undef X
    private:
        uint16_t _type;
        uint32_t _dev;
        Qid _qid;
        uint32_t _mode,
                 _atime,
                 _mtime;
        uint64_t _length;
        std::string _name,
            _uid,
            _gid,
            _muid;
};


/**
 * Holds the arguments of a request by the client or a response by the server;
 * This top level class contains the elements common to all message kinds
 */
class Action {
    public:
        Action() = default;
        explicit Action(Operation op);
        Action(Operation op, uint16_t tag);
        virtual ~Action() = default;
        constexpr auto getTag() const noexcept { return _tag; }
        void setTag(uint16_t value) noexcept { _tag = value; }
        virtual void setOperation(Operation op) noexcept { _op = op; }
        virtual void encode(Message& msg) const;
        virtual void decode(Message& msg);
        constexpr auto getOperation() const noexcept { return _op; }
        constexpr auto isRequest() const noexcept { return kzr::isRequest(_op); }
        constexpr auto isResponse() const noexcept { return kzr::isResponse(_op); }
        constexpr auto getConceptualOperation() const noexcept { return kzr::convert(_op); }
        constexpr auto isError() const noexcept { return getConceptualOperation() == ConceptualOperation::Error; }
    private:
        Operation _op;
        uint16_t _tag;
};
template<Operation op>
class FixedAction : public Action {
    public:
        using Parent = Action;
    public:
        FixedAction() : Parent(op) { }
        explicit FixedAction(uint16_t tag) : Parent(op, tag) { }
        ~FixedAction() override = default;
        void setOperation(Operation) noexcept override { 
            // do nothing since it is fixed!
        }
};
template<ConceptualOperation op>
class FixedResponse : public FixedAction<ConceptualOperationToROperation<op>> {
    public:
        using Parent = FixedAction<ConceptualOperationToROperation<op>>;
    public:
        using Parent::Parent;
        ~FixedResponse() override = default;
};
template<ConceptualOperation op>
class FixedRequest : public FixedAction<ConceptualOperationToTOperation<op>> {
    public:
        using Parent = FixedAction<ConceptualOperationToTOperation<op>>;
    public:
        using Parent::Parent;
        ~FixedRequest() override = default;

};
class ErrorResponse : public FixedResponse<ConceptualOperation::Error> {
    public:
        using Parent = FixedResponse<ConceptualOperation::Error>;
    public:
        using Parent::Parent;
        virtual ~ErrorResponse() = default;
        std::string getErrorName() const noexcept { return _ename; }
        void setErrorName(const std::string& value) noexcept { _ename = value; }
        virtual void encode(Message&) const;
        virtual void decode(Message&);
    private:
        std::string _ename;
};

class VersionAction : public Action {
    public:
        using Parent = Action;
    public:
        using Parent::Parent;
        ~VersionAction() override = default;
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

class AuthenticationRequest : public FixedRequest<ConceptualOperation::Auth> {
    public:
        using Parent = FixedRequest<ConceptualOperation::Auth>;
    public:
        using Parent::Parent;
        ~AuthenticationRequest() override = default;
        constexpr auto setAttachFid() const noexcept { return _afid; }
        void setAttachFid(uint32_t value) noexcept { _afid = value; }
#define X(title, name) \
        auto get ## title () const noexcept { return  name ; } \
        void set ## title (const std::string& value ) noexcept { name  = value ; }
        X(UserName, _uname); 
        X(AttachName, _aname);
#undef X
        void encode(Message&) const override;
        void decode(Message&) override;
    private:
        uint32_t _afid;
        std::string _uname;
        std::string _aname;
};

class AuthenticationResponse : public FixedResponse<ConceptualOperation::Auth> {
    public:
        using Parent = FixedResponse<ConceptualOperation::Auth>;
    public:
        using Parent::Parent;
        ~AuthenticationResponse() override = default;
        void encode(Message&) const override;
        void decode(Message&) override;
        Qid& getQid() noexcept { return _aqid; }
        const Qid& getQid() const noexcept { return _aqid; }
        void setQid(const Qid& qid) { _aqid = qid; }
    private:
        Qid _aqid;
};


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

