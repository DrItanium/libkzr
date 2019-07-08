/**
 * @file
 * Describes the abstract concept of a server
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

#include "Server.h"


namespace kzr {
    Server::Server(Connection& conn) : _conn(conn) { }
    Response 
    Server::process(const Request& input) noexcept {
        return std::visit([this](auto&& value) noexcept { return process(value); }, input);
    }
    Response 
    Server::process(const OpenRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "open unimplemented");
    }
    Response 
    Server::process(const WalkRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "walk unimplemented");
    }
    Response 
    Server::process(const VersionRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "version unimplemented");
    }
    Response 
    Server::process(const ReadRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "read unimplemented");
    }
    Response 
    Server::process(const StatRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "stat unimplemented");
    }
    Response 
    Server::process(const WriteRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "write unimplemented");
    }
    Response 
    Server::process(const ClunkRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "clunk unimplemented");
    }
    Response 
    Server::process(const FlushRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "flush unimplemented");
    }
    Response 
    Server::process(const AuthenticationRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "auth unimplemented");
    }
    Response 
    Server::process(const AttachRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "attach unimplemented");
    }
    Response 
    Server::process(const CreateRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "create unimplemented");
    }
    Response 
    Server::process(const RemoveRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "remove unimplemented");
    }
    Response 
    Server::process(const ErrorRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "illegal request of an error!");
    }
    Response 
    Server::process(const UndefinedRequest& req) noexcept {
        return ErrorResponse::make(req.getTag(), "Undefined request type!");
    }
    Request 
    Server::recieve() {
        Request r;
        _conn >> r;
        return r;
    }
    void 
    Server::reply(const Response& resp) {
        _conn << resp;
    }

    void 
    Server::invoke() noexcept {
        _isRunning = true;
        while(isRunning()) {
            reply(process(recieve()));
        }
    }
} // end namespace kzr
