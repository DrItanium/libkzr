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



MessageHeader::MessageHeader(Operation op, uint16_t tag) : _op(op), _tag(tag) { }
MessageHeader::MessageHeader(Operation op) : MessageHeader(op, -1) { }
void 
MessageHeader::encode(MessageStream& msg) const {
    msg << _op << _tag;
}
void 
MessageHeader::decode(MessageStream& msg) {
    msg >> _op >> _tag;
}

void
VersionBody::encode(MessageStream& msg) const {
    msg << _msize << _version;
}

void
VersionBody::decode(MessageStream& msg) {
    msg >> _msize >> _version;
}


void 
ErrorResponse::encode(MessageStream& msg) const {
    Parent::encode(msg);
    msg << _ename;
}
void
ErrorResponse::decode(MessageStream& msg) {
    Parent::decode(msg);
    msg >> _ename;
}

void
Qid::encode(MessageStream& msg) const {
    msg << _type << _version << _path;
}

void
Qid::decode(MessageStream& msg) {
    msg >> _type >> _version >> _path;
}

Qid::Qid(uint8_t t, uint64_t path, uint32_t version) : _type(t), _version(version), _path(path) { }

void
Stat::encode(MessageStream& msg) const {
    // need to construct the inner message and then tack the stat onto the front
    MessageStream innerMessage;
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
Stat::decode(MessageStream& msg) {
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
AuthenticationRequest::encode(MessageStream& msg) const {
    Parent::encode(msg);
    msg << _afid << _uname << _aname;
}

void
AuthenticationRequest::decode(MessageStream& msg) {
    Parent::decode(msg);
    msg >> _afid >> _uname >> _aname;
}

void
AuthenticationResponse::encode(MessageStream& msg) const {
    Parent::encode(msg);
    HasQid::encode(msg);
}
void
AuthenticationResponse::decode(MessageStream& msg) {
    Parent::decode(msg);
    HasQid::decode(msg);
}
void HasQid::encode(MessageStream& msg) const { msg << _qid; }
void HasQid::decode(MessageStream& msg) { msg >> _qid; }
void HasFid::encode(MessageStream& msg) const { msg << _fid; }
void HasFid::decode(MessageStream& msg) { msg >> _fid; }

void
FlushRequest::encode(MessageStream& msg) const {
    Parent::encode(msg);
    msg << _oldtag;
}
void
FlushRequest::decode(MessageStream& msg) {
    Parent::decode(msg);
    msg >> _oldtag;
}

void
AttachRequest::encode(MessageStream& msg) const {
    Parent::encode(msg);
    HasFid::encode(msg);
    msg << _afid << _uname << _aname;
}
void
AttachRequest::decode(MessageStream& msg) {
    Parent::decode(msg);
    HasFid::decode(msg);
    msg >> _afid >> _uname >> _aname;
}

void
AttachResponse::encode(MessageStream& msg) const {
    Parent::encode(msg);
    HasQid::encode(msg);
}
void
AttachResponse::decode(MessageStream& msg) {
    Parent::decode(msg);
    HasQid::decode(msg);
}

void
WalkRequest::encode(MessageStream& msg) const {
    Parent::encode(msg);
    HasFid::encode(msg);
    msg << _newfid << _wname;
}

void
WalkRequest::decode(MessageStream& msg) {
    Parent::decode(msg);
    HasFid::decode(msg);
    msg >> _newfid >> _wname;
}

void
WalkResponse::encode(MessageStream& msg) const {
    Parent::encode(msg);
    msg << _wqid;
}

void
WalkResponse::decode(MessageStream& msg) {
    Parent::decode(msg);
    msg >> _wqid;
}


void
OpenRequest::encode(MessageStream& msg) const {
    Parent::encode(msg);
    HasFid::encode(msg);
    msg << _mode;
}

void
OpenRequest::decode(MessageStream& msg) {
    Parent::decode(msg);
    HasFid::decode(msg);
    msg >> _mode;
}

void
CreateRequest::encode(MessageStream& msg) const {
    Parent::encode(msg);
    HasFid::encode(msg);
    HasName::encode(msg);
    msg << _perm << _mode;
}

void
CreateRequest::decode(MessageStream& msg) {
    Parent::decode(msg);
    HasFid::decode(msg);
    HasName::decode(msg);
    msg >> _perm >> _mode;
}

void
ReadRequest::encode(MessageStream& msg) const {
    Parent::encode(msg);
    HasCount::encode(msg);
}

void
ReadRequest::decode(MessageStream& msg) {
    Parent::decode(msg);
    HasCount::decode(msg);
}

void
ReadResponse::encode(MessageStream& msg) const {
    Parent::encode(msg);
    HasDataStorage::encode(msg);
}

void
ReadResponse::decode(MessageStream& msg) {
    Parent::decode(msg);
    HasDataStorage::decode(msg);
}

void
WriteRequest::encode(MessageStream& msg) const {
    Parent::encode(msg);
    HasDataStorage::encode(msg);
}

void
WriteRequest::decode(MessageStream& msg) {
    Parent::decode(msg);
    HasDataStorage::decode(msg);
}

void
WriteResponse::encode(MessageStream& msg) const {
    Parent::encode(msg);
    HasCount::encode(msg);
}

void
WriteResponse::decode(MessageStream& msg) {
    Parent::decode(msg);
    HasCount::decode(msg);
}

void
StatResponse::encode(MessageStream& msg) const {
    Parent::encode(msg);
    msg << _data;
}

void
StatResponse::decode(MessageStream& msg) {
    Parent::decode(msg);
    msg >> _data;
}

void
WStatRequest::encode(MessageStream& msg) const {
    Parent::encode(msg);
    HasFid::encode(msg);
    msg << _stat;
}
void
WStatRequest::decode(MessageStream& msg) {
    Parent::decode(msg);
    HasFid::decode(msg);
    msg >> _stat;
}

void
HasDataStorage::encode(MessageStream& msg) const {
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
HasDataStorage::decode(MessageStream& msg) {
    auto size = msg.decode<uint32_t>();
    _data.reserve(size);
    for (decltype(size) i = 0; i < size; ++i) {
        _data.emplace_back(msg.decode<uint8_t>());
    }
}

void
HasName::encode(MessageStream& msg) const {
    msg << _name; 
}

void
HasName::decode(MessageStream& msg) {
    msg >> _name; 
}

} // end namespace kzr

