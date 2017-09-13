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
*      File exec/interpreter/defines.h
*
*/

#ifndef _SM__INTERPRETER__DEFINES_H
#define _SM__INTERPRETER__DEFINES_H

#include <utility>
#include "sm/runtime/Object.h"
#include "sm/exec/Interpreter.h"

#define _OcFunc(Name) inline void Name(sm::exec::Interpreter& intp, std::array<uint8_t, 5> inst) noexcept

#define _OcPopStore(Name) \
    sm::Object Name = std::move(intp.exprStack.back()); \
    runtime::invalidate(Name); \
    intp.exprStack.pop_back();

#define _OcStore(Name) \
    sm::Object& Name = intp.exprStack.back();

#define _OcPopStore2 \
    _OcPopStore(tos); \
    _OcPopStore(tos1);

#define _OcValue(ObjName) \
    while(ObjName.type == ObjectType::WEAK_REFERENCE \
            || ObjName.type == ObjectType::STRONG_REFERENCE) \
        ObjName = ObjName.refGet();

#define _OcSimplifyRef(RefName) \
    if(RefName.type == ObjectType::WEAK_REFERENCE){ \
        RefName = RefName.refGet(); \
    } else if(RefName.type == ObjectType::STRONG_REFERENCE){ \
        RefName.type = ObjectType::WEAK_REFERENCE; \
    }

#define _OcOp(Operator, OperatorFloat, TokenType, Inline, Do) \
    switch(tos1.type){ \
        case ObjectType::INTEGER: \
            if(tos.type == ObjectType::INTEGER){ \
                tos1.i = tos1.i Operator tos.i; \
                intp.stacks_m.unlock(); \
                break; \
            } else if(tos.type == ObjectType::FLOAT){ \
                tos1.type = ObjectType::FLOAT; \
                tos1.f = OperatorFloat(tos1.i, tos.f); \
                intp.stacks_m.unlock(); \
                break; \
            } else { \
                intp.stacks_m.unlock(); \
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR, \
                    std::string("cannot perform 'operator" #Operator \
                    "' between <int> and ") \
                    + runtime::errorString(intp, tos)); \
            } \
        case ObjectType::FLOAT: \
            if(tos.type == ObjectType::INTEGER){ \
                tos1.f = OperatorFloat(tos1.f, tos.i); \
                intp.stacks_m.unlock(); \
                break; \
            } else if(tos.type == ObjectType::FLOAT){ \
                tos1.f = OperatorFloat(tos1.f, tos.f); \
                intp.stacks_m.unlock(); \
                break; \
            } else { \
                intp.stacks_m.unlock(); \
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR, \
                    std::string("cannot perform 'operator" #Operator \
                    "' between <float> and ") \
                    + runtime::errorString(intp, tos)); \
            } \
        case ObjectType::CLASS_INSTANCE: { \
            unsigned id = runtime::operatorId(TokenType); \
            Object op; \
            Function* op_ptr = nullptr; \
            ObjectVec_t args = { tos }; \
            if(!runtime::find<ObjectType::CLASS_INSTANCE>(tos1, op, id)){ \
                intp.stacks_m.unlock(); \
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR, \
                    std::string("cannot find 'operator" #Operator "' in ") \
                    + runtime::errorString(intp, tos1)); \
            } else if(!runtime::callable(op, tos1, op_ptr)){ \
                intp.stacks_m.unlock(); \
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR, \
                    std::string("'operator" #Operator \
                    "' is not a function in ") \
                    + runtime::errorString(intp, tos1)); \
            } \
            Object instance = std::move(tos1); \
            runtime::invalidate(instance); \
            intp.exprStack.pop_back(); \
            intp.stacks_m.unlock(); \
            intp.makeCall(op_ptr, args, instance, Inline); \
            Do; \
            break; \
        } \
        case ObjectType::BOX: { \
            unsigned id = runtime::operatorId(TokenType); \
            Object op; \
            Object self; \
            Function* op_ptr = nullptr; \
            ObjectVec_t args = { tos }; \
            if(!runtime::find<ObjectType::BOX>(tos1, op, id)){ \
                intp.stacks_m.unlock(); \
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR, \
                    std::string("cannot find 'operator" #Operator "' in ") \
                    + runtime::errorString(intp, tos1)); \
            } else if(!runtime::callable(op, self, op_ptr)){ \
                intp.stacks_m.unlock(); \
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR, \
                    std::string("'operator" #Operator \
                    "' is not a function in ") \
                    + runtime::errorString(intp, tos1)); \
            } \
            intp.exprStack.pop_back(); \
            intp.stacks_m.unlock(); \
            intp.makeCall(op_ptr, args, self, Inline); \
            Do; \
            break; \
        } \
        case ObjectType::STRING: { \
            unsigned id = runtime::operatorId(TokenType); \
            Object op; \
            Function* op_ptr = nullptr; \
            ObjectVec_t args = { tos }; \
            if(!runtime::find<ObjectType::STRING>(tos1, op, id)){ \
                intp.stacks_m.unlock(); \
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR, \
                    std::string("cannot find 'operator" #Operator "' in ") \
                    + runtime::errorString(intp, tos1)); \
            } else if(!runtime::callable(op, tos1, op_ptr)){ \
                intp.stacks_m.unlock(); \
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR, \
                    std::string("'operator" #Operator \
                    "' is not a function in ") \
                    + runtime::errorString(intp, tos1)); \
            } \
            Object str = std::move(tos1); \
            intp.exprStack.pop_back(); \
            intp.stacks_m.unlock(); \
            intp.makeCall(op_ptr, args, str, Inline); \
            Do; \
            break; \
        } \
        default: \
            intp.stacks_m.unlock(); \
            intp.rt->sources.printStackTrace(intp, error::ET_ERROR, \
                std::string("cannot find 'operator" #Operator "' in ") \
                + runtime::errorString(intp, tos1)); \
    }
#endif
