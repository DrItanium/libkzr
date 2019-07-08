/**
 * @file
 * Object representations of message kinds
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
#include <optional>
#include <vector>
#include <sstream>
#include <tuple>
#include <variant>
#include "Operations.h"
#include "Exception.h"
#include "MessageStream.h"

namespace kzr {

/**
 * A unique identification for the given file being accessed by the server
 */
class Qid  {
    public:
        Qid() = default;
        Qid(uint8_t type, uint64_t path, uint32_t version = 0);
        void setType(uint8_t v) noexcept { _type = v; }
        void setVersion(uint32_t v) noexcept { _version = v; }
        void setPath(uint64_t v) noexcept { _path = v; }
        /**
         * Get the type of the file (such as directory, normal file, 
         * append only file, etc)
         */
        constexpr auto getType() const noexcept { return _type; }
        /**
         * Get the revision number of the file. Each modification to the file
         * will cause the version field to be incremented.
         */
        constexpr auto getVersion() const noexcept { return _version; }
        /**
         * Get the unique inode-like number that refers to this file on the file
         * server.
         */
        constexpr auto getPath() const noexcept { return _path; }
        void encode(MessageStream&) const;
        void decode(MessageStream&);
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
        void encode(MessageStream& msg) const;
        void decode(MessageStream& msg);
    private:
        Qid _qid;
};

class HasName {
    public:
        const std::string& getName() const noexcept { return _name; }
        void setName(const std::string& value) noexcept { _name = value; }
        void encode(MessageStream& msg) const;
        void decode(MessageStream& msg);
    private:
        std::string _name;
};

class Stat : public HasQid, public HasName {
    public:
        Stat() = default;
        void encode(MessageStream&) const;
        void decode(MessageStream&);
#define X(name, field) \
        auto get ## name () const noexcept { return field ; } \
        void set ## name ( const std::string& value) noexcept { field = value ; }
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
        std::string _uid,
            _gid,
            _muid;
};

class HasFid {
    public:
        void encode(MessageStream& msg) const;
        void decode(MessageStream& msg);
        constexpr auto getFid() const noexcept { return _fid; }
        void setFid(uint32_t value) noexcept { _fid = value; }
    private:
        uint32_t _fid;
};

/**
 * Holds the arguments of a request by the client or a response by the server;
 * This top level class contains the elements common to all message kinds
 */
class MessageHeader {
    public:
        explicit MessageHeader(Operation op);
        MessageHeader(Operation op, uint16_t tag);
        virtual ~MessageHeader() = default;
        constexpr auto getTag() const noexcept { return _tag; }
        virtual void setTag(uint16_t value) noexcept { _tag = value; }
        virtual void encode(MessageStream& msg) const;
        virtual void decode(MessageStream& msg);
        constexpr auto getOperation() const noexcept { return _op; }
        constexpr auto isRequest() const noexcept { return kzr::isRequest(_op); }
        constexpr auto isResponse() const noexcept { return kzr::isResponse(_op); }
        constexpr auto getConceptualOperation() const noexcept { return kzr::convert(_op); }
        constexpr auto isError() const noexcept { return getConceptualOperation() == ConceptualOperation::Error; }
    private:
        Operation _op;
        uint16_t _tag;
};
enum class MessageDirection {
    Request,
    Response,
};
constexpr Operation translateConcept(ConceptualOperation op, MessageDirection dir) noexcept {
    if (op == ConceptualOperation::Undefined) {
        return dir == MessageDirection::Request ? Operation::TBad : Operation::RBad;
    } else {
        return (dir == MessageDirection::Request) ? getTMessageForm(op) : getRMessageForm(op);
    }

}

template<ConceptualOperation op, MessageDirection dir>
class Message : public MessageHeader {
    public:
        using Parent = MessageHeader;
        static constexpr Operation RawOperation = translateConcept(op, dir);
    public:
        explicit Message(uint16_t tag = notag) : Parent(RawOperation, tag) { }
        ~Message() override = default;
};
template<ConceptualOperation op>
using ResponseMessage = Message<op, MessageDirection::Response>;
template<ConceptualOperation op>
using RequestMessage = Message<op, MessageDirection::Request>;


