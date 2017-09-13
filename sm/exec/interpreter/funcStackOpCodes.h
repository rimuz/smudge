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

#include "sm/exec/interpreter/defines.h"
#include "sm/compile/defs.h"
#include "sm/runtime/casts.h"
#include "sm/runtime/id.h"

namespace sm{
    namespace exec{
        _OcFunc(Nop){}

        _OcFunc(StartBlock){
            std::lock_guard<std::mutex> lock(intp.stacks_m);
            intp.funcStack.back().codeBlocks.emplace_back(nullptr);
        }

        _OcFunc(EndBlock){
            intp.stacks_m.lock();
            ObjectDict_t* ptr = intp.funcStack.back().codeBlocks.back();
            if(ptr){
                intp.stacks_m.unlock();
                delete ptr; // TODO TODO TODO TODO TODO TODO
                intp.stacks_m.lock();
            }
            intp.funcStack.back().codeBlocks.pop_back();
            intp.stacks_m.unlock();
        }

        _OcFunc(Return){
            intp.stacks_m.lock();
            Object& back = intp.exprStack.back();
            bool wasInlined = intp.funcStack.back().inlined, empty = intp.funcStack.size() == 1;

            if(back.type == ObjectType::WEAK_REFERENCE){
                back = back.refGet();
            } else if(back.type == ObjectType::STRONG_REFERENCE){
                back.type = ObjectType::WEAK_REFERENCE;
            }

            // copying TODO TODO TODO TODO TODO TODO TODO
            ObjectDictVec_t dictVec = intp.funcStack.back().codeBlocks;
            intp.stacks_m.unlock();
            for(ObjectDictVec_t::iterator it = dictVec.begin(); it != dictVec.end(); ++it){
                delete *it;
            }

            intp.stacks_m.lock();
            intp.funcStack.pop_back();

            if(!empty){
                intp.pc = intp.funcStack.back().pc;
            }

            intp.stacks_m.unlock();
            intp.doReturn = wasInlined || empty;
        }

        _OcFunc(ReturnNull){
            intp.stacks_m.lock();
            ObjectDictVec_t dictVec = intp.funcStack.back().codeBlocks;
            bool wasInlined = intp.funcStack.back().inlined, empty = intp.funcStack.size() == 1;
            intp.funcStack.pop_back();
            intp.stacks_m.unlock();

            for(ObjectDictVec_t::iterator it = dictVec.begin(); it != dictVec.end(); ++it){
                delete *it;
            }

            intp.stacks_m.lock();
            intp.exprStack.emplace_back(Object());
            intp.stacks_m.unlock();

            intp.doReturn = wasInlined || empty;
            if(!empty){
                intp.pc = intp.funcStack.back().pc;
            }
        }

        _OcFunc(EndBlocks){
            unsigned param = (static_cast<uint16_t>(inst[1]) << 8) | inst[2];

            intp.stacks_m.lock();
            ObjectDictVec_t& vec = intp.funcStack.back().codeBlocks;
            ObjectDictVec_t::iterator end = vec.end();
            ObjectDictVec_t::iterator last = end - param;
            ObjectDict_t* ptr;

            while(end != last){
                if((ptr = *(--end)))
                    delete ptr;
                vec.pop_back();
            }
        }

        _OcFunc(Try){
            // TODO
        }

        _OcFunc(Catch){
            // TODO
        }

        _OcFunc(Finally){
            // TODO
        }

        _OcFunc(CallFunction){
            unsigned param = (static_cast<uint16_t>(inst[1]) << 8) | inst[2];

            intp.stacks_m.lock();
            ObjectVec_t::iterator end = intp.exprStack.end();
            Object func = *(end - (param+1));
            runtime::invalidate(func);
            Function* func_ptr;

            ObjectVec_t args(end - param, end);
            runtime::invalidate_all(args);
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
            intp.stacks_m.unlock();

            if(runtime::callable(obj, self, func_ptr)){
                intp.makeCall(func_ptr, args, self);
            } else {
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("cannot invoke 'operator()()' in ")
                    + runtime::errorString(intp, obj));
            }
        }

        _OcFunc(PerformBracing){
            unsigned param = (static_cast<uint16_t>(inst[1]) << 8) | inst[2];

            intp.stacks_m.lock();
            ObjectVec_t::iterator end = intp.exprStack.end();
            Object tosX = *(end - (param+1));
            Function* func_ptr;

            ObjectVec_t args(end - param, end);
            runtime::invalidate_all(args);
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
            runtime::invalidate(tosX);
            intp.stacks_m.unlock();

            if(!runtime::find_any(tosX, func, runtime::squareId)){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("cannot invoke 'operator[]()' in ")
                    + runtime::errorString(intp, tosX));
            }

            if(runtime::callable(func, self = tosX, func_ptr)){
                intp.makeCall(func_ptr, args, self);
            } else {
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("cannot invoke 'operator[]()' in ")
                    + runtime::errorString(intp, tosX));
            }
        }

        _OcFunc(Import){
            unsigned box = (static_cast<uint16_t>(inst[1]) << 8) | inst[2];
            unsigned nameId = runtime::idsStart + ((static_cast<uint16_t>(inst[3]) << 8) | inst[4]);

            Object imported;
            imported.type = ObjectType::BOX;
            imported.c_ptr = intp.rt->boxes[box];

            intp.stacks_m.lock();
            ObjectDict_t& dict = intp.funcStack.back().box->objects;
            intp.stacks_m.unlock();
            ObjectDict_t::const_iterator it = dict.find(nameId);

            if(it != dict.end()){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
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
                        intp.rt->sources.printStackTrace(intp, error::ET_BUG,
                            std::string("'<init>' is not a function in box '")
                            + intp.rt->boxNames[imported.c_ptr->boxName] + "' (err #4)");
                    }
                    intp.callFunction(fn, {}, self, true);
                }

                self = nullptr;

                // call 'new' function
                if(runtime::find<ObjectType::BOX>(imported, objFunc, lib::idNew)){
                    if(!runtime::callable(objFunc, self, fn)){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
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
