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
#ifndef KZR_OPERATIONS_H__
#define KZR_OPERATIONS_H__
#include <cstdint>
#include <istream>
#include <ostream>
#include <tuple>
namespace kzr {
    enum class Operation : uint8_t {
        TBad = 0, // these are unused codes so we can just reuse them here
        RBad = 1,
#define X(kind, value) \
        T ## kind = value, \
        R ## kind
        X(Version, 100),
        X(Auth, 102),
        X(Attach, 104),
        X(Error, 106),
        X(Flush, 108),
        X(Walk, 110),
        X(Open, 112),
        X(Create, 114),
        X(Read, 116),
        X(Write, 118),
        X(Clunk, 120),
        X(Remove, 122),
        X(Stat, 124),
        X(WStat, 126),
#undef X
    };

    /**
     * The operation type with the transmit/recieve information stripped away
     */
    enum class ConceptualOperation : uint8_t {
        Undefined,
#define X(kind, value) kind
        X(Version, 100),
        X(Auth, 102),
        X(Attach, 104),
        X(Error, 106),
        X(Flush, 108),
        X(Walk, 110),
        X(Open, 112),
        X(Create, 114),
        X(Read, 116),
        X(Write, 118),
        X(Clunk, 120),
        X(Remove, 122),
        X(Stat, 124),
        X(WStat, 126),
#undef X
    };
template<Operation op>
constexpr auto OperationToConceptualOperation = ConceptualOperation::Undefined;

template<ConceptualOperation op>
constexpr auto ConceptualOperationToTOperation = Operation::RError;
template<ConceptualOperation op>
constexpr auto ConceptualOperationToROperation = Operation::RError;
#define X(kind, value) \
        template<> constexpr auto OperationToConceptualOperation< Operation::T ## kind > = ConceptualOperation:: kind ; \
        template<> constexpr auto OperationToConceptualOperation< Operation::R ## kind > = ConceptualOperation:: kind ; \
        template<> constexpr auto ConceptualOperationToTOperation < ConceptualOperation :: kind > = Operation::T ## kind ; \
        template<> constexpr auto ConceptualOperationToROperation < ConceptualOperation :: kind > = Operation::R ## kind 
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
    constexpr ConceptualOperation convert(Operation op) noexcept {
        switch (op) {
#define X(name, _) \
            case Operation::T ## name : \
            case Operation::R ## name : \
                 return ConceptualOperation:: name
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
                return ConceptualOperation::Undefined;
        }
    }
    constexpr Operation getTMessageForm(ConceptualOperation op) noexcept {
        switch (op) {
#define X(name, _) \
            case ConceptualOperation:: name: \
            return Operation::T ## name 
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
                return Operation::TBad;
        }
    }
    constexpr Operation getRMessageForm(ConceptualOperation op) noexcept {
        switch (op) {
#define X(name, _) \
            case ConceptualOperation:: name: \
            return Operation::R ## name 
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
                return Operation::RBad;
        }
    }

    template<typename T>
    constexpr auto isEven(T value) noexcept {
        return (value & 1) == 0;
    }
    template<typename T>
    constexpr auto isOdd(T value) noexcept {
        return !isEven(value);
    }
    constexpr auto isRequest(Operation op) noexcept {
        return isEven<uint8_t>(static_cast<uint8_t>(op));
    }
    constexpr auto isResponse(Operation op) noexcept {
        return isOdd<uint8_t>(static_cast<uint8_t>(op));
    }
    constexpr auto isSessionClass(ConceptualOperation op) noexcept {
        switch (op) {
            case ConceptualOperation::Version:
            case ConceptualOperation::Auth:
            case ConceptualOperation::Attach:
            case ConceptualOperation::Flush:
            case ConceptualOperation::Error:
                return true;
            default:
                return false;
        }
    }
    constexpr auto isSessionClass(Operation op) noexcept {
        return isSessionClass(convert(op));
    }
    constexpr auto isFileClass(ConceptualOperation op) noexcept {
        switch (op) {
            case ConceptualOperation::Walk:
            case ConceptualOperation::Open:
            case ConceptualOperation::Create:
            case ConceptualOperation::Read:
            case ConceptualOperation::Write:
            case ConceptualOperation::Clunk:
                return true;
            default:
                return false;
        }
    }
    constexpr auto isFileClass(Operation op) noexcept { 
        return isFileClass(convert(op)); 
    }
    constexpr auto isMetadataClass(ConceptualOperation op) noexcept {
        switch (op) {
            case ConceptualOperation::Stat:
            case ConceptualOperation::WStat:
                return true;
            default:
                return false;
        }
    }
    constexpr auto isMetadataClass(Operation op) noexcept {
        return isMetadataClass(convert(op));
    }
    constexpr auto expectedResponseKind(Operation op) noexcept {
        if (isResponse(op)) {
            return Operation::RError;
        } else {
            // must be a transmit message
            switch (op) {
#define X(kind) case Operation:: T ## kind : return Operation:: R ## kind
                X(Version);
                X(Auth);
                X(Attach);
                X(Error);
                X(Flush);
                X(Walk);
                X(Open);
                X(Create);
                X(Read);
                X(Write);
                X(Clunk);
                X(Remove);
                X(Stat);
                X(WStat);
#undef X
                default: return Operation::RError;
            }
        }
    }
    class Message;
} // end namespce kzr
kzr::Message& operator<<(kzr::Message&, kzr::Operation);
kzr::Message& operator>>(kzr::Message&, kzr::Operation&);
#endif // end KZR_OPERATIONS_H__
