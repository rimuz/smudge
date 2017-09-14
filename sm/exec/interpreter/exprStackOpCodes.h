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

#include "sm/exec/interpreter/defines.h"
#include "sm/compile/defs.h"
#include "sm/runtime/casts.h"
#include "sm/runtime/Object.h"
#include "sm/runtime/id.h"

namespace sm{
    namespace lib {
        extern Class* cString;
    }

    namespace exec{
        _OcFunc(Pop){
            intp.stacks_m.lock();
            Object tos = std::move(intp.exprStack.back());
            intp.exprStack.pop_back();
            intp.stacks_m.unlock();
        }

        _OcFunc(PushInt0){
            std::lock_guard<std::mutex> lock(intp.stacks_m);
            Object obj;
            obj.type = ObjectType::INTEGER;
            obj.i = 0;
            intp.exprStack.emplace_back(obj);
        }

        _OcFunc(PushInt1){
            std::lock_guard<std::mutex> lock(intp.stacks_m);
            Object obj;
            obj.type = ObjectType::INTEGER;
            obj.i = 1;
            intp.exprStack.emplace_back(obj);
        }

        _OcFunc(PushNull){
            std::lock_guard<std::mutex> lock(intp.stacks_m);
            intp.exprStack.emplace_back(Object());
        }

        _OcFunc(Dup){
            std::lock_guard<std::mutex> lock(intp.stacks_m);
            intp.exprStack.emplace_back(intp.exprStack.back());
        }

        _OcFunc(Dup1){
            std::lock_guard<std::mutex> lock(intp.stacks_m);
            size_t sz = intp.exprStack.size();
            intp.exprStack.emplace_back(std::move(intp.exprStack[sz-1]));
            intp.exprStack[sz-1] = intp.exprStack[sz-2];
        }

        _OcFunc(PushInteger){
            std::lock_guard<std::mutex> lock(intp.stacks_m);
            Object obj;
            obj.type = ObjectType::INTEGER;
            obj.i = intp.rt->intConstants[(static_cast<uint16_t>(inst[1]) << 8) | inst[2]];
            intp.exprStack.emplace_back(obj);
        }

        _OcFunc(PushFloat){
            std::lock_guard<std::mutex> lock(intp.stacks_m);
            Object obj;
            obj.type = ObjectType::FLOAT;
            obj.f = intp.rt->floatConstants[(static_cast<uint16_t>(inst[1]) << 8) | inst[2]];
            intp.exprStack.emplace_back(obj);
        }

        _OcFunc(PushString){
            std::lock_guard<std::mutex> lock(intp.stacks_m);
            Object obj = makeString(intp.rt->stringConstants[(static_cast<uint16_t>(inst[1]) << 8) | inst[2]]);
            intp.exprStack.emplace_back(std::move(obj));
        }

        _OcFunc(PushIntValue){
            std::lock_guard<std::mutex> lock(intp.stacks_m);
            int16_t i16 = (static_cast<uint16_t>(inst[1]) << 8) | inst[2];
            Object obj = makeInteger(i16);
            intp.exprStack.emplace_back(std::move(obj));
        }

        _OcFunc(PushRef){
            intp.stacks_m.lock();
            ObjectDictVec_t& vec = intp.funcStack.back().codeBlocks;
            unsigned id = runtime::idsStart + ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]);
            ObjectDict_t::iterator oit;

            for(ObjectDictVec_t::reverse_iterator it = vec.rbegin(); it != vec.rend(); ++it){
                if(*it){
                    oit = (*it)->find(id);
                    if(oit != (*it)->end()){
                        Object ref;
                        ref.o_ptr = &oit->second;
                        ref.type = ObjectType::WEAK_REFERENCE;
                        intp.exprStack.emplace_back(std::move(ref));
                        intp.stacks_m.unlock();
                        return;
                    }
                }
            }

