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
#include <sys/types.h>
#include <sys/socket.h>
#include "SocketConnection.h"
#include "Exception.h"
namespace kzr {
SocketConnection::SocketConnection(SocketDomain dom, SocketType typ, int protocol) : Parent(socket(int(dom), int(typ), protocol), true), _domain(dom), _type(typ), _protocol(protocol), _mode(SocketMode::Undefined)  { }

SocketConnection::~SocketConnection() { }

void
SocketConnection::dial(const std::string& addr) {
    if (_mode != SocketMode::Undefined) {
        throw Exception("SocketConnection's mode is already set");
    } else {
        if (!isValidHandle()) {
            throw Exception("Provided socket is not valid! Cannot dial!");
        }
        _mode = SocketMode::ConnectTo;
        _address = addr;
        performDial();
    }
}
void
SocketConnection::announce(const std::string& addr) {
    if (_mode != SocketMode::Undefined) {
        throw Exception("SocketConnection's mode is already set");
    } else {
        if (!isValidHandle()) {
            throw Exception("Provided socket is not valid! Cannot announce!");
        }
        _mode = SocketMode::Listen;
        _address = addr;
        performAnnounce();
    }
}
} // end namespace kzr
