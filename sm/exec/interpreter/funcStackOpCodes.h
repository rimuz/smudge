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
            intp.funcStack.back().codeBlocks.emplace_back(nullptr);
        }

        _OcFunc(EndBlock){
            RootObjectDict_t* ptr = intp.funcStack.back().codeBlocks.back();
            intp.funcStack.back().codeBlocks.pop_back();
            if(ptr)
                delete ptr;
        }

        _OcFunc(Return){
            RootObject& back = intp.exprStack.back();
            bool wasInlined = intp.funcStack.back().inlined, empty = intp.funcStack.size() == 1;

            if(back->type == ObjectType::WEAK_REFERENCE){
                back = back->refGet();
            } else if(back->type == ObjectType::STRONG_REFERENCE){
                back->type = ObjectType::WEAK_REFERENCE;
            }

            RootObjectDictVec_t dictVec = std::move(intp.funcStack.back().codeBlocks);
            for(RootObjectDictVec_t::iterator it = dictVec.begin(); it != dictVec.end(); ++it)
                delete *it;

            intp.funcStack.pop_back();
            intp.doReturn = wasInlined || empty;

            if(!empty)
                intp.pc = intp.funcStack.back().pc;
        }

        _OcFunc(ReturnNull){
            bool wasInlined = intp.funcStack.back().inlined, empty = intp.funcStack.size() == 1;

            RootObjectDictVec_t dictVec = std::move(intp.funcStack.back().codeBlocks);
            for(RootObjectDictVec_t::iterator it = dictVec.begin(); it != dictVec.end(); ++it){
                delete *it;
            }

            intp.funcStack.pop_back();
            intp.exprStack.emplace_back(nullptr);
            intp.doReturn = wasInlined || empty;

            if(!empty)
                intp.pc = intp.funcStack.back().pc;
        }

        _OcFunc(EndBlocks){
            unsigned param = (static_cast<uint16_t>(inst[1]) << 8) | inst[2];

            RootObjectDictVec_t& vec = intp.funcStack.back().codeBlocks;
            RootObjectDictVec_t::iterator end = vec.end();
            RootObjectDictVec_t::iterator last = end - param;
            RootObjectDict_t* ptr;

            while(end != last){
                ptr = *(--end);
                vec.pop_back();
                if(ptr)
                    delete ptr;
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

            RootObjectVec_t::iterator end = intp.exprStack.end();
            RootObject func = *(end -param -1);
            Function* func_ptr;

            RootObjectVec_t args(
                    std::make_move_iterator(end - param),
                    std::make_move_iterator(end)
            );
            intp.exprStack.erase(end -param -1, end);

            for(Object& obj : args){
                if(obj.type == ObjectType::WEAK_REFERENCE){
                    obj = obj.refGet();
                } else if(obj.type == ObjectType::STRONG_REFERENCE){
                    obj.type = ObjectType::WEAK_REFERENCE;
                }
            }

            RootObject self;
            RootObject obj = func;
            _OcValue(obj);

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

            RootObjectVec_t::iterator end = intp.exprStack.end();
            RootObject tosX = *(end - (param+1));
            Function* func_ptr;

            RootObjectVec_t args(
                std::make_move_iterator(end - param),
                std::make_move_iterator(end)
            );
            intp.exprStack.erase(end - (param+1), end);

            for(Object& obj : args){
                if(obj.type == ObjectType::WEAK_REFERENCE){
                    obj = obj.refGet();
                } else if(obj.type == ObjectType::STRONG_REFERENCE){
                    obj.type = ObjectType::WEAK_REFERENCE;
                }
            }

            RootObject self;
            RootObject func;
            _OcValue(tosX);

            if(!runtime::find_any(tosX, func, runtime::squareId)){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("cannot invoke 'operator[]()' in ")
                    + runtime::errorString(intp, tosX));
            }

            if(runtime::callable(func, self = tosX, func_ptr)){
                intp.makeCall(func_ptr, args, self);
            } else {
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("'operator[]()' is not a function in ")
                    + runtime::errorString(intp, tosX));
            }
        }

        _OcFunc(Import){
            unsigned box = (static_cast<uint16_t>(inst[1]) << 8) | inst[2];
            unsigned nameId = runtime::idsStart + ((static_cast<uint16_t>(inst[3]) << 8) | inst[4]);
            Object imported = makeBox(intp.rt->boxes[box]);

            RootObjectDict_t& dict = intp.funcStack.back().box->objects;
            RootObjectDict_t::const_iterator it = dict.find(nameId);

            if(it != dict.end()){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("cannot import box '") + intp.rt->boxNames[box]
                    + "' because it's redefining variable, function or class named '" + intp.rt->nameFromId(nameId)
                    + "'");
            }
            dict.insert({nameId, RootObject(imported)});

            if(!imported.b_ptr->isInitialized){
                imported.b_ptr->isInitialized = true;

                RootObject objFunc;
                RootObject self;
                Function* fn;

                // call '<init>' function (inlined)
                if(runtime::find<ObjectType::BOX>(imported, objFunc, runtime::initId)){
                    if(!runtime::callable(objFunc, self, fn)){
                        intp.rt->sources.printStackTrace(intp, error::ET_BUG,
                            std::string("'<init>' is not a function in box '")
                            + intp.rt->boxNames[imported.b_ptr->name] + "' (err #4)");
                    }
                    intp.callFunction(fn, {}, self, true);
                }

                self = nullptr;

                // call 'new' function
                if(runtime::find<ObjectType::BOX>(imported, objFunc, lib::idNew)){
                    if(!runtime::callable(objFunc, self, fn)){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                            std::string("'new' is not a function in box '")
                            + intp.rt->boxNames[imported.b_ptr->name] + "'");
                    }
                    intp.makeCall(fn, {}, self, false);
                }
            }
        }
    }
}

#endif
