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
*      File exec/interpreter/singleOperandMathOpCodes.h
*
*/

#ifndef _SM__EXEC__INTERPRETER__SINGLEOPERANDMATHOPCODES_H
#define _SM__EXEC__INTERPRETER__SINGLEOPERANDMATHOPCODES_H

#include "sm/exec/interpreter/defines.h"
#include "sm/runtime/casts.h"
#include "sm/runtime/id.h"

namespace sm{
    namespace exec{
        _OcFunc(Compl){
            ++addr;
            _OcPopStore(tos);
            _OcValue(tos);

            switch(tos.type){
                case ObjectType::INTEGER:
                    tos.i = ~tos.i;
                    intp.exprStack.emplace_back(std::move(tos));
                    return;

                case ObjectType::CLASS_INSTANCE:{
                    unsigned id = runtime::operatorId(parse::TT_COMPL);
                    Object op;
                    Function* op_ptr = nullptr;

                    if(!runtime::find<ObjectType::CLASS_INSTANCE>(tos, op, id)
                            || !runtime::callable(op, tos, op_ptr)){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                            std::string("'operator~' not applicable for ")
                            + runtime::errorString(intp, tos));
                    }

                    intp.makeCall(op_ptr, {}, tos);
                    return;
                }

                case ObjectType::BOX: {
                    unsigned id = runtime::operatorId(parse::TT_COMPL);
                    Object op;
                    Object self;
                    Function* op_ptr = nullptr;

                    if(!runtime::find<ObjectType::BOX>(tos, op, id)
                            || !runtime::callable(op, self, op_ptr)){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                            std::string("'operator~' not applicable for ")
                            + runtime::errorString(intp, tos));
                    }

                    intp.makeCall(op_ptr, {}, self);
                    return;
                }

