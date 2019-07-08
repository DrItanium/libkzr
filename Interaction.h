/**
 * @file
 * Abstraction over message types
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
#ifndef KZR_CLIENT_H__
#define KZR_CLIENT_H__
#include <variant>
#include <functional>
#include "Message.h"
#include "Connection.h"
namespace kzr {
class Connection;
using Response = std::variant<
UndefinedResponse
#define X(name, _) ,BoundResponseType<ConceptualOperation:: name >
KZR_PROTOCOL_KINDS
#undef X
    >;

using Request = std::variant<
UndefinedRequest
#define X(name, _) , BoundRequestType<ConceptualOperation:: name >
KZR_PROTOCOL_KINDS
#undef X
    >;

/// A top level return and encoding type that client and servers send off
using Interaction = std::variant<Response, Request>;

using RecieveInteraction = std::function<Interaction()>;
using SendInteraction = std::function<void(const Interaction&)>;
} // end namespace kzr

kzr::MessageStream& operator<<(kzr::MessageStream&, const kzr::Request&);
kzr::MessageStream& operator>>(kzr::MessageStream&, kzr::Request&);
kzr::MessageStream& operator<<(kzr::MessageStream&, const kzr::Response&);
kzr::MessageStream& operator>>(kzr::MessageStream&, kzr::Response&);
kzr::MessageStream& operator<<(kzr::MessageStream&, const kzr::Interaction&);
kzr::MessageStream& operator>>(kzr::MessageStream&, kzr::Interaction&);
template<typename T>
kzr::Connection& operator<<(kzr::Connection& conn, const T& req) {
    kzr::MessageStream ms;
    conn << (ms << req);
    return conn;
}
template<typename T>
kzr::Connection& operator>>(kzr::Connection& conn, T& req) {
    kzr::MessageStream ms;
    conn >> ms;
    ms >> req;
    return conn;
}
#endif // end KZR_CLIENT_H__
