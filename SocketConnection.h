/**
 * @file
 * A file handle based connection which refers to an open socket.
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

#ifndef KZR_SOCKET_CONNECTION_H__
#define KZR_SOCKET_CONNECTION_H__
#include <sys/types.h>
#include <sys/socket.h>
#include "FileHandleConnection.h"
namespace kzr {
/** 
 * Wrapper around the C socket domain kinds.
 */
enum class SocketDomain : int {
    Unix = AF_UNIX,
    Local = AF_LOCAL,
    IPv4 = AF_INET,
    IPv6 = AF_INET6,
    IPX = AF_IPX,
    Netlink = AF_NETLINK,
    X25 = AF_X25,
    AX25 = AF_AX25,
    ATMPVC = AF_ATMPVC,
    AppleTalk = AF_APPLETALK,
    LowLevelPacket = AF_PACKET,
    KernelCryptoAPI = AF_ALG,
};
enum class SocketType : int {
    /**
     * Provides sequenced, reliable, two-way, connection-based byte streams.
     */
#define X(alias, raw) \
    alias = raw , \
    NonBlocking ## alias = raw | SOCK_NONBLOCK,  \
    alias ## CloseOnExec = raw | SOCK_CLOEXEC, \
    NonBlocking ## alias ## CloseOnExec = raw | SOCK_NONBLOCK | SOCK_CLOEXEC
    X(Stream, SOCK_STREAM),
    X(Datagram, SOCK_DGRAM),
    X(SequencedPacket, SOCK_SEQPACKET),
    X(Raw, SOCK_RAW),
    X(ReliableDatagram, SOCK_RDM),
#undef X
};
constexpr auto isNonBlocking(SocketType t) noexcept {
    return (int(t) & SOCK_NONBLOCK) != 0;
}
constexpr auto isCloseOnExec(SocketType t) noexcept {
    return (int(t) & SOCK_CLOEXEC) != 0;
}
class SocketConnection : public FileHandleConnection {
    public:
        using Parent = FileHandleConnection;
        enum class SocketMode {
            Undefined,
            ConnectTo,
            Listen,
        };
    public:
        SocketConnection(SocketDomain domain, SocketType type, int protocol = 0);
        virtual ~SocketConnection();
        constexpr auto getDomain() const noexcept { return _domain; }
        constexpr auto getType() const noexcept { return _type; }
        constexpr auto getProtocol() const noexcept { return _protocol; }
        void dial(const std::string& address);
        void announce(const std::string& address);
        const std::string& getAddress() const noexcept { return _address; }
    protected:
        virtual void performDial() = 0;
        virtual void performAnnounce() = 0;
    private:
        SocketDomain _domain;
        SocketType _type;
        int _protocol;
        SocketMode _mode;
        std::string _address;
};

} // end namespace kzr

#endif // end KZR_SOCKET_CONNECTION_H__