template<MessageDirection dir>
class UndefinedMessage final : public Message< ConceptualOperation::Undefined, dir> {
    private:
        static constexpr bool isRequest() noexcept {
            return dir == MessageDirection::Request;
        }
    public:
        using Parent = Message<ConceptualOperation::Undefined, dir>;
    public:
        using Parent::Parent;
        ~UndefinedMessage() override = default;
        void encode(MessageStream&) const override {
            if constexpr (isRequest()) {
                throw Exception("Undefined request!");
            } else {
                throw Exception("Undefined response!");
            }
        }
        void decode(MessageStream&) override {
            if constexpr (isRequest()) {
                throw Exception("Undefined request!");
            } else {
                throw Exception("Undefined response!");
            }
        }
};
using UndefinedResponse = UndefinedMessage<MessageDirection::Response>;
using UndefinedRequest = UndefinedMessage<MessageDirection::Request>;
class ErrorResponse : public ResponseMessage<ConceptualOperation::Error> {
    public:
        using Parent = ResponseMessage<ConceptualOperation::Error>;
        static ErrorResponse make(uint16_t tag, const std::string& msg) noexcept {
            ErrorResponse r(tag);
            r.setErrorName(msg);
            return r;
        }
    public:
        using Parent::Parent;
        virtual ~ErrorResponse() = default;
        void encode(MessageStream&) const override;
        void decode(MessageStream&) override;
        /**
         * Get the error message
         */
        std::string getErrorName() const noexcept { return _ename; }
        /**
         * Set the error message
         */
        void setErrorName(const std::string& value) noexcept { _ename = value; }
    private:
        std::string _ename;
};
// Requesting errors does not make sense but this is here for regularity
using ErrorRequest = RequestMessage<ConceptualOperation::Error>;
template<MessageDirection dir>
class VersionMessage : public Message<ConceptualOperation::Version, dir> {
    public:
        using Parent = Message<ConceptualOperation::Version, dir>;
    public:
        VersionMessage() : Parent(notag) { }
        ~VersionMessage() override = default;
        void encode(MessageStream& msg) const override {
            Parent::encode(msg);
            msg << _msize << _version;
        }
        void decode(MessageStream& msg) override {
            Parent::decode(msg);
            msg >> _msize >> _version;
        }
        void setTag(uint16_t) noexcept override { }
        /**
         * Get the string representation of the 9p protocol version
         */
        auto getVersion() const noexcept { return _version; }
        /**
         * Get the total size of a message
         */
        constexpr auto getMsize() const noexcept { return _msize; }
        void setMsize(uint16_t msize) noexcept { _msize = msize; }
        void setVersion(const std::string& value) { _version = value; }
    private:
        std::string _version;
        uint16_t _msize;
    };
using VersionRequest = VersionMessage<MessageDirection::Request>;
using VersionResponse = VersionMessage<MessageDirection::Response>;
/**
 * Negotiate authentication information with the server.
 */
class AuthenticationRequest : public RequestMessage<ConceptualOperation::Auth> {
    public:
        using Parent = RequestMessage<ConceptualOperation::Auth>;
    public:
        using Parent::Parent;
        ~AuthenticationRequest() override = default;
        /**
         * Retrieves the special authentication handle
         */
        constexpr auto getAuthenticationHandle() const noexcept { return _afid; }
        void setAuthenticationHandle(uint32_t value) noexcept { _afid = value; }
        /**
         * Retrieve the name of the user attempting the connection
         */
        auto getUserName() const noexcept { return _uname; }
        void setUserName(const std::string& value) noexcept { _uname = value; }
        /**
         * Retrieve the mount point the user is trying to authentication against.
         */
        auto getAttachName() const noexcept { return _aname; }
        void setAttachName(const std::string& value) noexcept { _aname = value; }
        void encode(MessageStream&) const override;
        void decode(MessageStream&) override;
    private:
        uint32_t _afid;
        std::string _uname;
        std::string _aname;
};
/**
 * Response from the server when authentication is being used, otherwise it will be an error
 */
class AuthenticationResponse : public ResponseMessage<ConceptualOperation::Auth>, public HasQid {
    public:
        using Parent = ResponseMessage<ConceptualOperation::Auth>;
    public:
        using Parent::Parent;
        ~AuthenticationResponse() override = default;
        void encode(MessageStream&) const override;
        void decode(MessageStream&) override;
};

class FlushRequest : public RequestMessage<ConceptualOperation::Flush> {
    public:
        using Parent = RequestMessage<ConceptualOperation::Flush>;
    public:
        using Parent::Parent;
        ~FlushRequest() override = default;
        void encode(MessageStream&) const override;
        void decode(MessageStream&) override;
        constexpr auto getOldTag() const noexcept { return _oldtag; }
        void setOldTag(uint16_t value) noexcept { _oldtag = value; }
    private:
        uint16_t _oldtag;
};

using FlushResponse = ResponseMessage<ConceptualOperation::Flush>;

/**
 * Establishes a connection with the file server, the fid is a unique id selected
 * by the client.
 */
