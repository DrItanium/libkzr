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
#ifndef KZR_MESSAGE_H__
#define KZR_MESSAGE_H__
#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <array>

namespace kzr {
/**
 * Generic version of decoding messages, it assumes the class/struct has a
 * decode method.
 * @param in the input stream to read from
 * @param data the value to write out to
 * @return the input stream
 */
template<typename T>
std::istream& decode(std::istream& in, T& data) {
    data.decode(in);
    return in;
}
/**
 * Generic version of encoding messages, it assumes the class/struct has an
 * encode method.
 * @param out the stream to write out to
 * @param data the data to write to the stream
 * @return the output stream
 */
template<typename T>
std::ostream& encode(std::ostream& out, const T& data) {
    data.encode(out);
    return out;
}

std::istream& decode(std::istream&, uint8_t&);
std::ostream& encode(std::ostream&, uint8_t);
std::istream& decode(std::istream&, uint16_t&);
std::ostream& encode(std::ostream&, uint16_t);
std::istream& decode(std::istream&, uint32_t&);
std::ostream& encode(std::ostream&, uint32_t);
std::istream& decode(std::istream&, uint64_t&);
std::ostream& encode(std::ostream&, uint64_t);
std::istream& decode(std::istream&, std::string&);
std::ostream& encode(std::ostream&, const std::string&);
template<typename T, size_t capacity>
std::istream& decode(std::istream& in, std::array<T, capacity>& collection) {
    for (auto& ptr : collection) {
        decode(in, ptr);
    }
    return in;
}
template<typename T, size_t capacity>
std::ostream& encode(std::ostream& out, const std::array<T, capacity>& collection) {
    for (const auto& ptr : collection) {
        encode(out, ptr);
    }
    return out;
}
} // end namespace kzr
#endif // end KZR_MESSAGE_H__

