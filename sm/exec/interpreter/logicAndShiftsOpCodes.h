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
*      File exec/interpreter/logicAndShiftOpCodes.h
*
*/

#ifndef _SM__EXEC__INTERPRETER__LOGICANDSHIFTOPCODES_H
#define _SM__EXEC__INTERPRETER__LOGICANDSHIFTOPCODES_H

#include "sm/exec/interpreter/defines.h"
#include "sm/runtime/casts.h"
#include "sm/runtime/id.h"
#include "sm/typedefs.h"

namespace sm{
    namespace exec{
        template <const char Operator[]>
        class _OcFloatError {
        public:
            const Interpreter* intp;

            _OcFloatError(const Interpreter& in) : intp(&in) {}

            inline float operator() (float_t a, float_t b){
                intp->rt->sources.printStackTrace(*intp, error::ET_ERROR,
                    std::string("cannot perform 'operator") + Operator
                    + "' with floating-point numbers.");
                return 0.f;
            }
        };

        constexpr char or_str[] = "|";
        constexpr char and_str[] = "&";
        constexpr char xor_str[] = "^";
        constexpr char left_shift_str[] = "<<";
        constexpr char right_shift_str[] = ">>";

        _OcFunc(Or){
            ++addr;
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(|, _OcFloatError<or_str>(intp), parse::TT_OR, false,);
        }

        _OcFunc(And){
            ++addr;
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(&, _OcFloatError<and_str>(intp), parse::TT_AND, false,);
        }

        _OcFunc(Xor){
            ++addr;
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(^, _OcFloatError<xor_str>(intp), parse::TT_XOR, false,);
        }

        _OcFunc(LeftShift){
            ++addr;
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(<<, _OcFloatError<left_shift_str>(intp), parse::TT_LEFT_SHIFT, false,);
        }

        _OcFunc(RightShift){
            ++addr;
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(>>, _OcFloatError<right_shift_str>(intp), parse::TT_RIGHT_SHIFT, false,);
        }

        _OcFunc(LogicAnd){
            Object tos = intp.exprStack.back();
            _OcValue(tos);

            if(!runtime::implicitToBool(tos)){
                addr += (*++addr << 8) | *++addr;
                return;
            }

            addr += 3;
            intp.exprStack.pop_back();
        }

        _OcFunc(LogicOr){
            Object tos = intp.exprStack.back();
            _OcValue(tos);

            if(!runtime::implicitToBool(tos)){
                addr += 3;
                intp.exprStack.pop_back();
                return;
            }

            addr += (*++addr << 8) | *++addr;
        }
    }
}

#endif
