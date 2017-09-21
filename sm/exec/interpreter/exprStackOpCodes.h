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
            intp.exprStack.pop_back();
        }

        _OcFunc(PushInt0){
            intp.exprStack.emplace_back(makeInteger(0));
        }

        _OcFunc(PushInt1){
            intp.exprStack.emplace_back(makeInteger(1));
        }

        _OcFunc(PushNull){
            intp.exprStack.emplace_back(nullptr);
        }

        _OcFunc(Dup){
            intp.exprStack.emplace_back(intp.exprStack.back());
        }

        _OcFunc(Dup1){
            size_t sz = intp.exprStack.size();
            intp.exprStack.emplace_back(std::move(intp.exprStack[sz-1]));
            intp.exprStack[sz-1] = intp.exprStack[sz-2];
        }

        _OcFunc(PushInteger){
            intp.exprStack.emplace_back(makeInteger((static_cast<uint16_t>(inst[1]) << 8) | inst[2]));
        }

        _OcFunc(PushFloat){
            intp.exprStack.emplace_back(makeFloat((static_cast<uint16_t>(inst[1]) << 8) | inst[2]));
        }

        _OcFunc(PushString){
            Object obj = makeString(intp.rt->stringConstants[(static_cast<uint16_t>(inst[1]) << 8) | inst[2]]);
            intp.exprStack.emplace_back(std::move(obj));
        }

        _OcFunc(PushIntValue){
            int16_t i16 = (static_cast<int16_t>(inst[1]) << 8) | inst[2];
            intp.exprStack.emplace_back(makeInteger(static_cast<integer_t>(i16)));
        }

        _OcFunc(PushRef){
            RootObjectDictVec_t& vec = intp.funcStack.back().codeBlocks;
            unsigned id = runtime::idsStart + ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]);
            RootObjectDict_t::iterator oit;
            ObjectDict_t::iterator oit2;

            for(RootObjectDictVec_t::reverse_iterator it = vec.rbegin(); it != vec.rend(); ++it){
                if(*it){
                    oit = (*it)->find(id);
                    if(oit != (*it)->end()){
                        intp.exprStack.emplace_back(makeRef(oit->second));
                        return;
                    }
                }
            }

            RootObject& in = intp.funcStack.back().thisObject;
            if(in->type != ObjectType::NONE){
                oit2 = in->i_ptr->objects.find(id);

                if(oit2 != in->i_ptr->objects.end()){
                    intp.exprStack.emplace_back(
                        oit2->second.type == ObjectType::FUNCTION
                        ? makeMethod(in, &oit2->second)
                        : makeRef(oit2->second)
                    );
                    return;
                }

                std::vector<Class*> to_check {in->i_ptr->base};
                while(!to_check.empty()){
                    Class* base = to_check.back();
                    to_check.pop_back();

                    oit2 = base->objects.find(id);
                    if(oit2 != base->objects.end()){
                        intp.exprStack.emplace_back(
                            oit2->second.type == ObjectType::FUNCTION
                            ? makeMethod(in, &oit2->second)
                            : makeRef(oit2->second)
                        );
                        return;
                    }
                    to_check.insert(to_check.end(), base->bases.rbegin(), base->bases.rend());
                }
            }

            RootObjectDict_t& objects = intp.funcStack.back().box->objects;
            oit = objects.find(id);

            if(oit == objects.end()){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("cannot find symbol '")
                    + intp.rt->nameFromId(id) + "'");
            }
            intp.exprStack.emplace_back(makeRef(oit->second));
        }

        _OcFunc(PushThis){
            intp.exprStack.emplace_back(intp.funcStack.back().thisObject);
        }

        _OcFunc(PushBox){
            intp.exprStack.emplace_back(makeBox(intp.funcStack.back().box));
        }

        _OcFunc(PushClass){
            if(intp.funcStack.back().thisObject->type == ObjectType::CLASS_INSTANCE){
                intp.exprStack.emplace_back(makeClass(intp.funcStack.back().thisObject->i_ptr->base));
            } else {
                intp.exprStack.emplace_back(nullptr);
            }
        }

        _OcFunc(Find){
            RootObject& ref = intp.exprStack.back();
            RootObject obj = (ref->type == ObjectType::WEAK_REFERENCE
                    || ref->type == ObjectType::STRONG_REFERENCE) ? RootObject(ref->refGet()) : ref;
            unsigned id = runtime::idsStart + ((static_cast<uint16_t>(inst[1]) << 8) | inst[2]);

            switch(obj->type){
                case ObjectType::BOX: {
                    RootObjectDict_t::iterator it = obj->b_ptr->objects.find(id);
                    if(it != obj->b_ptr->objects.end()){
                        ref = makeRef(it->second);
                        return;
                    }

                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("cannot find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
                }

                case ObjectType::CLASS_INSTANCE: {
                    ObjectDict_t::iterator it = obj->i_ptr->objects.find(id);
                    if(it != obj->i_ptr->objects.end()){
                        ref = it->second.type == ObjectType::FUNCTION
                            ? makeMethod(obj, &it->second) : makeRef(it->second);
                        return;
                    }

                    std::vector<Class*> to_check {obj->i_ptr->base};
                    while(!to_check.empty()){
                        Class* base = to_check.back();
                        to_check.pop_back();

                        it = base->objects.find(id);
                        if(it != base->objects.end()){
                            ref = it->second.type == ObjectType::FUNCTION
                                ? makeMethod(obj, &it->second)
                                : makeRef(it->second);
                            return;
                        }

                        to_check.insert(to_check.end(), base->bases.rbegin(), base->bases.rend());
                    }

                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("cannot find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
                }

                case ObjectType::STRING:{
                    ObjectDict_t::iterator it = lib::cString->objects.find(id);
                    if(it != lib::cString->objects.end()){
                        ref = makeMethod(obj, &it->second);
                        return;
                    }

                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("cannot find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
                }

                case ObjectType::ENUM:{
                    ObjectDict_t::iterator it = obj->e_ptr->values.find(id);
                    if(it != obj->e_ptr->values.end()){
                        ref = makeRef(it->second);
                        return;
                    }

                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("cannot find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
                }

                default:
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("cannoted find '") + intp.rt->nameFromId(id)
                        + "' in " + runtime::errorString(intp, obj));
            }
        }

        _OcFunc(Iterate){
            RootObject tos = intp.exprStack.back();
            _OcValue(tos);

            bool isString = tos->type == ObjectType::STRING;
            if(tos->type != ObjectType::CLASS_INSTANCE && !isString){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    "for-each supported only for objects (no PODs)");
            }

            RootObject func;
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
            RootObject& tos = intp.exprStack.back();
            _OcValue(tos);

            if(tos->type != ObjectType::CLASS_INSTANCE){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    "'iterate()' did not return a class instance.");
            }

            RootObject func;
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

            RootObject& ref = intp.exprStack.back();
            _OcValue(ref);

            if(ref->type != ObjectType::INTEGER){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("expected integer inside super call "
                        "(found ") + runtime::errorString(intp, ref)
                        + " instead)");
            } else if(ref->i < 0){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    "super call's integer must be greater or equal to zero");
            }
            unsigned super = ref->i;

            auto& back = intp.funcStack.back();
            if(back.thisObject->type == ObjectType::NONE){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("'super' is <null>, unable to find '")
                    + intp.rt->nameFromId(id) + "'");
            }

            ClassVec_t& supers = back.thisObject->i_ptr->base->bases;

            if(supers.size() <= super){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("class '")
                    + intp.rt->nameFromId(back.thisObject->i_ptr->base->name)
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
                        // should never happen normally
                        ref = makeRef(it->second);
                    }
                    return;
                }
                to_check.insert(to_check.end(), base->bases.rbegin(), base->bases.rend());
            }

            intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                std::string("cannot find function '")
                + intp.rt->nameFromId(id) + "' in <super> of class '"
                + intp.rt->nameFromId(supers[super]->name) + "'");
        }
    }
}

#endif
