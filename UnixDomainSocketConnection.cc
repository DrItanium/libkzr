/**
 * @file
 * A unix domain socket connection implementation.
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

#include "Exception.h"
#include "UnixDomainSocketConnection.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#include <csignal>
namespace kzr {
UnixDomainSocketConnection::UnixDomainSocketConnection() : Parent(SocketDomain::Unix, SocketType::Stream, 0) { }
UnixDomainSocketConnection::~UnixDomainSocketConnection() { }

void
UnixDomainSocketConnection::setupStructures(sockaddr_un& sa, socklen_t& salen) {
    memset(&sa, 0, sizeof(decltype(sa)));
    sa.sun_family = AF_UNIX;
    getAddress().copy(sa.sun_path, sizeof(sa.sun_path));
    salen = SUN_LEN(&sa);
}

void
UnixDomainSocketConnection::performDial() {
    sockaddr_un sa;
    socklen_t salen;

    setupStructures(sa, salen);
    if (::connect(getHandle(), (sockaddr*)&sa, salen)) {
        throw Exception("Could not connect to unix domain socket");
    } 
}

void
UnixDomainSocketConnection::performAnnounce() {
    sockaddr_un sa;
    socklen_t salen;
    constexpr auto maximumCacheSize = 32u;

    std::signal(SIGPIPE, SIG_IGN);
    setupStructures(sa, salen);
    const int yes = 1;
    if (setsockopt(getHandle(), SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes)) < 0) {
        throw Exception("Could not set socket options!");
    } 
    ::unlink(getAddress().c_str());
    if (bind(getHandle(), (sockaddr*)&sa, salen) < 0) {
        throw Exception("Could not bind socket!");
    } 
    ::chmod(getAddress().c_str(), S_IRWXU);
    if (::listen(getHandle(), maximumCacheSize) < 0) {
        throw Exception("Could not listen on socket!");
    }
}

} // end namespace kzr

