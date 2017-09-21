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


#include "sm/exec/interpreter/defines.h"
#include "sm/runtime/casts.h"
#include "sm/runtime/id.h"

namespace sm{
    namespace exec{
        _OcFunc(DefineVar){
            unsigned id = ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]) + runtime::idsStart;
            _OcPopStore(tos);
            _OcSimplifyRef(tos);

            RootObjectDict_t*& dict = intp.funcStack.back().codeBlocks.back();
            if(!dict)
                dict = new RootObjectDict_t;
            RootObjectDict_t::const_iterator it = dict->find(id);

            if(it != dict->end()){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("redeclaration of variable named '") + intp.rt->nameFromId(id)
                    + "' in the same scope");
            }

            intp.exprStack.emplace_back(makeRef((*dict)[id] = tos));
        }

        _OcFunc(DefineGlobalVar){
            unsigned id = ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]) + runtime::idsStart;

            _OcPopStore(tos);
            _OcSimplifyRef(tos);

            auto& back = intp.funcStack.back();
            if(back.thisObject->type == ObjectType::NONE){
                RootObjectDict_t& dict = back.box->objects;
                RootObjectDict_t::const_iterator it = dict.find(id);

                if(it != dict.end())
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("redeclaration of variable named '") + intp.rt->nameFromId(id)
                        + "' in the class " + intp.rt->boxNames[intp.funcStack.back().box->name]
                        + "::" + intp.rt->nameFromId(back.thisObject->i_ptr->base->name)
                    );

                intp.exprStack.emplace_back(makeRef(dict[id] = tos));
            } else {
                ObjectDict_t& dict = back.thisObject->i_ptr->objects;
                ObjectDict_t::const_iterator it = dict.find(id);

                if(it != dict.end())
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("redeclaration of variable named '") + intp.rt->nameFromId(id)
                        + "' in the box " + intp.rt->boxNames[intp.funcStack.back().box->name]
                    );

                intp.exprStack.emplace_back(makeRef(dict[id] = tos));
            }
        }

        _OcFunc(DefineNullVar){
            unsigned id = ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]) + runtime::idsStart;

            RootObjectDict_t*& dict = intp.funcStack.back().codeBlocks.back();
            if(!dict)
                dict = new RootObjectDict_t;
            RootObjectDict_t::const_iterator it = dict->find(id);

            if(it != dict->end()){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("redeclaration of variable named '") + intp.rt->nameFromId(id)
                    + "' in the same scope");
            }

            intp.exprStack.emplace_back(makeRef((*dict)[id] = nullptr));
        }

        _OcFunc(DefineGlobalNullVar){
            unsigned id = ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]) + runtime::idsStart;

            auto& back = intp.funcStack.back();
            if(back.thisObject->type == ObjectType::NONE){
                RootObjectDict_t& dict = back.box->objects;
                RootObjectDict_t::const_iterator it = dict.find(id);

                if(it != dict.end())
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("redeclaration of variable named '") + intp.rt->nameFromId(id)
                        + "' in the class " + intp.rt->boxNames[intp.funcStack.back().box->name]
                        + "::" + intp.rt->nameFromId(back.thisObject->i_ptr->base->name)
                    );
                intp.exprStack.emplace_back(makeRef(dict[id] = nullptr));
            } else {
                ObjectDict_t& dict = back.thisObject->i_ptr->objects;
                ObjectDict_t::const_iterator it = dict.find(id);

                if(it != dict.end())
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("redeclaration of variable named '") + intp.rt->nameFromId(id)
                        + "' in the box " + intp.rt->boxNames[intp.funcStack.back().box->name]
                    );
                intp.exprStack.emplace_back(makeRef(dict[id] = nullptr));
            }
        }
    }
}

#endif
