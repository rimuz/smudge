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
 *      File exec/interpreter/defineOpCodes.h
 *
*/

#ifndef _SM__EXEC__INTERPRETER__DEFINEOPCODES_H
#define _SM__EXEC__INTERPRETER__DEFINEOPCODES_H


#include "exec/interpreter/defines.h"
#include "runtime/casts.h"
#include "runtime/id.h"

namespace sm{
    namespace exec{
        _OcFunc(DefineVar){
            unsigned id = ((*++addr << 8) | *++addr) + runtime::idsStart;
            ++addr;

            _OcPopStore(tos);
            _OcSimplifyRef(tos);

            ObjectDict_t*& dict = intp.funcStack.back().codeBlocks.back();
            if(!dict)
                dict = new ObjectDict_t;
            ObjectDict_t::const_iterator it = dict->find(id);

            if(it != dict->end()){
                intp.rt->sources.printStackTrace(intp, error::ERROR,
                    std::string("redeclaration of variable named '") + intp.rt->nameFromId(id)
                    + "' in the same scope");
            }

            Object ref;
            ref.o_ptr = &((*dict)[id] = tos);
            ref.type = ObjectType::WEAK_REFERENCE;
            intp.exprStack.emplace_back(std::move(ref));
        }

        _OcFunc(DefineGlobalVar){
            unsigned id = ((*++addr << 8) | *++addr) + runtime::idsStart;
            ++addr;

            _OcPopStore(tos);
            _OcSimplifyRef(tos);

            auto& back = intp.funcStack.back();
            ObjectDict_t& dict = back.thisObject.type == ObjectType::NONE ?
                back.box->objects : back.thisObject.i_ptr->objects;
            ObjectDict_t::const_iterator it = dict.find(id);

            if(it != dict.end()){
                intp.rt->sources.printStackTrace(intp, error::ERROR,
                    std::string("redeclaration of variable named '") + intp.rt->nameFromId(id)
                    + (back.thisObject.type == ObjectType::NONE ?
                        (std::string("' in the class ")
                        + intp.rt->boxNames[intp.funcStack.back().box->boxName]
                        + "::" + intp.rt->nameFromId(back.thisObject.i_ptr->base->name))
                      : (std::string("' in the box ")
                        + intp.rt->boxNames[intp.funcStack.back().box->boxName])
                    ));
            }

            Object ref;
            ref.o_ptr = &(dict[id] = tos);
            ref.type = ObjectType::WEAK_REFERENCE;
            intp.exprStack.emplace_back(std::move(ref));
        }

        _OcFunc(DefineNullVar){
            unsigned id = ((*++addr << 8) | *++addr) + runtime::idsStart;
            ++addr;

            ObjectDict_t*& dict = intp.funcStack.back().codeBlocks.back();
            if(!dict)
                dict = new ObjectDict_t;
            ObjectDict_t::const_iterator it = dict->find(id);

            if(it != dict->end()){
                intp.rt->sources.printStackTrace(intp, error::ERROR,
                    std::string("redeclaration of variable named '") + intp.rt->nameFromId(id)
                    + "' in the same scope");
            }


            Object ref;
            ref.o_ptr = &((*dict)[id] = Object());
            ref.type = ObjectType::WEAK_REFERENCE;
            intp.exprStack.emplace_back(std::move(ref));
        }

        _OcFunc(DefineGlobalNullVar){
            unsigned id = ((*++addr << 8) | *++addr) + runtime::idsStart;
            ++addr;

            auto& back = intp.funcStack.back();
            ObjectDict_t& dict = back.thisObject.type == ObjectType::NONE ?
                back.box->objects : back.thisObject.i_ptr->objects;
            ObjectDict_t::const_iterator it = dict.find(id);

            if(it != dict.end()){
                intp.rt->sources.printStackTrace(intp, error::ERROR,
                    std::string("redeclaration of variable named '") + intp.rt->nameFromId(id)
                    + (back.thisObject.type == ObjectType::NONE ?
                        (std::string("' in the class ")
                        + intp.rt->boxNames[intp.funcStack.back().box->boxName]
                        + "::" + intp.rt->nameFromId(back.thisObject.i_ptr->base->name))
                      : (std::string("' in the box ")
                        + intp.rt->boxNames[intp.funcStack.back().box->boxName])
                    ));
            }

            Object ref;
            ref.o_ptr = &(dict[id] = Object());
            ref.type = ObjectType::WEAK_REFERENCE;
            intp.exprStack.emplace_back(std::move(ref));
        }
    }
}

#endif
