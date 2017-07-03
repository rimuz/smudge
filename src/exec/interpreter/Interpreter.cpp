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
 *      File exec/interpreter/Interpreter.cpp
 *
*/

#include <iostream>

#include "exec/interpreter/arithmeticOpCodes.h"
#include "exec/interpreter/assignOpCodes.h"
#include "exec/interpreter/compareOpCodes.h"
#include "exec/interpreter/compareOpCodes.h"
#include "exec/interpreter/defineOpCodes.h"
#include "exec/interpreter/exprStackOpCodes.h"
#include "exec/interpreter/funcStackOpCodes.h"
#include "exec/interpreter/jumpOpCodes.h"
#include "exec/interpreter/logicAndShiftsOpCodes.h"
#include "exec/interpreter/makeOpCodes.h"
#include "exec/interpreter/singleOperandMathOpCodes.h"
#include "exec/Interpreter.h"
#include "runtime/id.h"

#define _OcCase(OpCode, FuncName) \
    case compile::OpCode:{ \
        FuncName(*this, addr); \
        break; \
    }

namespace sm{
    namespace exec{
        Object Interpreter::_start(){
            while(!funcStack.empty()){
                if(funcStack.back().inlined){
                    funcStack.pop_back();
                    break;
                }

                ByteCode_t::const_iterator& addr = funcStack.back().addr;
                switch(*addr){
                    _OcCase(NOP, Nop);
                    _OcCase(POP, Pop);
                    _OcCase(IS_NULL, IsNull);
                    _OcCase(ADD, Add);
                    _OcCase(SUB, Sub);
                    _OcCase(MUL, Mul);
                    _OcCase(DIV, Div);
                    _OcCase(MOD, Mod);
                    _OcCase(OR, Or);
                    _OcCase(AND, And);
                    _OcCase(XOR, Xor);
                    _OcCase(LEFT_SHIFT, LeftShift);
                    _OcCase(RIGHT_SHIFT, RightShift);
                    _OcCase(EQUAL, Equal);
                    _OcCase(NOT_EQUAL, NotEqual);
                    _OcCase(GREATER, Greater);
                    _OcCase(GREATER_OR_EQUAL, GreaterOrEqual);
                    _OcCase(LESS, Less);
                    _OcCase(LESS_OR_EQUAL, LessOrEqual);
                    _OcCase(ASSIGN, Assign);
                    _OcCase(ASSIGN_ADD, AssignAdd);
                    _OcCase(ASSIGN_SUB, AssignSub);
                    _OcCase(ASSIGN_MUL, AssignMul);
                    _OcCase(ASSIGN_DIV, AssignDiv);
                    _OcCase(ASSIGN_MOD, AssignMod);
                    _OcCase(ASSIGN_OR, AssignOr);
                    _OcCase(ASSIGN_AND, AssignAnd);
                    _OcCase(ASSIGN_XOR, AssignXor);
                    _OcCase(ASSIGN_LEFT_SHIFT, AssignLeftShift);
                    _OcCase(ASSIGN_RIGHT_SHIFT, AssignRightShift);
                    _OcCase(MAKE_VOID_LIST, MakeVoidList);
                    _OcCase(MAKE_VOID_TUPLE, MakeVoidTuple);
                    _OcCase(MAKE_REF, MakeRef);
                    _OcCase(COMPL, Compl);
                    _OcCase(NOT, Not);
                    _OcCase(UNARY_PLUS, UnaryPlus);
                    _OcCase(UNARY_MINUS, UnaryMinus);
                    _OcCase(INC, Inc);
                    _OcCase(DEC, Dec);
                    _OcCase(POST_INC, PostInc);
                    _OcCase(POST_DEC, PostDec);
                    _OcCase(START_BLOCK, StartBlock);
                    _OcCase(END_BLOCK, EndBlock);
                    _OcCase(THROW_EXCEPTION, ThrowException);
                    _OcCase(RETURN, Return);
                    _OcCase(RETURN_NULL, ReturnNull);
                    _OcCase(PUSH_INT_0, PushInt0);
                    _OcCase(PUSH_INT_1, PushInt1);
                    _OcCase(PUSH_NULL, PushNull);
                    _OcCase(PUSH_THIS, PushThis);
                    _OcCase(PUSH_BOX, PushBox);
                    _OcCase(PUSH_CLASS, PushClass);
                    _OcCase(FIND_NEW, FindNew);
                    _OcCase(FIND_DELETE, FindDelete);
                    _OcCase(FIND_NEW_SUPER, FindNewSuper);
                    _OcCase(FIND_DELETE_SUPER, FindDeleteSuper);
                    _OcCase(ITERATE, Iterate);
                    _OcCase(IT_NEXT, ItNext);
                    _OcCase(MAKE_SUPER, MakeSuper);
                    _OcCase(DUP, Dup);
                    _OcCase(DUP1, Dup1);
                    _OcCase(END_BLOCKS, EndBlocks);
                    _OcCase(PUSH_INTEGER, PushInteger);
                    _OcCase(PUSH_FLOAT, PushFloat);
                    _OcCase(PUSH_STRING, PushString);
                    _OcCase(PUSH_INT_VALUE, PushIntValue);
                    _OcCase(PUSH_REF, PushRef);
                    _OcCase(JUMP_F, JumpF);
                    _OcCase(JUMP_B, JumpB);
                    _OcCase(JUMP_IF_F, JumpIfF);
                    _OcCase(JUMP_IF_B, JumpIfB);
                    _OcCase(JUMP_IF_NOT_F, JumpIfNotF);
                    _OcCase(JUMP_IF_NOT_B, JumpIfNotB);
                    _OcCase(LOGIC_AND, LogicAnd);
                    _OcCase(LOGIC_OR, LogicOr);
                    _OcCase(ELVIS, Elvis);
                    _OcCase(TRY, Try);
                    _OcCase(CATCH, Catch);
                    _OcCase(FINALLY, Finally);
                    _OcCase(CALL_FUNCTION, CallFunction);
                    _OcCase(PERFORM_BRACING, PerformBracing);
                    _OcCase(DEFINE_VAR, DefineVar);
                    _OcCase(DEFINE_GLOBAL_VAR, DefineGlobalVar);
                    _OcCase(DEFINE_NULL_VAR, DefineNullVar);
                    _OcCase(DEFINE_GLOBAL_NULL_VAR, DefineGlobalNullVar);
                    _OcCase(ASSIGN_NULL_POP, AssignNullPop);
                    _OcCase(FIND, Find);
                    _OcCase(FIND_SUPER, FindSuper);
                    _OcCase(MAKE_LIST, MakeList);
                    _OcCase(MAKE_TUPLE, MakeTuple);
                    _OcCase(FOREACH_CHECK, ForeachCheck);
                    _OcCase(SWITCH_CASE, SwitchCase);
                    _OcCase(IMPORT, Import);

                    default: {
                        rt->sources.printStackTrace(*this, error::FATAL_ERROR,
                            std::string("unsupported instruction's opcode (")
                            + std::to_string(static_cast<unsigned>(*addr)) + ").");
                    }
                }
            }

            Object obj(std::move(exprStack.back()));
            exprStack.pop_back();
            return obj;
        }

