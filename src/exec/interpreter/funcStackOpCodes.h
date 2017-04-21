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
            // NO Advance
        }

        _OcFunc(ReturnNull){
            ObjectDictVec_t& dictVec = intp.funcStack.back().codeBlocks;
            for(ObjectDictVec_t::iterator it = dictVec.begin(); it != dictVec.end(); ++it){
                delete *it;
            }
            intp.funcStack.pop_back();
            intp.exprStack.emplace_back(Object());
            // NO Advance
        }

        _OcFunc(CallNewFunction){
            // TODO
            ++addr;
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
            const Object& func = *(end - (param+1));
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

            if(runtime::callable(func, func_ptr)){
                intp.makeCall(func_ptr, args);
            } else {
                Object obj = func;
                _OcValue(obj);
                if(obj.type == ObjectType::METHOD){
                    Function* func_ptr = obj.m_ptr->func_ptr->f_ptr;
                    intp.makeCall(func_ptr, args, obj.m_ptr->self);
                } else {
                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("cannot find 'operator()' in ") + runtime::errorString(*intp.rt, obj));
                }
            }
        }

        _OcFunc(PerformBracing){
            // TODO
            addr += 3;
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
                    + "' because it's redefining variable named '" + intp.rt->nameFromId(nameId)
                    + "'");
            }

            dict[nameId] = std::move(imported);
        }
    }
}

#endif