            Object& in = intp.funcStack.back().thisObject;
            if(in.type != ObjectType::NONE){
                oit = in.i_ptr->objects.find(id);

                if(oit != in.i_ptr->objects.end()){
                    Object ref;
                    if(oit->second.type == ObjectType::FUNCTION){
                        ref = makeMethod(in, &oit->second);
                    } else {
                        ref.o_ptr = &oit->second;
                        ref.type = ObjectType::WEAK_REFERENCE;
                    }
                    intp.exprStack.emplace_back(std::move(ref));
                    intp.stacks_m.unlock();
                    return;
                }

                std::vector<Class*> to_check {in.i_ptr->base};
                while(!to_check.empty()){
                    Class* base = to_check.back();
                    to_check.pop_back();

                    oit = base->objects.find(id);
                    if(oit != base->objects.end()){
                        Object ref;
                        if(oit->second.type == ObjectType::FUNCTION){
                            ref = makeMethod(in, &oit->second);
                        } else {
                            ref.o_ptr = &oit->second;
                            ref.type = ObjectType::WEAK_REFERENCE;
                        }
                        intp.exprStack.emplace_back(std::move(ref));
                        intp.stacks_m.unlock();
                        return;
                    }
                    to_check.insert(to_check.end(), base->bases.rbegin(), base->bases.rend());
                }
            }

            ObjectDict_t& objects = intp.funcStack.back().box->objects;
            oit = objects.find(id);