                default:
                    intp.rt->sources.printStackTrace(*intp.rt, error::ET_ERROR,
                        std::string("'operator~' not applicable for ") +
                        runtime::errorString(intp, tos));
            }
        }

        _OcFunc(Not){
            ++addr;
            _OcStore(tos);
            _OcValue(tos);
            tos = makeBool(!runtime::implicitToBool(tos));
        }

        _OcFunc(UnaryPlus){
            ++addr;
            _OcPopStore(tos);
            _OcValue(tos);

            switch(tos.type){
                case ObjectType::INTEGER:
                case ObjectType::FLOAT:
                    intp.exprStack.emplace_back(std::move(tos));
                    return;

                case ObjectType::CLASS_INSTANCE:{
                    unsigned id = runtime::operatorId(parse::TT_PRE_PLUS);
                    Object op;
                    Function* op_ptr = nullptr;
                    ObjectVec_t args { Object(), makeTrue() };

                    if(!runtime::find<ObjectType::CLASS_INSTANCE>(tos, op, id)
                            || !runtime::callable(op, tos, op_ptr)){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                            std::string("'operator unary+' not applicable for ")
                            + runtime::errorString(intp, tos));
                    }

                    intp.makeCall(op_ptr, args, tos);
                    return;
                }

                case ObjectType::BOX: {
                    unsigned id = runtime::operatorId(parse::TT_PRE_PLUS);
                    Object op;
                    Object self;
                    Function* op_ptr = nullptr;
                    ObjectVec_t args { Object(), makeTrue() };

                    if(!runtime::find<ObjectType::BOX>(tos, op, id)
                            || !runtime::callable(op, self, op_ptr)){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                            std::string("'operator unary+' not applicable for ")
                            + runtime::errorString(intp, tos));
                    }

                    intp.makeCall(op_ptr, args, self);
                    return;
                }

                default:
                    intp.rt->sources.printStackTrace(*intp.rt, error::ET_ERROR,
                        std::string("'operator unary+' not applicable for ") +
                        runtime::errorString(intp, tos));
            }
        }

        _OcFunc(UnaryMinus){
            ++addr;
            _OcPopStore(tos);
            _OcValue(tos);

            switch(tos.type){
                case ObjectType::INTEGER:
                    tos.i = -tos.i;
                    intp.exprStack.emplace_back(std::move(tos));
                    return;

                case ObjectType::FLOAT:
                    tos.f = -tos.f;
                    intp.exprStack.emplace_back(std::move(tos));
                    return;

                case ObjectType::CLASS_INSTANCE:{
                    unsigned id = runtime::operatorId(parse::TT_PRE_MINUS);
                    Object op;
                    Function* op_ptr = nullptr;
                    ObjectVec_t args { Object(), makeTrue() };

                    if(!runtime::find<ObjectType::CLASS_INSTANCE>(tos, op, id)
                            || !runtime::callable(op, tos, op_ptr)){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                            std::string("'operator unary-' not applicable for ")
                            + runtime::errorString(intp, tos));
                    }

                    intp.makeCall(op_ptr, args, tos);
                    return;
                }

                case ObjectType::BOX: {
                    unsigned id = runtime::operatorId(parse::TT_PRE_MINUS);
                    Object op;
                    Object self;
                    Function* op_ptr = nullptr;
                    ObjectVec_t args { Object(), makeTrue() };

                    if(!runtime::find<ObjectType::BOX>(tos, op, id)
                            || !runtime::callable(op, self, op_ptr)){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                            std::string("'operator unary-' not applicable for ")
                            + runtime::errorString(intp, tos));
                    }

                    intp.makeCall(op_ptr, args, self);
                    return;
                }

                default:
                    intp.rt->sources.printStackTrace(*intp.rt, error::ET_ERROR,
                        std::string("'operator unary-' not applicable for ") +
                        runtime::errorString(intp, tos));
            }
        }

        _OcFunc(Inc){
            Object& back = intp.exprStack.back();
            if(back.type != ObjectType::WEAK_REFERENCE && back.type != ObjectType::STRONG_REFERENCE)
                intp.rt->sources.printStackTrace(*intp.rt, error::ET_ERROR,
                    "cannot apply 'operator++' to temporary object (non-reference)");

            ByteCode_t::const_iterator dummy;
            Dup(intp, addr);
            PushInt1(intp, dummy);
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(+, std::plus<float_t>(), parse::TT_PLUS, true,
                runtime::validate(intp.exprStack, intp.start()));
            Assign(intp, dummy);
        }

        _OcFunc(Dec){
            Object& back = intp.exprStack.back();
            if(back.type != ObjectType::WEAK_REFERENCE && back.type != ObjectType::STRONG_REFERENCE)
                intp.rt->sources.printStackTrace(*intp.rt, error::ET_ERROR,
                    "cannot apply 'operator--' to temporary object (non-reference)");

            ByteCode_t::const_iterator dummy;
            Dup(intp, addr);
            PushInt1(intp, dummy);
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(-, std::minus<float_t>(), parse::TT_MINUS, true,
                runtime::validate(intp.exprStack, intp.start()));
            Assign(intp, dummy);
        }

        _OcFunc(PostInc){
            Object& back = intp.exprStack.back();
            if(back.type != ObjectType::WEAK_REFERENCE && back.type != ObjectType::STRONG_REFERENCE)
                intp.rt->sources.printStackTrace(*intp.rt, error::ET_ERROR,
                    "cannot apply 'operator++' to temporary object (non-reference)");

            Object copy = intp.exprStack.back();
            ByteCode_t::const_iterator dummy;

            _OcValue(copy);
            Dup(intp, addr);
            PushInt1(intp, dummy);
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(+, std::plus<float_t>(), parse::TT_PLUS, true,
                runtime::validate(intp.exprStack, intp.start()));
            Assign(intp, dummy);
            intp.exprStack.back() = copy;
        }

        _OcFunc(PostDec){
            Object& back = intp.exprStack.back();
            if(back.type != ObjectType::WEAK_REFERENCE && back.type != ObjectType::STRONG_REFERENCE)
                intp.rt->sources.printStackTrace(*intp.rt, error::ET_ERROR,
                    "cannot apply 'operator--' to temporary object (non-reference)");

            Object copy = intp.exprStack.back();
            ByteCode_t::const_iterator dummy;

            _OcValue(copy);
            Dup(intp, addr);
            PushInt1(intp, dummy);
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(-, std::minus<float_t>(), parse::TT_MINUS, true,
                runtime::validate(intp.exprStack, intp.start()));
            Assign(intp, dummy);
            intp.exprStack.back() = copy;
        }
    }
}

#endif
