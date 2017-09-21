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
 *      File exec/interpreter/jumpOintp.pcodes.h
 *
*/

#ifndef _SM__EXEC__INTERPRETER__JUMPOPCODES_H
#define _SM__EXEC__INTERPRETER__JUMPOPCODES_H

#include "sm/exec/interpreter/defines.h"
#include "sm/runtime/casts.h"

namespace sm{
    namespace lib {
        extern Class* cTuple;
    }

    namespace exec{
        _OcFunc(JumpF){
            intp.pc += ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]) -1;
        }

        _OcFunc(JumpB){
            intp.pc -= ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]) +1;
        }

        _OcFunc(JumpIfF){
            _OcPopStore(tos);
            _OcValue(tos);

            if(runtime::implicitToBool(tos))
                intp.pc += ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]) -1;
        }

        _OcFunc(JumpIfB){
            _OcPopStore(tos);
            _OcValue(tos);

            if(runtime::implicitToBool(tos))
                intp.pc -= ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]) +1;
        }

        _OcFunc(JumpIfNotF){
            _OcPopStore(tos);
            _OcValue(tos);

            if(!runtime::implicitToBool(tos))
                intp.pc += ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]) -1;
        }

        _OcFunc(JumpIfNotB){
            _OcPopStore(tos);
            _OcValue(tos);

            if(!runtime::implicitToBool(tos))
                intp.pc -= ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]) +1;
        }

        _OcFunc(Elvis){
            if(intp.exprStack.back()->type == ObjectType::NONE){
                intp.exprStack.pop_back();
                return;
            }
            intp.pc += ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]) -1;
        }

        _OcFunc(SwitchCase){
            Dup1(intp, {});
            Equal(intp, {});

            _OcPopStore(tos);
            _OcValue(tos);

            if(!runtime::implicitToBool(tos))
                intp.pc += ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]) -1;
        }

        _OcFunc(ForeachCheck){
            RootObject& tos = intp.exprStack.back();
            ObjectVec_t* vec;

            if(!hasVector(intp, tos, vec) || vec->size() != 2){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    "'next()' must return a list or a tuple of size 2.");
            }

            RootObjectVec_t::iterator end = intp.exprStack.end();
            if(!runtime::implicitToBool((*vec)[1])){
                intp.exprStack.erase(end -4, end);
                intp.pc += ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]) -1;
                return;
            }

            RootObject obj = (*vec)[0];
            if(obj->type == ObjectType::WEAK_REFERENCE){
                obj = obj->refGet();
            } else if(obj->type == ObjectType::STRONG_REFERENCE){
                obj->type = ObjectType::WEAK_REFERENCE;
            }

            *(*(end -4))->o_ptr = std::move(obj);
            intp.exprStack.pop_back();
        }

        _OcFunc(ThrowException){
            // TODO
        }
    }
}

#endif