            if(oit == objects.end()){
                intp.stacks_m.unlock();
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("cannot find symbol '")
                    + intp.rt->nameFromId(id) + "'");
            }

            Object ref;
            ref.o_ptr = &oit->second;
            ref.type = ObjectType::WEAK_REFERENCE;
            intp.exprStack.emplace_back(std::move(ref));
            intp.stacks_m.unlock();
        }

        _OcFunc(PushThis){
            std::lock_guard<std::mutex> lock(intp.stacks_m);
            intp.exprStack.emplace_back(intp.funcStack.back().thisObject);
        }

        _OcFunc(PushBox){
            std::lock_guard<std::mutex> lock(intp.stacks_m);
            Object box;
            box.type = ObjectType::BOX;
            box.c_ptr = intp.funcStack.back().box;
            intp.exprStack.emplace_back(std::move(box));
        }

        _OcFunc(PushClass){
            std::lock_guard<std::mutex> lock(intp.stacks_m);
            if(intp.funcStack.back().thisObject.type == ObjectType::CLASS_INSTANCE){
                Object clazz;
                clazz.type = ObjectType::CLASS;
                clazz.c_ptr = intp.funcStack.back().thisObject.i_ptr->base;
                intp.exprStack.emplace_back(std::move(clazz));
            } else {
                intp.exprStack.emplace_back(Object());
            }
        }

        _OcFunc(Find){
            intp.stacks_m.lock();
            Object& ref = intp.exprStack.back();
            Object obj = (ref.type == ObjectType::WEAK_REFERENCE
                    || ref.type == ObjectType::STRONG_REFERENCE) ? ref.refGet() : ref;
            unsigned id = runtime::idsStart + ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]);

            switch(obj.type){
                case ObjectType::BOX: {
                    ObjectDict_t::iterator it = obj.c_ptr->objects.find(id);
                    if(it != obj.c_ptr->objects.end()){
                        ref.o_ptr = &it->second;
                        ref.type = ObjectType::WEAK_REFERENCE;
                        intp.stacks_m.unlock();
                        return;
                    }

                    intp.stacks_m.unlock();
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
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
                        intp.stacks_m.unlock();
                        return;
                    }

                    std::vector<Class*> to_check {obj.i_ptr->base};
                    while(!to_check.empty()){
                        Class* base = to_check.back();
                        to_check.pop_back();

                        it = base->objects.find(id);
                        if(it != base->objects.end()){
                            if(it->second.type == ObjectType::FUNCTION){
                                ref = makeMethod(obj, &it->second);
                            } else {
                                ref.o_ptr = &it->second;
                                ref.type = ObjectType::WEAK_REFERENCE;
                            }
                            intp.stacks_m.unlock();
                            return;
                        }

                        to_check.insert(to_check.end(), base->bases.rbegin(), base->bases.rend());
                    }

                    intp.stacks_m.unlock();
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("cannot find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
                }

                case ObjectType::STRING:{
                    ObjectDict_t::iterator it = lib::cString->objects.find(id);
                    if(it != lib::cString->objects.end()){
                        ref = makeMethod(obj, &it->second);
                        intp.stacks_m.unlock();
                        return;
                    }

                    intp.stacks_m.unlock();
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("cannot find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
                }

                case ObjectType::ENUM:{
                    ObjectDict_t::iterator it = obj.e_ptr->values.find(id);
                    if(it != obj.e_ptr->values.end()){
                        ref.o_ptr = &it->second;
                        ref.type = ObjectType::WEAK_REFERENCE;
                        intp.stacks_m.unlock();
                        return;
                    }

                    intp.stacks_m.unlock();
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("cannot find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
                }

                default:
                    intp.stacks_m.unlock();
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("cannoted find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
            }
        }

        _OcFunc(Iterate){
            intp.stacks_m.lock();
            Object tos = intp.exprStack.back();
            _OcValue(tos);
            intp.stacks_m.unlock();

            bool isString = tos.type == ObjectType::STRING;
            if(tos.type != ObjectType::CLASS_INSTANCE && !isString){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    "for-each supported only for objects (no PODs)");
            }

            Object func;
            Function* f_ptr;

            if(!isString){
                if(!runtime::find<ObjectType::CLASS_INSTANCE>(tos, func, lib::idIterate)){
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("cannot find 'iterate' in ")
                        + runtime::errorString(intp, tos) + " (required by for-each)");
                } else if(!runtime::callable(func, tos, f_ptr)){
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("'iterate' is not a function in ")
                        + runtime::errorString(intp, tos) + " (required by for-each)");
                }
            } else {
                runtime::find<ObjectType::STRING>(tos, func, lib::idIterate);
                runtime::callable(func, tos, f_ptr);
            }

            intp.makeCall(f_ptr, {}, tos);
        }

        _OcFunc(ItNext){
            intp.stacks_m.lock();
            Object tos = intp.exprStack.back();
            _OcValue(tos);
            intp.stacks_m.unlock();

            if(tos.type != ObjectType::CLASS_INSTANCE){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    "'iterate()' did not return a class instance.");
            }

            Object func;
            Function* f_ptr;
            if(!runtime::find<ObjectType::CLASS_INSTANCE>(tos, func, lib::idNext)){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("cannot find 'next' in ")
                    + runtime::errorString(intp, tos) + " (required by for-each)");
            } else if(!runtime::callable(func, tos, f_ptr)){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("'next' is not a function in ")
                    + runtime::errorString(intp, tos) + " (required by for-each)");
            }

            intp.makeCall(f_ptr, {}, tos);
        }

        _OcFunc(FindSuper){
            unsigned id = runtime::idsStart + ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]);

            intp.stacks_m.lock();
            Object& ref = intp.exprStack.back();
            _OcValue(ref);

            if(ref.type != ObjectType::INTEGER){
                intp.stacks_m.unlock();
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("expected integer inside super call "
                        "(found ") + runtime::errorString(intp, ref)
                        + " instead)");
            } else if(ref.i < 0){
                intp.stacks_m.unlock();
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    "super call's integer must be greater than zero");
            }
            unsigned super = ref.i;

            auto& back = intp.funcStack.back();
            if(back.thisObject.type == ObjectType::NONE){
                intp.stacks_m.unlock();
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("'super' is <null>, unable to find '")
                    + intp.rt->nameFromId(id) + "'");
            }

            std::vector<Class*>& supers = back.thisObject.i_ptr->base->bases;

            if(supers.size() <= super){
                intp.stacks_m.unlock();
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("class '")
                    + intp.rt->nameFromId(back.thisObject.i_ptr->base->name)
                    + "' has not super number " + std::to_string(super));
            }

            ObjectDict_t::iterator it;
            std::vector<Class*> to_check {supers[super]};
            while(!to_check.empty()){
                Class* base = to_check.back();
                to_check.pop_back();

                it = base->objects.find(id);
                if(it != base->objects.end()){
                    if(it->second.type == ObjectType::FUNCTION){
                        ref = makeMethod(back.thisObject, &it->second);
                    } else {
                        // should never happen
                        ref.o_ptr = &it->second;
                        ref.type = ObjectType::WEAK_REFERENCE;
                    }
                    intp.stacks_m.unlock();
                    return;
                }
                to_check.insert(to_check.end(), base->bases.rbegin(), base->bases.rend());
            }

            intp.stacks_m.unlock();
            intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                std::string("cannot find function '")
                + intp.rt->nameFromId(id) + "' in <super> of class '"
                + intp.rt->nameFromId(supers[super]->name) + "'");
        }
    }
}

#endif
