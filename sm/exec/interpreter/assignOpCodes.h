                        /*
 *      Copyright 2016-2017 Riccardo Musso
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 *
 *      File exec/interpreter/assignOpCodes.h
 *
*/

#ifndef _SM__EXEC__INTERPRETER__ASSIGNOPCODES_H
#define _SM__EXEC__INTERPRETER__ASSIGNOPCODES_H

#include "sm/exec/interpreter/defines.h"
#include "sm/exec/interpreter/arithmeticOpCodes.h"
#include "sm/exec/interpreter/logicAndShiftsOpCodes.h"
#include "sm/exec/interpreter/exprStackOpCodes.h"
#include "sm/runtime/casts.h"

namespace sm{
    namespace exec{
        _OcFunc(Dup1);

        _OcFunc(Assign){
            _OcPopStore2;
            _OcValue(tos);

            if(tos1->type != ObjectType::WEAK_REFERENCE
                    && tos1->type != ObjectType::STRONG_REFERENCE){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("cannot perform 'operator=' to "
                    "temporary object \"")
                    + runtime::errorString(intp, tos1) + "\" (rvalue)");
            }

            tos1->refSet(tos);
            tos1->type = ObjectType::WEAK_REFERENCE;
            intp.exprStack.emplace_back(std::move(tos1));
        }

        _OcFunc(AssignAdd){
            Dup1(intp, {});
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(+, std::plus<float_t>(), parse::TT_PLUS, true,
                intp.exprStack.emplace_back(intp.start()));
            Assign(intp, {});
        }

        _OcFunc(AssignSub){
            Dup1(intp, {});
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(-, std::minus<float_t>(), parse::TT_MINUS, true,
                intp.exprStack.emplace_back(intp.start()));
            Assign(intp, {});
        }

        _OcFunc(AssignMul){
            Dup1(intp, {});
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(*, std::multiplies<float_t>(), parse::TT_MULT, true,
                intp.exprStack.emplace_back(intp.start()));
            Assign(intp, {});
        }

        _OcFunc(AssignDiv){
            Dup1(intp, {});
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(/, std::divides<float_t>(), parse::TT_DIV, true,
                intp.exprStack.emplace_back(intp.start()));
            Assign(intp, {});
        }

        _OcFunc(AssignMod){
            Dup1(intp, {});
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(%, std::fmod, parse::TT_MOD, true,
                intp.exprStack.emplace_back(intp.start()));
            Assign(intp, {});
        }

        _OcFunc(AssignOr){
            Dup1(intp, {});
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(|, _OcFloatError<or_str>(intp), parse::TT_OR, true,
                intp.exprStack.emplace_back(intp.start()));
            Assign(intp, {});
        }

        _OcFunc(AssignAnd){
            Dup1(intp, {});
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(&, _OcFloatError<and_str>(intp), parse::TT_AND, true,
                intp.exprStack.emplace_back(intp.start()));
            Assign(intp, {});
        }

        _OcFunc(AssignXor){
            Dup1(intp, {});
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(^, _OcFloatError<xor_str>(intp), parse::TT_XOR, true,
                intp.exprStack.emplace_back(intp.start()));
            Assign(intp, {});
        }

        _OcFunc(AssignLeftShift){
            Dup1(intp, {});
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(<<, _OcFloatError<left_shift_str>(intp), parse::TT_LEFT_SHIFT, true,
                intp.exprStack.emplace_back(intp.start()));
            Assign(intp, {});
        }

        _OcFunc(AssignRightShift){
            Dup1(intp, {});
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(>>, _OcFloatError<right_shift_str>(intp), parse::TT_RIGHT_SHIFT, true,
                intp.exprStack.emplace_back(intp.start()));
            Assign(intp, {});
        }

        _OcFunc(AssignNullPop){
            PushRef(intp, inst);
            PushNull(intp, {});
            Assign(intp, {});
            intp.exprStack.pop_back();
        }
    }
}

#endif
