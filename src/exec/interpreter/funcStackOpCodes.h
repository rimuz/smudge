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
 *      File exec/interpreter/funcStackOpCodes.h
 *
*/

#ifndef _SM__EXEC__INTERPPRETER__FUNCSTACKOPCODES_H
#define _SM__EXEC__INTERPPRETER__FUNCSTACKOPCODES_H

#include "exec/interpreter/defines.h"
#include "compile/defs.h"
#include "runtime/casts.h"
#include "runtime/id.h"

namespace sm{
    namespace exec{
        _OcFunc(Nop){
            ++addr;
        }

        _OcFunc(StartBlock){
            intp.funcStack.back().codeBlocks.emplace_back(nullptr);
            ++addr;
        }

        _OcFunc(EndBlock){
            ObjectDict_t* ptr = intp.funcStack.back().codeBlocks.back();
            if(ptr)
                delete ptr;
            intp.funcStack.back().codeBlocks.pop_back();
            ++addr;
        }

        _OcFunc(Return){
            Object& back = intp.exprStack.back();
            if(back.type == ObjectType::WEAK_REFERENCE){
                back = back.refGet();
            } else if(back.type == ObjectType::STRONG_REFERENCE){
                back.type = ObjectType::WEAK_REFERENCE;
            }

            ObjectDictVec_t& dictVec = intp.funcStack.back().codeBlocks;
            for(ObjectDictVec_t::iterator it = dictVec.begin(); it != dictVec.end(); ++it){
                delete *it;
            }
            intp.funcStack.pop_back();
        }

        _OcFunc(ReturnNull){
            ObjectDictVec_t& dictVec = intp.funcStack.back().codeBlocks;
            for(ObjectDictVec_t::iterator it = dictVec.begin(); it != dictVec.end(); ++it){
                delete *it;
            }
            intp.funcStack.pop_back();
            intp.exprStack.emplace_back(Object());
        }

        _OcFunc(EndBlocks){
            unsigned param = (*++addr << 8) | *++addr;
            ObjectDictVec_t& vec = intp.funcStack.back().codeBlocks;
            ObjectDictVec_t::iterator end = vec.end();
            ObjectDictVec_t::iterator last = end - param;
            ObjectDict_t* ptr;

            while(end != last){
                if((ptr = *(--end)))
                    delete ptr;
                vec.pop_back();
            }
            ++addr;
        }

        _OcFunc(Try){
            // TODO
            addr += 3;
        }

        _OcFunc(Catch){
            // TODO
            addr += 3;
        }

        _OcFunc(Finally){
            // TODO
            addr += 3;
        }

        _OcFunc(CallFunction){
            unsigned param = (*++addr << 8) | *++addr;
            ++addr;

            ObjectVec_t::iterator end = intp.exprStack.end();
            Object func = *(end - (param+1));
            Function* func_ptr;

            ObjectVec_t args(end - param, end);
            intp.exprStack.erase(end - (param+1), end);

            for(Object& obj : args){
                if(obj.type == ObjectType::WEAK_REFERENCE){
                    obj = obj.refGet();
                } else if(obj.type == ObjectType::STRONG_REFERENCE){
                    obj.type = ObjectType::WEAK_REFERENCE;
                }
            }

            Object self;
            Object obj = func;
            _OcValue(obj);

            if(obj.type == ObjectType::CLASS){
                std::vector <std::tuple<Object, Function*>> inits;
                ObjectDict_t::iterator it;
                Class* base = obj.c_ptr;

                self = makeFastInstance(intp.rt->gc, obj.c_ptr, false);
                do {
                    it = base->objects.find(runtime::initId);
                    if(it != base->objects.end()){
                        inits.emplace_back(self, nullptr);
                        if(!runtime::callable(it->second, std::get<0>(inits.back()),
                                std::get<1>(inits.back()))){
                            intp.rt->sources.printStackTrace(intp, error::ERROR,
                                std::string("'<init>' is not a function in ")
                                + runtime::errorString(intp, self));
                        }
                    }
                } while(!base->bases.empty() && (base = base->bases.front()));

                for(auto rit = inits.rbegin(); rit != inits.rend(); ++rit){
                    // we don't care about the <init>() return value.
                    intp.callFunction(std::get<1>(*rit), {}, std::get<0>(*rit), true);
                }

                it = base->objects.find(lib::idNew);
                if(it != base->objects.end()){
                    obj = it->second;
                    if(!runtime::callable(obj, self, func_ptr))
                        intp.rt->sources.printStackTrace(intp, error::ERROR,
                            std::string("'new' is not a function in ")
                            + runtime::errorString(intp, self));
                    // such as <init>(), we don't care about the 'new()' return value.
                    intp.callFunction(func_ptr, {}, self, true);
                }

                intp.exprStack.emplace_back(std::move(self));
                return;
            }

            if(runtime::callable(obj, self, func_ptr)){
                intp.makeCall(func_ptr, args, self);
            } else {
                intp.rt->sources.printStackTrace(intp, error::ERROR,
                    std::string("cannot invoke 'operator()()' in ")
                    + runtime::errorString(intp, obj));
            }
        }

