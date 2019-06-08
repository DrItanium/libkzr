/**
 * @file
 * Generic class decl
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

#include "Connection.h"
#include <sstream>


namespace kzr {
void
Connection::write(const Message& msg) {
    // grab the string representation and construct a new string with it at the
    // front
    auto contents = msg.str();
    if (auto len = contents.length(); len != (uint32_t(len))) {
        throw Exception("length of the message is too long to write out!");
    } else {
        auto acutalLen = uint32_t(len) + 4; // need to include the four bytes for the length field
        std::ostringstream newStr;
        // shove the size before
        newStr.put(uint8_t(actualLen));
        newStr.put(uint8_t(actualLen >> 8));
        newStr.put(uint8_t(actualLen >> 16));
        newStr.put(uint8_t(actualLen >> 24));
        newStr << contents;
        rawWrite(newStr.str());
    }
}
void
Connection::read(Message& msg) {
    // need to call rawRead twice, first to get the length, then the second
    // time to actually ingest the data
    std::string sizeAcquire(' ', 4);
    if (auto bytesRead = rawRead(sizeAcquire); bytesRead != 4) {
        throw Exception("Expected to read 4 bytes but only read ", bytesRead, "!");
    } else {
        // now we build our size field
        if (uint32_t messageSize = build(uint8_t(sizeAcquire[0]), uint8_t(sizeAcquire[1]), uint8_t(sizeAcquire[2]), uint8_t(sizeAcquire[3])); messageSize < 4) {
            throw Exception("Expected at least 4 bytes as the size of the message!");
        } else {
            auto correctedMessageSize = messageSize - 4;
            // now we reserve this storage inside our new string
            std::string storage(' ', correctedMessageSize);
            if (auto bytesRead2 = rawRead(storage); bytesRead2 != messageSize) {
                throw Exception("only able to read ", bytesRead2, "/", messageSize, " bytes!");
            } else {
                if ((bytesRead2 + bytesRead) != messageSize) {
                    throw Exception("Not enough bytes read!");
                } else {
                    msg.write(storage);
                }
            }
        }
    }
}
} // end namespace kzr