class AttachRequest : public RequestMessage<ConceptualOperation::Attach>, public HasFid {
    public:
        using Parent = RequestMessage<ConceptualOperation::Attach>;
    public:
        using Parent::Parent;
        ~AttachRequest() override = default;
        void encode(MessageStream&) const override;
        void decode(MessageStream&) override;
        constexpr auto getAuthenticationHandle() const noexcept { return _afid; }
        void setAuthenticationHandle(uint32_t value) noexcept { _afid = value; }
        /**
         * Retrieve the name of the user attempting the connection
         */
        auto getUserName() const noexcept { return _uname; }
        void setUserName(const std::string& value) noexcept { _uname = value; }
        /**
         * Retrieve the mount point the user is trying to authentication against.
         */
        auto getAttachName() const noexcept { return _aname; }
        void setAttachName(const std::string& value) noexcept { _aname = value; }
    private:
        uint32_t _afid;
        std::string _uname, _aname;
};
/**
 * Response from the server related to an attach request
 */
class AttachResponse : public ResponseMessage<ConceptualOperation::Attach>, public HasQid {
    
    public:
        using Parent = ResponseMessage<ConceptualOperation::Attach>;
    public:
        using Parent::Parent;
        ~AttachResponse() override = default;
        void encode(MessageStream&) const override;
        void decode(MessageStream&) override;
};


/**
 * Serves two purposes, directory traversal and fid cloning. When the path name is
 * is empty then it means to perform a fid clone
 */
class WalkRequest : public RequestMessage<ConceptualOperation::Walk>, public HasFid {
    public:
        using Parent = RequestMessage<ConceptualOperation:: Walk>; 
    public: 
        using Parent::Parent; 
        ~ WalkRequest () override = default ; 
        void encode(MessageStream&) const override; 
        void decode(MessageStream&) override;
        auto& getWname() noexcept { return _wname; }
        const auto& getWname() const noexcept { return _wname; }
        void setNewFid(uint32_t value) noexcept { _newfid = value; }
        constexpr auto getNewFid() const noexcept { return _newfid; }
        bool isDirectoryTraversal() const noexcept { return _wname.size() != 0; }
        bool isFidClone() const noexcept { return !isDirectoryTraversal(); }
    private:
        uint32_t _newfid;
        std::vector<std::string> _wname;
};

class WalkResponse : public ResponseMessage<ConceptualOperation::Walk> {
    public:
        using Parent = ResponseMessage<ConceptualOperation:: Walk>; 
    public: 
        using Parent::Parent; 
        ~ WalkResponse () override = default ; 
        void encode(MessageStream&) const override; 
        void decode(MessageStream&) override;
        auto& getWqid() noexcept { return _wqid; }
        const auto& getWqid() const noexcept { return _wqid; }
    private:
        std::vector<Qid> _wqid;
};

class OpenRequest : public RequestMessage<ConceptualOperation::Open>, public HasFid {
    public:
        using Parent = RequestMessage<ConceptualOperation:: Open>; 
    public: 
        using Parent::Parent; 
        ~OpenRequest() override = default ; 
        void encode(MessageStream&) const override; 
        void decode(MessageStream&) override;
        constexpr auto getMode() const noexcept { return _mode; }
        void setMode(uint8_t v) noexcept { _mode = v; }
    private:
        uint8_t _mode;
};

