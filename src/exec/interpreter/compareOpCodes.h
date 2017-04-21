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
*      File exec/interpreter/compareOpCodes.h
*
*/

#ifndef _SM__EXEC__INTERPRETER__COMPAREOPCODES_H
#define _SM__EXEC__INTERPRETER__COMPAREOPCODES_H

#include "exec/interpreter/defines.h"
#include "runtime/casts.h"
#include "runtime/id.h"

#define _OcCmpOp(Operator, TokenType) \
    switch(tos1.type){ \
        case ObjectType::INTEGER: \
            if(tos.type == ObjectType::INTEGER){ \
                tos1.i = tos1.i Operator tos.i; \
                intp.exprStack.emplace_back(std::move(tos1)); \
                return; \
            } else if(tos.type == ObjectType::FLOAT){ \
                tos1.i = tos1.i Operator tos.f; \
                intp.exprStack.emplace_back(std::move(tos1)); \
                return; \
            } else { \
                intp.rt->sources.printStackTrace(intp, error::ERROR, \
                    std::string("cannot perform 'operator" #Operator \
                    "' between <int> and ") \
                    + runtime::errorString(*intp.rt, tos)); \
            } \
        case ObjectType::FLOAT: \
            if(tos.type == ObjectType::INTEGER){ \
                tos.i = tos1.f Operator tos.i; \
                intp.exprStack.emplace_back(std::move(tos)); \
                return; \
            } else if(tos.type == ObjectType::FLOAT){ \
                tos.type = ObjectType::INTEGER; \
                tos.i = tos1.f Operator tos.f; \
                intp.exprStack.emplace_back(std::move(tos)); \
                return; \
            } else { \
                intp.rt->sources.printStackTrace(intp, error::ERROR, \
                    std::string("cannot perform 'operator" #Operator \
                    "' between <float> and ") \
                    + runtime::errorString(*intp.rt, tos)); \
            } \
        case ObjectType::CLASS_INSTANCE: { \
            unsigned id = runtime::operatorId(TokenType); \
            Object op; \
            Function* op_ptr = nullptr; \
            ObjectVec_t args = { tos }; \
            if(!runtime::find<ObjectType::CLASS_INSTANCE>(tos1, op, id)){ \
                intp.rt->sources.printStackTrace(intp, error::ERROR, \
                    std::string("cannot find 'operator" #Operator "' in ") \
                    + runtime::errorString(*intp.rt, tos1)); \
            } else if(!runtime::callable(op, op_ptr)){ \
                intp.rt->sources.printStackTrace(intp, error::ERROR, \
                    std::string("'operator" #Operator \
                    "' is not a function in ") \
                    + runtime::errorString(*intp.rt, tos1)); \
            } \
            intp.makeCall(op_ptr, args, tos1); \
            return; \
        } \
        case ObjectType::STRING:{ \
            unsigned id = runtime::operatorId(TokenType); \
            Object op; \
            Function* op_ptr = nullptr; \
            ObjectVec_t args = { tos }; \
            if(!runtime::find<ObjectType::STRING>(tos1, op, id)){ \
                intp.rt->sources.printStackTrace(intp, error::ERROR, \
                    std::string("cannot find 'operator" #Operator "' in ") \
                    + runtime::errorString(*intp.rt, tos1)); \
            } else if(!runtime::callable(op, op_ptr)){ \
                intp.rt->sources.printStackTrace(intp, error::ERROR, \
                    std::string("'operator" #Operator \
                    "' is not a function in ") \
                    + runtime::errorString(*intp.rt, tos1)); \
            } \
            intp.makeCall(op_ptr, args, tos1); \
            return; \
        } \
        case ObjectType::BOX: { \
            unsigned id = runtime::operatorId(TokenType); \
            Object op; \
            Function* op_ptr = nullptr; \
            ObjectVec_t args = { tos }; \
            if(!runtime::find<ObjectType::BOX>(tos1, op, id)){ \
                intp.rt->sources.printStackTrace(intp, error::ERROR, \
                    std::string("cannot find 'operator" #Operator "' in ") \
                    + runtime::errorString(*intp.rt, tos1)); \
            } else if(!runtime::callable(op, op_ptr)){ \
                intp.rt->sources.printStackTrace(intp, error::ERROR, \
                    std::string("'operator" #Operator \
                    "' is not a function in ") \
                    + runtime::errorString(*intp.rt, tos1)); \
            } \
            intp.makeCall(op_ptr, args); \
            return; \
        } \
        default: \
            intp.rt->sources.printStackTrace(intp, error::ERROR, \
                std::string("cannot find 'operator" #Operator "' in ") \
                + runtime::errorString(*intp.rt, tos1)); \
    }

namespace sm{
    namespace exec{
        _OcFunc(IsNull){
            _OcPopStore(tos);
            _OcValue(tos);

            Object obj;
            obj.type = ObjectType::INTEGER;
            obj.i = tos.type == ObjectType::NONE;

            intp.exprStack.emplace_back(std::move(obj));
            ++addr;
        }

        _OcFunc(Equal){
            ++addr;
            _OcPopStore2;
            _OcValue(tos1);
            _OcValue(tos);
            _OcCmpOp(==, parse::TT_EQUAL);
        }

        _OcFunc(NotEqual){
            ++addr;
            _OcPopStore2;
            _OcValue(tos1);
            _OcValue(tos);
            _OcCmpOp(!=, parse::TT_NOT_EQUAL);
        }

        _OcFunc(Greater){
            ++addr;
            _OcPopStore2;
            _OcValue(tos1);
            _OcValue(tos);
            _OcCmpOp(>, parse::TT_GREATER);
        }

        _OcFunc(GreaterOrEqual){
            ++addr;
            _OcPopStore2;
            _OcValue(tos1);
            _OcValue(tos);
            _OcCmpOp(>=, parse::TT_GREATER_OR_EQUAL);
        }

        _OcFunc(Less){
            ++addr;
            _OcPopStore2;
            _OcValue(tos1);
            _OcValue(tos);
            _OcCmpOp(<, parse::TT_LESS);
        }

        _OcFunc(LessOrEqual){
            ++addr;
            _OcPopStore2;
            _OcValue(tos1);
            _OcValue(tos);
            _OcCmpOp(<=, parse::TT_LESS_OR_EQUAL);
        }
    }
}

#endif
