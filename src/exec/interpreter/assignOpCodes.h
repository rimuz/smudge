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

#include "exec/interpreter/defines.h"
#include "exec/interpreter/arithmeticOpCodes.h"
#include "exec/interpreter/logicAndShiftsOpCodes.h"
#include "runtime/casts.h"

namespace sm{
    namespace exec{
        _OcFunc(Dup1);

        _OcFunc(Assign){
            ++addr;
            _OcPopStore2;
            _OcValue(tos);

            if(tos1.type != ObjectType::WEAK_REFERENCE
                    && tos1.type != ObjectType::STRONG_REFERENCE){
                intp.rt->sources.printStackTrace(intp, error::ERROR,
                    std::string("cannot perform 'operator=' to "
                    "temporary object \"")
                    + runtime::errorString(intp, tos1) + "\" (rvalue)");
            }

            tos1.refSet(tos);
            tos1.type = ObjectType::WEAK_REFERENCE;
            intp.exprStack.emplace_back(std::move(tos1));
        }

        _OcFunc(AssignAdd){
            ByteCode_t::const_iterator dummy;
            Dup1(intp, addr);
            Add(intp, dummy);
            Assign(intp, dummy);
        }

        _OcFunc(AssignSub){
            ByteCode_t::const_iterator dummy;
            Dup1(intp, addr);
            Sub(intp, dummy);
            Assign(intp, dummy);
        }

        _OcFunc(AssignMul){
            ByteCode_t::const_iterator dummy;
            Dup1(intp, addr);
            Mul(intp, dummy);
            Assign(intp, dummy);
        }

        _OcFunc(AssignDiv){
            ByteCode_t::const_iterator dummy;
            Dup1(intp, addr);
            Div(intp, dummy);
            Assign(intp, dummy);
        }

        _OcFunc(AssignMod){
            ByteCode_t::const_iterator dummy;
            Dup1(intp, addr);
            Mod(intp, dummy);
            Assign(intp, dummy);
        }

        _OcFunc(AssignOr){
            ByteCode_t::const_iterator dummy;
            Dup1(intp, addr);
            Or(intp, dummy);
            Assign(intp, dummy);
        }

        _OcFunc(AssignAnd){
            ByteCode_t::const_iterator dummy;
            Dup1(intp, addr);
            And(intp, dummy);
            Assign(intp, dummy);
        }

        _OcFunc(AssignXor){
            ByteCode_t::const_iterator dummy;
            Dup1(intp, addr);
            Xor(intp, dummy);
            Assign(intp, dummy);
        }

        _OcFunc(AssignLeftShift){
            ByteCode_t::const_iterator dummy;
            Dup1(intp, addr);
            LeftShift(intp, dummy);
            Assign(intp, dummy);
        }

        _OcFunc(AssignRightShift){
            ByteCode_t::const_iterator dummy;
            Dup1(intp, addr);
            RightShift(intp, dummy);
            Assign(intp, dummy);
        }
    }
}

#endif
