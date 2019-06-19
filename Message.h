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
#include "Exception.h"

namespace kzr {
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

class HasQid {
    public:
        Qid& getQid() noexcept { return _qid; }
        const Qid& getQid() const noexcept { return _qid; }
        void setQid(const Qid& qid) { _qid = qid; }
        void encode(Message& msg) const;
        void decode(Message& msg);
    private:
        Qid _qid;
};

class Stat : public HasQid {
    public:
        Stat() = default;
        void encode(Message&) const;
        void decode(Message&);
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
        uint32_t _mode,
                 _atime,
                 _mtime;
        uint64_t _length;
        std::string _name,
            _uid,
            _gid,
            _muid;
};

class HasFid {
    public:
        void encode(Message& msg) const;
        void decode(Message& msg);
        constexpr auto getFid() const noexcept { return _fid; }
        void setFid(uint32_t value) noexcept { _fid = value; }
    private:
        uint32_t _fid;
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
class VersionBody {
    public:
        void encode(Message&) const;
        void decode(Message&);
        auto getVersion() const noexcept { return _version; }
        constexpr auto getMsize() const noexcept { return _msize; }
        void setMsize(uint16_t msize) noexcept { _msize = msize; }
        void setVersion(const std::string& value) { _version = value; }
    private:
        std::string _version;
        uint16_t _msize;

};
class VersionRequest : public FixedRequest<ConceptualOperation::Version>, public VersionBody {
    public:
        using Parent = FixedRequest<ConceptualOperation::Version>;
    public:
        using Parent::Parent;
        ~VersionRequest() override = default;
        void encode(Message&) const override;
        void decode(Message&) override;
};

class VersionResponse : public FixedResponse<ConceptualOperation::Version>, public VersionBody {
    public:
        using Parent = FixedResponse<ConceptualOperation::Version>;
    public:
        using Parent::Parent;
        ~VersionResponse() override = default;
        void encode(Message&) const override;
        void decode(Message&) override;
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

class AuthenticationResponse : public FixedResponse<ConceptualOperation::Auth>, public HasQid {
    public:
        using Parent = FixedResponse<ConceptualOperation::Auth>;
    public:
        using Parent::Parent;
        ~AuthenticationResponse() override = default;
        void encode(Message&) const override;
        void decode(Message&) override;
};

class FlushRequest : public FixedRequest<ConceptualOperation::Flush> {
    public:
        using Parent = FixedRequest<ConceptualOperation::Flush>;
    public:
        using Parent::Parent;
        ~FlushRequest() override = default;
        void encode(Message&) const override;
        void decode(Message&) override;
        constexpr auto getOldTag() const noexcept { return _oldtag; }
        void setOldTag(uint16_t value) noexcept { _oldtag = value; }
    private:
        uint16_t _oldtag;
};

using FlushResponse = FixedResponse<ConceptualOperation::Flush>;

class AttachRequest : public FixedRequest<ConceptualOperation::Attach>, public HasFid {
    public:
        using Parent = FixedRequest<ConceptualOperation::Attach>;
    public:
        using Parent::Parent;
        ~AttachRequest() override = default;
        void encode(Message&) const override;
        void decode(Message&) override;
#define X(title, name) \
        constexpr auto get ## title () const noexcept { return name ; } \
        void set ## title (uint32_t value) noexcept { name = value ; } 
        X(AFid, _afid);
#undef X
#define X(title, name) \
        auto get ## title () const noexcept { return  name ; } \
        void set ## title (const std::string& value ) noexcept { name  = value ; }
        X(UserName, _uname); 
        X(AttachName, _aname);
#undef X
    private:
        uint32_t _afid;
        std::string _uname, _aname;
};
class AttachResponse : public FixedResponse<ConceptualOperation::Attach>, public HasQid {
    
