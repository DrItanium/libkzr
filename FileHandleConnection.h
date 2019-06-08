/**
 * @file
 * Connection which sends and recieves bytes to a file handle, automatically
 * closed on destruction
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

#ifndef KZR_FILE_HANDLE_CONNECTION_H__
#define KZR_FILE_HANDLE_CONNECTION_H__
#include <string>
#include "Connection.h"
namespace kzr {

class FileHandleConnection : public Connection {
    public:
        using Parent = Connection;
    public:
        FileHandleConnection(int fd, bool destroy = true);
        virtual ~FileHandleConnection();
        constexpr auto destroyOnDestruction() const noexcept { return _destroy; }
        constexpr auto getHandle() const noexcept { return _handle; }
    protected:
        [[nodiscard]] virtual size_t rawWrite(const std::string& data) override;
        [[nodiscard]] virtual size_t rawRead(std::string& data) override;
    private:
        int _handle;
        bool _destroy;

};

} // end namespace kzr

#endif // end KZR_FILE_HANDLE_CONNECTION_H__
