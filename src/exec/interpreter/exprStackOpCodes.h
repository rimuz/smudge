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
 *      File exec/interpreter/exprStackOpCodes.h
 *
*/

#ifndef _SM__EXEC__INTERPRETER__EXPRSTACKOPCODES_H
#define _SM__EXEC__INTERPRETER__EXPRSTACKOPCODES_H

#include <utility>

#include "exec/interpreter/defines.h"
#include "runtime/casts.h"
#include "runtime/Object.h"
#include "runtime/id.h"

namespace sm{
    namespace lib {
        extern Class* cString;
    }

    namespace exec{
        _OcFunc(Pop){
           intp.exprStack.pop_back();
           ++addr;
        }

        _OcFunc(PushInt0){
            Object obj;
            obj.type = ObjectType::INTEGER;
            obj.i = 0;
            intp.exprStack.emplace_back(obj);
            ++addr;
        }

        _OcFunc(PushInt1){
            Object obj;
            obj.type = ObjectType::INTEGER;
            obj.i = 1;
            intp.exprStack.emplace_back(obj);
            ++addr;
        }

        _OcFunc(PushNull){
            intp.exprStack.emplace_back(Object());
            ++addr;
        }

        _OcFunc(Dup){
            intp.exprStack.emplace_back(intp.exprStack.back());
            ++addr;
        }

        _OcFunc(Dup1){
            size_t sz = intp.exprStack.size();
            intp.exprStack.emplace_back(std::move(intp.exprStack[sz-1]));
            intp.exprStack[sz-1] = intp.exprStack[sz-2];
            ++addr;
        }

        _OcFunc(PushInteger){
            Object obj;
            obj.type = ObjectType::INTEGER;
            obj.i = intp.rt->intConstants[(*++addr << 8) | *++addr];
            intp.exprStack.emplace_back(obj);
            ++addr;
        }

        _OcFunc(PushFloat){
            Object obj;
            obj.type = ObjectType::FLOAT;
            obj.f = intp.rt->floatConstants[(*++addr << 8) | *++addr];
            intp.exprStack.emplace_back(obj);
            ++addr;
        }

        _OcFunc(PushString){
            Object obj = makeString(intp.rt->stringConstants[(*++addr << 8) | *++addr]);
            intp.exprStack.emplace_back(std::move(obj));
            ++addr;
        }

        _OcFunc(PushRef){
            ObjectDictVec_t& vec = intp.funcStack.back().codeBlocks;
            unsigned id = runtime::idsStart + ((*++addr << 8) | *++addr);
            ObjectDict_t::iterator oit;
            ++addr;

            for(ObjectDictVec_t::reverse_iterator it = vec.rbegin(); it != vec.rend(); ++it){
                if(*it){
                    oit = (*it)->find(id);
                    if(oit != (*it)->end()){
                        Object ref;
                        ref.o_ptr = &oit->second;
                        ref.type = ObjectType::WEAK_REFERENCE;
                        intp.exprStack.emplace_back(std::move(ref));
                        return;
                    }
                }
            }

            ObjectDict_t& objects = intp.funcStack.back().box->objects;

            oit = objects.find(id);
            if(oit == objects.end()){
                intp.rt->sources.printStackTrace(intp, error::ERROR, std::string("cannot find symbol '")
                    + intp.rt->nameFromId(id) + "'");
                runtime::Runtime_t::exit(1);
            }

            Object ref;
            ref.o_ptr = &oit->second;
            ref.type = ObjectType::WEAK_REFERENCE;
            intp.exprStack.emplace_back(std::move(ref));
        }

        _OcFunc(PushThis){
            // TODO
            ++addr;
        }

        _OcFunc(PushSuper){
            // TODO
            ++addr;
        }

        _OcFunc(Find){
            Object& ref = intp.exprStack.back();
            Object obj = (ref.type == ObjectType::WEAK_REFERENCE
                    || ref.type == ObjectType::STRONG_REFERENCE) ? ref.refGet() : ref;
            unsigned id = runtime::idsStart + ((*++addr << 8) | *++addr);
            ++addr;

            switch(obj.type){
                case ObjectType::BOX: {
                    ObjectDict_t::iterator it = obj.c_ptr->objects.find(id);
                    if(it != obj.c_ptr->objects.end()){
                        ref.o_ptr = &it->second;
                        ref.type = ObjectType::WEAK_REFERENCE;
                        return;
                    }

                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("cannot find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
                }

                case ObjectType::CLASS_INSTANCE: {
                    ObjectDict_t::iterator it = obj.i_ptr->objects.find(id);
                    if(it != obj.i_ptr->objects.end()){
                        if(it->second.type == ObjectType::FUNCTION){
                            ref = makeMethod(obj, &it->second);
                        } else {
                            ref.o_ptr = &it->second;
                            ref.type = ObjectType::WEAK_REFERENCE;
                        }
                        return;
                    }

                    Class* base = obj.i_ptr->base;
                    it = base->objects.find(id);
                    if(it != base->objects.end()){
                        ref = makeMethod(obj, &it->second);
                        return;
                    }

                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("cannot find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
                }

                case ObjectType::STRING:{
                    ObjectDict_t::iterator it = lib::cString->objects.find(id);
                    if(it != lib::cString->objects.end()){
                        ref = makeMethod(obj, &it->second);
                        return;
                    }

                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("cannot find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
                }

                case ObjectType::ENUM:{
                    ObjectDict_t::iterator it = obj.e_ptr->values.find(id);
                    if(it != obj.e_ptr->values.end()){
                        ref.o_ptr = &it->second;
                        ref.type = ObjectType::WEAK_REFERENCE;
                        return;
                    }

                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("cannot find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
                }

                default:
                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("cannot find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
            }
        }
    }
}

#endif