    public:
        using Parent = FixedResponse<ConceptualOperation::Attach>;
    public:
        using Parent::Parent;
        ~AttachResponse() override = default;
        void encode(Message&) const override;
        void decode(Message&) override;
};

class WalkRequest : public FixedRequest<ConceptualOperation::Walk>, public HasFid {
    public:
        using Parent = FixedRequest<ConceptualOperation:: Walk>; 
    public: 
        using Parent::Parent; 
        ~ WalkRequest () override = default ; 
        void encode(Message&) const override; 
        void decode(Message&) override;
        auto& getWname() noexcept { return _wname; }
        const auto& getWname() const noexcept { return _wname; }
        void setNewFid(uint32_t value) noexcept { _newfid = value; }
        constexpr auto getNewFid() const noexcept { return _newfid; }
    private:
        uint32_t _newfid;
        std::vector<std::string> _wname;
};

class WalkResponse : public FixedResponse<ConceptualOperation::Walk> {
    public:
        using Parent = FixedResponse<ConceptualOperation:: Walk>; 
    public: 
        using Parent::Parent; 
        ~ WalkResponse () override = default ; 
        void encode(Message&) const override; 
        void decode(Message&) override;
        auto& getWqid() noexcept { return _wqid; }
        const auto& getWqid() const noexcept { return _wqid; }
    private:
        std::vector<Qid> _wqid;
};

class OpenRequest : public FixedRequest<ConceptualOperation::Open>, public HasFid {
    public:
        using Parent = FixedRequest<ConceptualOperation:: Open>; 
    public: 
        using Parent::Parent; 
        ~OpenRequest() override = default ; 
        void encode(Message&) const override; 
        void decode(Message&) override;
        constexpr auto getMode() const noexcept { return _mode; }
        void setMode(uint8_t v) noexcept { _mode = v; }
    private:
        uint8_t _mode;
};

template<ConceptualOperation op>
class OpenOrCreateResponse : public FixedResponse<op>, public HasQid {
    public:
        static_assert(op == ConceptualOperation::Create ||
                      op == ConceptualOperation::Open, "Illegal OpenOrCreateResponse kind!");
        using Parent = FixedResponse<op>;
    public:
        using Parent::Parent;
        ~OpenOrCreateResponse() override = default;
        void encode(Message& msg) const override {
            Parent::encode(msg);
            HasQid::encode(msg);
            msg << _iounit;
        }
        void decode(Message& msg) override {
            Parent::decode(msg);
            HasQid::decode(msg);
            msg >> _iounit;
        }
        constexpr auto getIounit() const noexcept { return _iounit; }
        void setIounit(uint32_t v) noexcept { _iounit = v; }
    private:
        uint32_t _iounit;

};
using OpenResponse = OpenOrCreateResponse<ConceptualOperation::Open>;
class CreateRequest : public FixedRequest<ConceptualOperation::Create>, public HasFid {
    public:
        using Parent = FixedRequest<ConceptualOperation:: Create>; 
    public: 
        using Parent::Parent; 
        ~CreateRequest() override = default ; 
        void encode(Message&) const override; 
        void decode(Message&) override;
        constexpr auto getPermissions() const noexcept { return _perm; }
        void setPermissions(uint32_t v) noexcept { _perm = v; }
        constexpr auto getMode() const noexcept { return _mode; }
        void setMode(uint8_t v) noexcept { _mode = v; }
        auto getName() const noexcept { return  _name ; } 
        void setName(const std::string& value ) noexcept { _name  = value ; }
    private:
        std::string _name;
        uint32_t _perm;
        uint8_t _mode;
};

using CreateResponse = OpenOrCreateResponse<ConceptualOperation::Create>;

template<ConceptualOperation op>
class FidRequest : public FixedRequest<op>, public HasFid {
    public:
        using Parent = FixedRequest<op>;
    public:
        using Parent::Parent;
        ~FidRequest() override = default;
        void encode(Message& msg) const override {
            Parent::encode(msg);
            HasFid::encode(msg);
        }
        void decode(Message& msg) override {
            Parent::decode(msg);
            HasFid::decode(msg);
        }
};

class HasCount {
    public:
        constexpr auto getCount() const noexcept { return _count; }
        void setCount(uint32_t v) noexcept { _count = v; }
        void encode(Message& msg) const;
        void decode(Message& msg);
    private:
        uint32_t _count;
};
class HasOffset {
    public:
        constexpr auto getOffset() const noexcept { return _offset; }
        void setOffset(uint64_t v) noexcept { _offset = v; }
        void encode(Message& msg) const;
        void decode(Message& msg);
    private:
        uint64_t _offset;
};
class HasDataStorage {
    public:
        auto size() const noexcept { return _data.size(); }
        auto& getData() noexcept { return _data; }
        const auto& getData() const noexcept { return _data; }
        void encode(Message& msg) const;
        void decode(Message& msg);
    private:
        std::vector<uint8_t> _data;
};
template<ConceptualOperation op>
class ReadWriteRequest : public FidRequest<op>, public HasOffset {
    public:
        using Parent = FidRequest<op>;
    public:
        using Parent::Parent;
        ~ReadWriteRequest() override = default;
        void encode(Message& msg) const override {
            Parent::encode(msg);
            HasOffset::encode(msg);
        }
        void decode(Message& msg) override {
            Parent::decode(msg);
            HasOffset::decode(msg);
        }
};
class ReadRequest : public ReadWriteRequest<ConceptualOperation::Read>, public HasCount {
    public:
        using Parent = ReadWriteRequest<ConceptualOperation::Read>;
    public:
        using Parent::Parent;
        ~ReadRequest() override = default;
        void encode(Message& msg) const override;
        void decode(Message& msg) override;
};

class ReadResponse : public FixedResponse<ConceptualOperation::Read>, public HasDataStorage {
    public:
        using Parent = FixedResponse<ConceptualOperation::Read>;
    public:
        using Parent::Parent;
        ~ReadResponse() override = default;
        void encode(Message&) const override;
        void decode(Message&) override;
};

class WriteRequest : public ReadWriteRequest<ConceptualOperation::Write>, public HasDataStorage {
    public:
        using Parent = ReadWriteRequest<ConceptualOperation::Write>;
    public:
        using Parent::Parent;
        ~WriteRequest() override = default;
        void encode(Message&) const override;
        void decode(Message&) override;
};

class WriteResponse : public FixedResponse<ConceptualOperation::Write>, public HasCount {
    public:
        using Parent = FixedResponse<ConceptualOperation::Write>;
    public:
        using Parent::Parent;
        ~WriteResponse() override = default;
        void encode(Message&) const override;
        void decode(Message&) override;
};

using ClunkRequest = FidRequest<ConceptualOperation::Clunk>;
using ClunkResponse = FixedResponse<ConceptualOperation::Clunk>;
using RemoveRequest = FidRequest<ConceptualOperation::Remove>;
using RemoveResponse = FixedResponse<ConceptualOperation::Remove>;
using StatRequest = FidRequest<ConceptualOperation::Stat>;
using WStatResponse = FixedResponse<ConceptualOperation::WStat>;



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

template<typename T>
kzr::Message& operator<<(kzr::Message& msg, const std::vector<T>& collec) {
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
kzr::Message& operator>>(kzr::Message& msg, std::vector<T>& collec) {
    uint16_t len;
    msg >> len;
    for (auto i = 0; i < len; ++i) {
        collec.emplace_back();
        msg >> collec.back();
    }
    return msg;
}

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

