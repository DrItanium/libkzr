/**
 * @file
 * Abstract interactions with message types
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
#include "Interaction.h"
#include "Connection.h"
kzr::MessageStream& 
operator<<(kzr::MessageStream& msg, const kzr::Request& request) {
    std::visit([&msg](auto&& value) { msg << value; }, request);
    return msg;
}
kzr::MessageStream& 
operator<<(kzr::MessageStream& msg, const kzr::Response& request) {
    std::visit([&msg](auto&& value) { msg << value; }, request);
    return msg;
}
kzr::MessageStream& 
operator>>(kzr::MessageStream& msg, kzr::Request& request) {
    // decoding is a bit harder actually, we have to figure out the type first and emplace that into memory
    if (auto op = msg.peek(); op) {
        switch (convert(kzr::Operation(*op))) {
#define X(name, _) \
            case kzr::ConceptualOperation:: name : \
                 request.emplace<kzr::BoundRequestType<kzr::ConceptualOperation:: name>>(); \
                 msg.decode(std::get<kzr::BoundRequestType<kzr::ConceptualOperation:: name>>(request)); \
                break ;
            KZR_PROTOCOL_KINDS
#undef X
            default:
                throw kzr::Exception("Illegal type found!");
        }
        return msg;
    } else {
        throw kzr::Exception("Cannot deduce type because message is not in a good state!");
    }
}

kzr::MessageStream& 
operator>>(kzr::MessageStream& msg, kzr::Response& request) {
    // decoding is a bit harder actually, we have to figure out the type first and emplace that into memory
    if (auto op = msg.peek(); op) {
        switch (convert(kzr::Operation(*op))) {
#define X(name, _) \
            case kzr::ConceptualOperation:: name : \
                 request.emplace<kzr::BoundResponseType<kzr::ConceptualOperation:: name>>(); \
                 msg.decode(std::get<kzr::BoundResponseType<kzr::ConceptualOperation:: name>>(request)); \
                break;
            KZR_PROTOCOL_KINDS
#undef X
            default:
                throw kzr::Exception("Illegal type found!");
        }
        return msg;
    } else {
        throw kzr::Exception("Cannot deduce type because message is not in a good state!");
    }
}

kzr::MessageStream&
operator<<(kzr::MessageStream& msg, const kzr::Interaction& thing) {
    std::visit([&msg](auto&& value) { msg << value; }, thing); 
    return msg;
}
kzr::MessageStream&
operator>>(kzr::MessageStream& msg, kzr::Interaction& thing) {
    if (auto op = msg.peek(); op) {
        if (auto lookup = kzr::Operation(*op); kzr::isRequest(lookup)) {
            thing.emplace<kzr::Request>();
            msg >> std::get<kzr::Request>(thing);
        } else {
            // its a response so act accordingly
            thing.emplace<kzr::Response>();
            msg >> std::get<kzr::Response>(thing);
        }
        return msg;
    } else {
        throw kzr::Exception("Cannot deduce type because message is not in a good state!");
    }
}