        Object Interpreter::callFunction(Function* fn, const ObjectVec_t& args,
                const Object& self, bool inlined){
            if(!funcStack.empty() && !inlined){
                std::cerr << "error: alredy calling function." << std::endl;
                runtime::Runtime_t::exit(1);
            }
            makeCall(fn, args, self, inlined);
            return _start();
        }

        void Interpreter::makeCall(Function* fn, const ObjectVec_t& args,
                const Object& self, bool inlined){
            if(inlined){
                funcStack.emplace_back();
                funcStack.back().inlined = true;
            }

            if(fn->flags & FF_NATIVE){
                Object ret = (*fn->native_ptr)(*this, fn, self, args);
                if(ret.type == ObjectType::WEAK_REFERENCE){
                    ret = ret.refGet();
                } else if(ret.type == ObjectType::STRONG_REFERENCE){
                    ret.type = ObjectType::WEAK_REFERENCE;
                }
                exprStack.emplace_back(std::move(ret));
            } else {
                funcStack.emplace_back();
                CallInfo_t& backInfo = funcStack.back();
                backInfo.codeBlocks.reserve(5);
                size_t address;

                if(fn->arguments.empty()){
                    address = fn->address;
                } else {
                    /*
                     * 'fn->arguments' are the arguments expected by the
                     * function, 'args' are the arguments actually given by
                     * the caller.
                    */

                    ObjectDict_t* dict = new ObjectDict_t;
                    backInfo.codeBlocks.push_back(dict);

                    bool is_vararg = fn->flags & FF_VARARGS;
                    size_t n_args = args.size();
                    size_t n_expected = is_vararg ? fn->arguments.size()-1 : fn->arguments.size();

                    for(size_t i = 0; i != n_expected; ++i){
                        Object obj;
                        if(i < n_args){
                            obj = args[i];
                        }
                        (*dict)[std::get<0>(fn->arguments[i])] = std::move(obj);
                    }

                    if(is_vararg){
                        Object ls;
                        if(n_args > n_expected){
                            ls = makeList(rt->gc, false,
                                ObjectVec_t(args.begin() + n_expected, args.end()));
                        } else {
                            ls = makeList(rt->gc, false);
                        }
                        (*dict)[std::get<0>(fn->arguments[n_expected])] = std::move(ls);
                    }

                    if(n_args == 0){
                        address = fn->address;
                    } else if(n_args < fn->arguments.size()){
                        address = std::get<1>(fn->arguments[n_args -1]);
                    } else {
                        address = std::get<1>(fn->arguments.back());
                    }
                }

                backInfo.codeBlocks.push_back(nullptr);
                backInfo.function = fn;
                backInfo.addr = rt->code.begin() + address;
                backInfo.box = rt->boxes[fn->boxName];
                backInfo.thisObject = self;
            }
        }
    }
}