template<ConceptualOperation op>
class OpenOrCreateResponse : public ResponseMessage<op>, public HasQid {
    public:
        static_assert(op == ConceptualOperation::Create ||
                      op == ConceptualOperation::Open, "Illegal OpenOrCreateResponse kind!");
        using Parent = ResponseMessage<op>;
    public:
        using Parent::Parent;
        ~OpenOrCreateResponse() override = default;
        void encode(MessageStream& msg) const override {
            Parent::encode(msg);
            HasQid::encode(msg);
            msg << _iounit;
        }
        void decode(MessageStream& msg) override {
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
class CreateRequest : public RequestMessage<ConceptualOperation::Create>, public HasFid, public HasName {
    public:
        using Parent = RequestMessage<ConceptualOperation:: Create>; 
    public: 
        using Parent::Parent; 
        ~CreateRequest() override = default ; 
        void encode(MessageStream&) const override; 
        void decode(MessageStream&) override;
        constexpr auto getPermissions() const noexcept { return _perm; }
        void setPermissions(uint32_t v) noexcept { _perm = v; }
        constexpr auto getMode() const noexcept { return _mode; }
        void setMode(uint8_t v) noexcept { _mode = v; }
    private:
        uint32_t _perm;
        uint8_t _mode;
};

using CreateResponse = OpenOrCreateResponse<ConceptualOperation::Create>;

template<ConceptualOperation op>
class FidRequest : public RequestMessage<op>, public HasFid {
    public:
        using Parent = RequestMessage<op>;
    public:
        using Parent::Parent;
        ~FidRequest() override = default;
        void encode(MessageStream& msg) const override {
            Parent::encode(msg);
            HasFid::encode(msg);
        }
        void decode(MessageStream& msg) override {
            Parent::decode(msg);
            HasFid::decode(msg);
        }
};

class HasCount {
    public:
        constexpr auto getCount() const noexcept { return _count; }
        void setCount(uint32_t v) noexcept { _count = v; }
        void encode(MessageStream& msg) const { msg << _count; }
        void decode(MessageStream& msg) { msg >> _count; }
    private:
        uint32_t _count;
};
class HasOffset {
    public:
        constexpr auto getOffset() const noexcept { return _offset; }
        void setOffset(uint64_t v) noexcept { _offset = v; }
        void encode(MessageStream& msg) const { msg << _offset; }
        void decode(MessageStream& msg) { msg >> _offset; }
    private:
        uint64_t _offset;
};
class HasDataStorage {
    public:
        auto size() const noexcept { return _data.size(); }
        auto& getData() noexcept { return _data; }
        const auto& getData() const noexcept { return _data; }
        void encode(MessageStream& msg) const;
        void decode(MessageStream& msg);
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
        void encode(MessageStream& msg) const override {
            Parent::encode(msg);
            HasOffset::encode(msg);
        }
        void decode(MessageStream& msg) override {
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
        void encode(MessageStream& msg) const override;
        void decode(MessageStream& msg) override;
};

class ReadResponse : public ResponseMessage<ConceptualOperation::Read>, public HasDataStorage {
    public:
        using Parent = ResponseMessage<ConceptualOperation::Read>;
    public:
        using Parent::Parent;
        ~ReadResponse() override = default;
        void encode(MessageStream&) const override;
        void decode(MessageStream&) override;
};

class WriteRequest : public ReadWriteRequest<ConceptualOperation::Write>, public HasDataStorage {
    public:
        using Parent = ReadWriteRequest<ConceptualOperation::Write>;
    public:
        using Parent::Parent;
        ~WriteRequest() override = default;
        void encode(MessageStream&) const override;
        void decode(MessageStream&) override;
};

class WriteResponse : public ResponseMessage<ConceptualOperation::Write>, public HasCount {
    public:
        using Parent = ResponseMessage<ConceptualOperation::Write>;
    public:
        using Parent::Parent;
        ~WriteResponse() override = default;
        void encode(MessageStream&) const override;
        void decode(MessageStream&) override;
};

using ClunkRequest = FidRequest<ConceptualOperation::Clunk>;
using ClunkResponse = ResponseMessage<ConceptualOperation::Clunk>;
using RemoveRequest = FidRequest<ConceptualOperation::Remove>;
using RemoveResponse = ResponseMessage<ConceptualOperation::Remove>;
using StatRequest = FidRequest<ConceptualOperation::Stat>;
class StatResponse : public ResponseMessage<ConceptualOperation::Stat> {
    public:
        using Parent = ResponseMessage<ConceptualOperation::Stat>;
    public:
        using Parent::Parent;
        ~StatResponse() override = default;
        void encode(MessageStream&) const override;
        void decode(MessageStream&) override;
        const std::string& getData() const noexcept { return _data; }
        std::string& getData() noexcept { return _data; }
        void setData(const std::string& value) noexcept { _data = value; }
    private:
        std::string _data;
};
class WStatRequest : public RequestMessage<ConceptualOperation::WStat>, public HasFid {
    public:
        using Parent = RequestMessage<ConceptualOperation::WStat>;
    public:
        using Parent::Parent;
        ~WStatRequest() override = default;
        void encode(MessageStream&) const override;
        void decode(MessageStream&) override;
        Stat& getStat() noexcept { return _stat; }
        const Stat& getStat() const noexcept { return _stat; }
        void setStat(const Stat& stat) noexcept { _stat = stat; }
    private:
        Stat _stat;
};
using WStatResponse = ResponseMessage<ConceptualOperation::WStat>;

BindRequestResponseToTypes(Error, ErrorRequest, ErrorResponse);
BindRequestResponseToTypes(Version, VersionRequest, VersionResponse);
BindRequestResponseToTypes(Auth, AuthenticationRequest, AuthenticationResponse);
BindRequestResponseToTypes(Flush, FlushRequest, FlushResponse);
BindRequestResponseToTypes(Attach, AttachRequest, AttachResponse);
BindRequestResponseToTypes(Walk, WalkRequest, WalkResponse);
BindRequestResponseToTypes(Open, OpenRequest, OpenResponse);
BindRequestResponseToTypes(Create, CreateRequest, CreateResponse);
BindRequestResponseToTypes(Read, ReadRequest, ReadResponse);
BindRequestResponseToTypes(Write, WriteRequest, WriteResponse);
BindRequestResponseToTypes(Clunk, ClunkRequest, ClunkResponse);
BindRequestResponseToTypes(Remove, RemoveRequest, RemoveResponse);
BindRequestResponseToTypes(Stat, StatRequest, StatResponse);
BindRequestResponseToTypes(WStat, WStatRequest, WStatResponse);


} // end namespace kzr


#endif // end KZR_MESSAGE_H__

