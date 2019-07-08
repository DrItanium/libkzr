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
#ifndef KZR_SERVER_H__
#define KZR_SERVER_H__
#include "Operations.h"
#include "Interaction.h"
#include "Message.h"
#include "Connection.h"
namespace kzr {
class Server {
    public:
        virtual ~Server() = default;
        Server(Connection& connection);
        Response process(const Request& input) noexcept;
        virtual Response process(const OpenRequest&) noexcept;
        virtual Response process(const WalkRequest&) noexcept;
        virtual Response process(const VersionRequest&) noexcept;
        virtual Response process(const ReadRequest&) noexcept;
        virtual Response process(const StatRequest&) noexcept;
        virtual Response process(const WriteRequest&) noexcept;
        virtual Response process(const ClunkRequest&) noexcept;
        virtual Response process(const FlushRequest&) noexcept;
        virtual Response process(const AuthenticationRequest&) noexcept;
        virtual Response process(const AttachRequest&) noexcept;
        virtual Response process(const CreateRequest&) noexcept;
        virtual Response process(const RemoveRequest&) noexcept;
        void invoke() noexcept;
        void stopExecution() noexcept { _isRunning = false; }
        constexpr auto isRunning() const noexcept { return _isRunning; }
    private:
        Request recieve();
        void reply(const Response&);
        Response process(const ErrorRequest&) noexcept; 
        Response process(const UndefinedRequest&) noexcept;
    private:
        Connection& _conn;
        bool _isRunning = false;
};
} // end namespace kzr
#endif // end KZR_SERVER_H__
