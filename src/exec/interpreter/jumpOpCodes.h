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
 *      File exec/interpreter/jumpOpCodes.h
 *
*/

#ifndef _SM__EXEC__INTERPRETER__JUMPOPCODES_H
#define _SM__EXEC__INTERPRETER__JUMPOPCODES_H

#include "exec/interpreter/defines.h"
#include "runtime/casts.h"

namespace sm{
    namespace exec{
        _OcFunc(JumpF){
            addr += (*++addr << 8) | *++addr;
        }

        _OcFunc(JumpB){
            addr -= (*++addr << 8) | *++addr;
        }

        _OcFunc(JumpIfF){
            _OcPopStore(tos);
            _OcValue(tos);

            if(!runtime::implicitToBool(tos)){
                addr += 3;
                return;
            }

            addr += (*++addr << 8) | *++addr;
        }

        _OcFunc(JumpIfB){
            _OcPopStore(tos);
            _OcValue(tos);

            if(!runtime::implicitToBool(tos)){
                addr += 3;
                return;
            }

            addr -= (*++addr << 8) | *++addr;
        }

        _OcFunc(JumpIfNotF){
            _OcPopStore(tos);
            _OcValue(tos);

            if(!runtime::implicitToBool(tos)){
                addr += (*++addr << 8) | *++addr;
                return;
            }

            addr += 3;
        }

        _OcFunc(JumpIfNotB){
            _OcPopStore(tos);
            _OcValue(tos);

            if(!runtime::implicitToBool(tos)){
                addr -= (*++addr << 8) | *++addr;
                return;
            }

            addr += 3;
        }

        _OcFunc(Elvis){
            const Object& tos = intp.exprStack.back();

            if(tos.type == ObjectType::NONE){
                intp.exprStack.pop_back();
                addr += 3;
                return;
            }
            addr += (*++addr << 8) | *++addr;
        }

        _OcFunc(SwitchCase){
            ByteCode_t::const_iterator dummy;
            Dup1(intp, dummy);
            Equal(intp, dummy);

            _OcPopStore(tos);
            _OcValue(tos);

            if(!runtime::implicitToBool(tos)){
                addr += (*++addr << 8) | *++addr;
                return;
            }

            addr += 3;
        }

        _OcFunc(ThrowException){
            // TODO
            ++addr;
        }
    }
}

#endif