        _OcFunc(PerformBracing){
            unsigned param = (*++addr << 8) | *++addr;
            ++addr;

            ObjectVec_t::iterator end = intp.exprStack.end();
            Object tosX = *(end - (param+1));
            Function* func_ptr;

            ObjectVec_t args(end - param, end);
            intp.exprStack.erase(end - (param+1), end);

            for(Object& obj : args){
                if(obj.type == ObjectType::WEAK_REFERENCE){
                    obj = obj.refGet();
                } else if(obj.type == ObjectType::STRONG_REFERENCE){
                    obj.type = ObjectType::WEAK_REFERENCE;
                }
            }

            Object self;
            Object func;
            _OcValue(tosX);

            switch(tosX.type){
                case ObjectType::BOX:
                    if(runtime::find<ObjectType::BOX>(tosX, func,
                            runtime::operatorId(parse::TT_SQUARE_OPEN)))
                        break;

                case ObjectType::CLASS_INSTANCE:
                    if(runtime::find<ObjectType::CLASS_INSTANCE>(tosX, func,
                            runtime::operatorId(parse::TT_SQUARE_OPEN))){
                        self = tosX;
                        break;
                    }

                default:
                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("cannot invoke 'operator[]()' in ")
                        + runtime::errorString(intp, tosX));
            }

            if(runtime::callable(func, self, func_ptr)){
                intp.makeCall(func_ptr, args, self);
            } else {
                intp.rt->sources.printStackTrace(intp, error::ERROR,
                    std::string("cannot invoke 'operator[]()' in ")
                    + runtime::errorString(intp, tosX));
            }
        }

        _OcFunc(Import){
            unsigned box = (*++addr << 8) | *++addr;
            unsigned nameId = runtime::idsStart + ((*++addr << 8) | *++addr);
            ++addr;

            Object imported;
            imported.type = ObjectType::BOX;
            imported.c_ptr = intp.rt->boxes[box];

            ObjectDict_t& dict = intp.funcStack.back().box->objects;
            ObjectDict_t::const_iterator it = dict.find(nameId);

            if(it != dict.end()){
                intp.rt->sources.printStackTrace(intp, error::ERROR,
                    std::string("cannot import box '") + intp.rt->boxNames[box]
                    + "' because it's redefining variable, function or class named '" + intp.rt->nameFromId(nameId)
                    + "'");
            }

            dict.insert({nameId, imported});

            if(!imported.c_ptr->isInitialized){
                imported.c_ptr->isInitialized = true;

                Object objFunc;
                Object self;
                Function* fn;

                // call '<init>' function (inlined)
                if(runtime::find<ObjectType::BOX>(imported, objFunc, runtime::initId)){
                    if(!runtime::callable(objFunc, self, fn)){
                        intp.rt->sources.printStackTrace(intp, error::BUG,
                            std::string("'<init>' is not a function in box '")
                            + intp.rt->boxNames[imported.c_ptr->boxName] + "' (err #4)");
                    }
                    intp.callFunction(fn, {}, self, true);
                }

                self = nullptr;

                // call 'new' function
                if(runtime::find<ObjectType::BOX>(imported, objFunc, lib::idNew)){
                    if(!runtime::callable(objFunc, self, fn)){
                        intp.rt->sources.printStackTrace(intp, error::ERROR,
                            std::string("'new' is not a function in box '")
                            + intp.rt->boxNames[imported.c_ptr->boxName] + "'");
                    }
                    intp.makeCall(fn, {}, self, false);
                }
            }
        }
    }
}

#endif
