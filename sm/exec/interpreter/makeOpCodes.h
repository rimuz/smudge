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
 *      File exec/interpreter/makeOpCodes.h
 *
*/

#ifndef _SM_EXEC__INTERPRETER__MAKEOPCODES_H
#define _SM_EXEC__INTERPRETER__MAKEOPCODES_H

#include "sm/exec/interpreter/defines.h"
#include "sm/runtime/casts.h"

namespace sm{
    namespace exec{
        _OcFunc(MakeVoidList){
            intp.exprStack.emplace_back(makeList(intp));
        }

        _OcFunc(MakeVoidTuple){
            intp.exprStack.emplace_back(makeTuple(intp));
        }

        _OcFunc(MakeRef){
            RootObject& tos = intp.exprStack.back();
            if(tos->type == ObjectType::WEAK_REFERENCE
                    || tos->type == ObjectType::STRONG_REFERENCE){
                tos->type = ObjectType::STRONG_REFERENCE;
            } else {
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("cannot get reference from temporary object ")
                    + runtime::errorString(intp, tos));
            }
        }

        _OcFunc(MakeList){
            unsigned size = ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]);

            RootObjectVec_t::iterator end = intp.exprStack.end();
            RootObjectVec_t::iterator first = end-size;
            RootObjectVec_t vec(
                std::make_move_iterator(first),
                std::make_move_iterator(end)
            );
            intp.exprStack.erase(first, end);

            RootObject list = makeList(intp, std::move(vec));
            intp.exprStack.emplace_back(std::move(list));
        }

        _OcFunc(MakeTuple){
            unsigned size = ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]);

            RootObjectVec_t::iterator end = intp.exprStack.end();
            RootObjectVec_t::iterator first = end-size;
            RootObjectVec_t vec(
                std::make_move_iterator(first),
                std::make_move_iterator(end)
            );
            intp.exprStack.erase(first, end);

            RootObject tuple = makeTuple(intp, std::move(vec));
            intp.exprStack.emplace_back(std::move(tuple));;
        }

        _OcFunc(MakeSuper){
            RootObject& super = intp.exprStack.back();
            RootObject& derived = *(intp.exprStack.end() -2);
            _OcValue(super);

            derived->o_ptr->c_ptr->bases.emplace_back(super->c_ptr);
            intp.exprStack.pop_back();
        }
    }
}

#endif
