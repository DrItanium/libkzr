/**
 * @file
 * Describes the set of 9p2000 operations
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
#ifndef KZR_CORE_H__
#define KZR_CORE_H__
namespace kzr {

template<typename T, typename R, T mask, T shift = static_cast<T>(0)>
[[nodiscard]] constexpr R decodeBits(T value) noexcept {
    return static_cast<R>((value & mask) >> shift);
}

template<typename T, typename R>
[[nodiscard]] constexpr R decodeBits(T value, T mask, T shift = static_cast<T>(0)) noexcept {
    return static_cast<R>((value & mask) >> shift);
}

template<typename T, typename R, T mask, T shift = static_cast<T>(0)>
[[nodiscard]] constexpr T encodeBits(T value, R insert) noexcept {
    return static_cast<T>((value & (~mask)) | ((static_cast<T>(insert) << shift) & mask));
}

template<typename T, typename R>
[[nodiscard]] constexpr T encodeBits(T value, R insert, T mask, T shift = static_cast<T>(0)) noexcept {
    return static_cast<T>((value & (~mask)) | ((static_cast<T>(insert) << shift) & mask));
}

} // end namespace kzr

#endif // end KZR_CORE_H__
