/**
 * @file
 * Routines related to the Operation enum class
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

#include "Operations.h"
#include "Message.h" // need some of the other encode operations
#include "Exception.h"


kzr::Message&
operator>>(kzr::Message& in, kzr::Operation& op) {
    uint8_t temp;
    in.decode(temp);
    op = static_cast<kzr::Operation>(temp);
    return in;
}

kzr::Message&
operator<<(kzr::Message& msg, kzr::Operation op) {
    msg.encode(uint8_t(op)); 
    return msg;
}

namespace kzr {
Operation
getTMessageForm(ConceptualOperation op) {
    switch (op) {
#define X(name, _) case ConceptualOperation:: name : return Operation:: T ## name 
        X(Version, 100);
        X(Auth, 102);
        X(Attach, 104);
        X(Error, 106);
        X(Flush, 108);
        X(Walk, 110);
        X(Open, 112);
        X(Create, 114);
        X(Read, 116);
        X(Write, 118);
        X(Clunk, 120);
        X(Remove, 122);
        X(Stat, 124);
        X(WStat, 126);
#undef X
        default:
            throw Exception("Bad ConceptualOperation!");
    }
}
Operation
getRMessageForm(ConceptualOperation op) {
    switch (op) {
#define X(name, _) case ConceptualOperation:: name : return Operation:: R ## name 
        X(Version, 100);
        X(Auth, 102);
        X(Attach, 104);
        X(Error, 106);
        X(Flush, 108);
        X(Walk, 110);
        X(Open, 112);
        X(Create, 114);
        X(Read, 116);
        X(Write, 118);
        X(Clunk, 120);
        X(Remove, 122);
        X(Stat, 124);
        X(WStat, 126);
#undef X
        default:
            throw Exception("Bad ConceptualOperation!");
    }
}
} // end namespace kzr

