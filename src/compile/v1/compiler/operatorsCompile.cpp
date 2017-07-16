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
 *      File compile/v1/compiler/operatorsCompile.cpp
 *
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <tuple>

#include "compile/v1/Compiler.h"
#include "compile/Statement.h"
#include "parse/Tokenizer.h"
#include "runtime/gc.h"
#include "runtime/id.h"

using namespace sm::parse;
using namespace sm::compile;

namespace sm{
    namespace compile{
        namespace v1 {
            using namespace ParType;

            void Compiler::_operatorsCompile(CompilerStates& states){
                TokenVec_t::const_iterator& it = states.it;
                ParInfo_t& info = states.parStack.back();

                if(it->type >= TT_OPERATORS_START && it->type <= TT_OPERATORS_END){
                    enum_t closing = 0, closingSpecial = 0;
                    bool closed = false, doPop = false, doEndBlock = false,
                        resetOutput = false, doDeclareVar = false, doDeclareGlobalVar = false,
                        logicOr = false, logicAnd = false;

                    if(it->type == TT_ROUND_CLOSE){
                        if(info.parType == EXPR_ROUND){
                            if(it != states.begin && std::prev(it)->type == TT_ROUND_OPEN){
                                states.output->push_back(MAKE_VOID_TUPLE);
                                states.parStack.pop_back();
                                states.operators.pop_back();
                                states.isLastOperand = true;
                                return;
                            } else if(!states.isLastOperand){
                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                    "expected valid expression before ')'.");
                            }
                        }

                        if(std::prev(it)->type != TT_ROUND_OPEN && !states.isLastOperand){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected valid operand before ')'.");
                        }

                        for (ParStack_t::const_reverse_iterator it = states.parStack.rbegin();
                                it != states.parStack.rend(); ++it){
                            if(it->parType == FUNC_CALL || it->parType == TUPLE
                                    || it->parType == REF_CALL || it->parType == IS_NULL_CALL
                                    || it->parType == SUPER_FIND_CALL){
                                closing = TT_ROUND_OPEN;
                                closingSpecial = it->parType;
                                break;
                            } else if(it->parType == EXPR_ROUND){
                                closing = TT_ROUND_OPEN;
                                break;
                            } else if(it->parType == DEFAULT_ARGUMENT
                                    || it->parType == SUPER_EXPR){
                                closingSpecial = it->parType;
                                break;
                            } else if(it->isRound()){
                                closingSpecial = it->parType;
                                doPop = it->parType == FOR_HEAD3 && !states.wasStatementEmpty;
                                break;
                            }
                        }

                        states.isLastOperand = true;
                    } else if(it->type == TT_SQUARE_CLOSE){
                        if(states.parStack.back().parType != BRACING){
                            if(it != states.begin && std::prev(it)->type == TT_SQUARE_OPEN){
                                states.output->push_back(MAKE_VOID_LIST);
                                states.parStack.pop_back();
                                states.operators.pop_back();
                                states.isLastOperand = true;
                                return;
                            } else if(!states.isLastOperand){
                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                    "expected valid expression before ']'.");
                            }
                        }
                        states.isLastOperand = true;
                        closing = TT_SQUARE_OPEN;
                    } else if(it->type == TT_CURLY_CLOSE){
                        if(!states.isStatementEmpty){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' before '}'.");
                        }
                        if(!info.isCurly()){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected '{' before '}'.");
                        }

                        if(info.parType == CODE_BLOCK){
                            closing = TT_CURLY_OPEN;
                        } else {
                            closingSpecial = info.parType;
                        }

                        closed = true;
                        states.isLastOperand = true;
                        doEndBlock = info.parType != FUNCTION_BODY;
                    } else if(it->type == TT_SEMICOLON){
                        if(!states.isLastOperand && !states.isStatementEmpty){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected operand before ';'.");
                        }

                        doPop = !states.wasStatementEmpty && info.parType != FOR_HEAD2;
                        for (ParStack_t::const_reverse_iterator it = states.parStack.rbegin();
                                it != states.parStack.rend(); ++it){
                            if(it->isDeclaration() || it->isRound()){
                                continue;
                            } else if(it->parType == RETURN_STATEMENT){
                                closingSpecial = RETURN_STATEMENT;
                                doPop = false;
                                break;
                            } else if(it->parType == ELVIS_OPERATOR){
                                closingSpecial = ELVIS_OPERATOR;
                                break;
                            } else if(it->isSpecialStatement()) {
                                closingSpecial = it->parType - (_STATEMENTS_START - _CURLY_START);
                                doEndBlock = it->parType != FUNCTION_STATEMENT;
                                break;
                            } else if(it->isForStatement()){
                                closingSpecial = it->parType;
                                break;
                            } else {
                                states.isLastOperand = false;
                                break;
                            }
                        }

                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        states.expectedLvalue = false;
                    } else if(it->type == TT_COMMA){
                        if(!states.isLastOperand){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected operand before ','.");
                        }

                        for (ParStack_t::reverse_iterator it = states.parStack.rbegin();
                                it != states.parStack.rend(); ++it){
                            if(it->parType == GLOBAL_VAR_DECL){
                                doDeclareGlobalVar = true;
                                doPop = true;
                                break;
                            } else if(it->parType == VAR_DECL){
                                doDeclareVar = true;
                                doPop = true;
                                break;
                            } else if(it->parType == DEFAULT_ARGUMENT
                                    || it->parType == SUPER_EXPR){
                                closingSpecial = it->parType;
                                break;
                            } else if(it->isDeclaration()){
                                continue;
                            } else if(it->canCommaIncrement()) {
                                it->arg0++;
                                break;
                            } else if(it->parType == EXPR_ROUND){
                                it->arg0 = 2;
                                it->parType = TUPLE;
                                break;
                            } else {
                                break;
                            }
                        }

                        states.isLastOperand = true;
                    } else if(it->type == TT_QUESTION_MARK){
                        if(!states.isLastOperand){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected operand before '?'.");
                        }

                        closing = TT_IF_KW; // TT_QUESTION_MARK was alredy taken!
                        closed = true;
                    } else if(it->type == TT_COLON){
                        if(!states.isLastOperand){
                            if(states.parStack.back().parType == CONDITIONAL_OPERATOR1
                                    && states.operators.back().type == TT_QUESTION_MARK){
                                ParInfo_t& backInfo = states.parStack.back();
                                backInfo.parType = ELVIS_OPERATOR;
                                (*states.output)[backInfo.arg0-1] = ELVIS;
                                states.operators.back().type = TT_ELVIS;

                                return;
                            }

                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected operand before ':'.");
                        }

                        bool closingSwitchCase = false;

                        for (ParStack_t::const_reverse_iterator crit = states.parStack.rbegin();; ++crit){
                            if(crit == states.parStack.rend()){
                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                    "expected '?' before ':'.");
                            } else if(crit->isDeclaration()){
                                continue;
                            } else if(crit->parType == CONDITIONAL_OPERATOR1){
                                break;
                            } else if(crit->parType == CASE_HEAD){
                                closingSwitchCase = true;
                                break;
                            } else {
                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                    "expected '?' before ':'.");
                            }
                        }

                        if(closingSwitchCase){
                            closing = TT_CASE_KW;
                            closingSpecial = CASE_HEAD;
                        } else {
                            closing = TT_QUESTION_MARK;
                        }

                        states.isLastOperand = false;
                    } else if(it->type == TT_LOGIC_OR){
                        logicOr = true;
                    } else if(it->type == TT_LOGIC_AND){
                        logicAnd = true;
                    }

                    if(states.isLastOperand || it->type == TT_SEMICOLON
                            || it->type == TT_COLON){
                        states.isLastOperand = false;
                        Operator_t backOp;

                        while(!states.operators.empty()){
                            backOp = states.operators.back();

                            if(backOp.type == TT_ROUND_OPEN
                                    || backOp.type == TT_SQUARE_OPEN
                                    || backOp.type == TT_QUESTION_MARK
                                    || backOp.type == TT_CASE_KW){
                                if(closing){
                                    if(closing == backOp.type){
                                        if(closing != TT_QUESTION_MARK){
                                            states.isLastOperand = true;
                                            states.operators.pop_back();
                                        } else {
                                            states.operators.back().type = TT_COLON;
                                        }
                                        closed = true;
                                    } else if(closing != TT_IF_KW){
                                        char expected = 0;
                                        switch(backOp.type){
                                            case TT_ROUND_OPEN:
                                                expected = ')';
                                                break;
                                            case TT_SQUARE_OPEN:
                                                expected = ']';
                                                break;
                                            case TT_QUESTION_MARK:
                                                expected = ':';
                                                break;
                                        }
                                        _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                            std::string("expected '") + expected + "' before "
                                                + representation(*it) + ".");
                                    }
                                }
                                break;
                            }

                            if((backOp.pr <= static_cast<unsigned>(it->i) && !(backOp.pr == 13 && it->i == 13)) // also if tok and it are both assignment operators
                                    || (closing && closing != TT_IF_KW) || closingSpecial
                                    || it->type == TT_COMMA){
                                switch(backOp.type){
                                    case TT_ASSIGN:
                                        break;
                                    case TT_VAR_KW:{
                                        ParInfo_t& back = states.parStack.back();
                                        size_t idx = back.arg0;
                                        bool isGlobal = back.parType == GLOBAL_VAR_DECL;

                                        states.output->insert(states.output->end(), {
                                            bc(isGlobal ? DEFINE_GLOBAL_VAR : DEFINE_VAR),
                                            bc(idx >> 8), bc(idx & 0xFF)
                                        });
                                        states.parStack.pop_back();

                                        if(isGlobal){
                                            resetOutput = true;
                                        }
                                        break;
                                    }

                                    case TT_LOGIC_AND:
                                    case TT_LOGIC_OR:
                                    case TT_ELVIS:
                                    case TT_COLON:{
                                        size_t label = states.parStack.back().arg0;
                                        unsigned diff = states.output->size() - label -1;
                                        (*states.output)[label] = (diff >> 8) & 0xFF;
                                        (*states.output)[label+1] = diff & 0xFF;
                                        states.parStack.pop_back();
                                        break;
                                    }

                                    case TT_PLUS_ASSIGN:
                                    case TT_PLUS:
                                        states.output->push_back(ADD);
                                        break;
                                    case TT_MINUS_ASSIGN:
                                    case TT_MINUS:
                                        states.output->push_back(SUB);
                                        break;
                                    case TT_MULT_ASSIGN:
                                    case TT_MULT:
                                        states.output->push_back(MUL);
                                        break;
                                    case TT_DIV_ASSIGN:
                                    case TT_DIV:
                                        states.output->push_back(DIV);
                                        break;
                                    case TT_MOD_ASSIGN:
                                    case TT_MOD:
                                        states.output->push_back(MOD);
                                        break;
                                    case TT_OR_ASSIGN:
                                    case TT_OR:
                                        states.output->push_back(OR);
                                        break;
                                    case TT_AND_ASSIGN:
                                    case TT_AND:
                                        states.output->push_back(AND);
                                        break;
                                    case TT_XOR_ASSIGN:
                                    case TT_XOR:
                                        states.output->push_back(XOR);
                                        break;
                                    case TT_EQUAL:
                                        states.output->push_back(EQUAL);
                                        break;
                                    case TT_NOT_EQUAL:
                                        states.output->push_back(NOT_EQUAL);
                                        break;
                                    case TT_GREATER:
                                        states.output->push_back(GREATER);
                                        break;
                                    case TT_GREATER_OR_EQUAL:
                                        states.output->push_back(GREATER_OR_EQUAL);
                                        break;
                                    case TT_LESS:
                                        states.output->push_back(LESS);
                                        break;
                                    case TT_LESS_OR_EQUAL:
                                        states.output->push_back(LESS_OR_EQUAL);
                                        break;
                                    case TT_LEFT_SHIFT_ASSIGN:
                                    case TT_LEFT_SHIFT:
                                        states.output->push_back(LEFT_SHIFT);
                                        break;
                                    case TT_RIGHT_SHIFT_ASSIGN:
                                    case TT_RIGHT_SHIFT:
                                        states.output->push_back(RIGHT_SHIFT);
                                        break;
                                    case TT_NOT:
                                        states.output->push_back(NOT);
                                        break;
                                    case TT_COMPL:
                                        states.output->push_back(COMPL);
                                        break;
                                    case TT_PRE_INC:
                                        states.output->push_back(INC);
                                        break;
                                    case TT_PRE_DEC:
                                        states.output->push_back(DEC);
                                        break;
                                    case TT_PRE_PLUS:
                                        states.output->push_back(UNARY_PLUS);
                                        break;
                                    case TT_PRE_MINUS:
                                        states.output->push_back(UNARY_MINUS);
                                        break;
                                    default:
                                        _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                            std::string("unexpected operator: ")
                                                + representation(*it) + ".");
                                }

                                if(backOp.type == TT_ASSIGN){
                                    states.output->push_back(ASSIGN);
                                } else if(backOp.type >= TT_ASSIGN_START
                                        && backOp.type <= TT_ASSIGN_END){
                                    states.output->back() += ASSIGN_START - OPERATORS_START;
                                }
                            } else {
                                break;
                            }
                            states.operators.pop_back();
                        } // end while

                        if(doPop)
                            states.output->push_back(POP);
                        if(doEndBlock)
                            states.output->push_back(END_BLOCK);
                        if(!closing && !closingSpecial && it->type != TT_SEMICOLON
                                && it->type != TT_COMMA)
                            states.operators.emplace_back(it->type, it->i);
                        if(doDeclareVar){
                            _declareVar(states, *states.output, false);
                        } else if(doDeclareGlobalVar){
                            states.output = &_rt->code;
                            if(states.currClass){
                                _declareVar(states, _classTemp, true);
                            } else {
                                _declareVar(states, _temp, true);
                            }
                            resetOutput = false;
                        }
                        if(resetOutput)
                            states.output = &_rt->code;
                        if(logicOr) {
                            states.parStack.emplace_back(LOGIC_OR_OPERATOR);
                            states.output->insert(states.output->end(), {
                                LOGIC_OR, 0, 0
                            });
                            states.parStack.back().arg0 = states.output->size() - 2;
                        } else if(logicAnd){
                            states.parStack.emplace_back(LOGIC_AND_OPERATOR);
                            states.output->insert(states.output->end(), {
                                LOGIC_AND, 0, 0
                            });
                            states.parStack.back().arg0 = states.output->size() - 2;
                        }

                        if(it->type == TT_SEMICOLON && !states.operators.empty()){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "missing something before ';'.");
                        }

                        if(closing){
                            bool popBack = true;

                            if(!closed){
                                char expected = 0;
                                switch(it->type){
                                    case TT_ROUND_CLOSE:
                                        expected = '(';
                                        break;
                                    case TT_SQUARE_CLOSE:
                                        expected = '[';
                                        break;
                                    case TT_COLON:
                                        expected = '?';
                                        break;
                                }
                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                    std::string("expected '") + expected + "' before "
                                        + representation(*it) + ".");
                            }

                            if(closing == TT_IF_KW){
                                states.output->insert(states.output->end(), {
                                    JUMP_IF_NOT_F, 0, 0
                                });

                                states.parStack.emplace_back(CONDITIONAL_OPERATOR1);
                                states.operators.emplace_back(it->type, it->i);
                                states.parStack.back().arg0 = states.output->size() - 2;

                                states.isLastOperand = false;
                                return;
                            }

                            ParInfo_t& info = states.parStack.back();
                            if(info.arg0 >> 16){
                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                    "Args count can't be greater than 65535.");
                            }

                            if(info.parType == FUNC_CALL){ // sorry, I was bored of switches
                                states.output->insert(states.output->end(), {
                                    CALL_FUNCTION, bc(info.arg0 >> 8), bc(info.arg0 & 0xFF)
                                });
                            } else if(info.parType == BRACING){
                                states.output->insert(states.output->end(), {
                                    PERFORM_BRACING, bc(info.arg0 >> 8), bc(info.arg0 & 0xFF)
                                });
                            } else if(info.parType == TUPLE){
                                states.output->insert(states.output->end(), {
                                    MAKE_TUPLE, bc(info.arg0 >> 8), bc(info.arg0 & 0xFF)
                                });
                            } else if(info.parType == LIST){
                                states.output->insert(states.output->end(), {
                                    MAKE_LIST, bc(info.arg0 >> 8), bc(info.arg0 & 0xFF)
                                });
                            } else if(info.parType == CONDITIONAL_OPERATOR1){
                                states.output->insert(states.output->end(), {
                                    JUMP_F, 0, 0
                                });

                                size_t label = info.arg0;
                                unsigned diff = states.output->size() - label -1;
                                (*states.output)[label] = (diff >> 8) & 0xFF;
                                (*states.output)[label+1] = diff & 0xFF;

                                info.arg0 = states.output->size() -2;
                                info.parType = CONDITIONAL_OPERATOR2;
                                popBack = false;
                            } else if(info.parType == CASE_HEAD){
                                ParInfo_t& switchInfo = states.parStack[states.parStack.size() -2];

                                switchInfo.arg1 = states.output->size() +1;
                                states.output->insert(states.output->end(), {
                                    SWITCH_CASE, 0, 0
                                });

                                size_t label = switchInfo.arg2;
                                if(label){
                                    unsigned diff = states.output->size() - label -1;
                                    (*states.output)[label] = (diff >> 8) & 0xFF;
                                    (*states.output)[label+1] = diff & 0xFF;
                                }

                                switchInfo.arg2 = 0;
                                states.isStatementEmpty = true;
                                states.isLastOperand = false;
                            } else if(info.parType == REF_CALL){
                                if(states.wasStatementEmpty){
                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                        "reference must point to something.");
                                }

                                states.output->push_back(MAKE_REF);
                                states.isStatementEmpty = false;
                                states.isLastOperand = true;
                            } else if(info.parType == IS_NULL_CALL){
                                if(states.wasStatementEmpty){
                                    // `nothing` is `null` :D
                                    states.output->push_back(PUSH_INT_1);
                                } else {
                                    states.output->push_back(IS_NULL);
                                }

                                states.isStatementEmpty = false;
                                states.isLastOperand = true;
                            } else if(info.parType == SUPER_FIND_CALL){
                                if(states.wasStatementEmpty){
                                    states.output->emplace_back(PUSH_INT_0);
                                }

                                expect_next(*this, states, TT_DOT);
                                expect_next(*this, states, TT_TEXT);

                                unsigned nid = runtime::genOrdinaryId(*_rt, it->content)
                                        - runtime::idsStart;

                                states.output->insert(states.output->end(), {
                                    FIND_SUPER, bc(nid >> 8), bc(nid & 0xFF)
                                });
                                states.isStatementEmpty = false;
                                states.isLastOperand = true;
                            } else if(info.parType == VAR_DECL){
                                popBack = false;
                            }

                            /* else if(info.parType == CODE_BLOCK); // do nothing. */
                            if(popBack)
                                states.parStack.pop_back();
                        } else if(closingSpecial){
                            bool repeat = info.isCodeBlock() || info.parType == RETURN_STATEMENT;
                            do {
                                ParInfo_t& info = states.parStack.back(); // overrides above info

                                if(!closingSpecial){
                                    if(states.parStack.empty() && states.currClass
                                            && states.isClassStatement){
                                        if(!_classTemp.empty()){
                                            Function* fn = new Function;
                                            fn->address = _rt->code.size();
                                            fn->fnName = runtime::initId;
                                            fn->boxName = states.currBox->boxName;

                                            // inserting init code into bytecode and linking it to the class
                                            states.currClass->objects.insert({runtime::initId, makeFunction(fn)});
                                            _rt->code.insert(_rt->code.end(), _classTemp.begin(), _classTemp.end());
                                            _rt->code.push_back(RETURN_NULL);
                                            _classTemp.clear();
                                        }
                                        states.currClass = nullptr;
                                        states.isClassStatement = false;
                                        break;
                                    }

                                    closingSpecial = info.parType - (_STATEMENTS_START - _CURLY_START);
                                    if(closingSpecial != FUNCTION_BODY)
                                        states.output->push_back(END_BLOCK);
                                }

                                switch(closingSpecial){
                                    case IF_HEAD:{
                                        if(states.wasStatementEmpty){
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected valid expression inside if.");
                                        } else if(++it == states.end){
                                            --it;
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected valid expression before 'eof'.");
                                        } else if(it->type == TT_CURLY_OPEN){
                                            info.parType = IF_BODY;
                                        } else {
                                            info.parType = IF_STATEMENT;
                                            --it;
                                        }

                                        states.output->insert(states.output->end(), {
                                            JUMP_IF_NOT_F, 0, 0
                                        });

                                        info.arg0 = states.output->size() -2;
                                        states.output->push_back(START_BLOCK);
                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case WHILE_HEAD:{
                                        if(states.wasStatementEmpty){
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected expression inside 'while'.");
                                        } else if(++it == states.end){
                                            --it;
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected valid expression before 'eof'.");
                                        } else if(it->type == TT_CURLY_OPEN){
                                            info.parType = WHILE_BODY;
                                        } else {
                                            info.parType = WHILE_STATEMENT;
                                            --it;
                                        }

                                        states.output->insert(states.output->end(), {
                                            JUMP_IF_NOT_F, 0, 0
                                        });

                                        info.arg1 = states.output->size() -2;
                                        states.output->push_back(START_BLOCK);

                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case DO_WHILE_HEAD:{
                                        if(states.wasStatementEmpty){
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected expression inside 'while'.");
                                        } else if(++it == states.end){
                                            --it;
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected ';' after do-while.");
                                        } else if(it->type != TT_SEMICOLON){
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected ';' after do-while.");
                                        }

                                        size_t label = info.arg0;
                                        unsigned diff = states.output->size() - label +2;

                                        states.output->insert(states.output->end(), {
                                            JUMP_IF_B, bc(diff >> 8), bc(diff & 0xFF)
                                        });

                                        if(info.loopStatements){
                                            size_t target = states.output->size() -1;
                                            IndexVector_t& bvec = info.loopStatements->breaks,
                                                cvec = info.loopStatements->continues;
                                            IndexVector_t::const_iterator cit;

                                            for(cit = bvec.begin(); cit != bvec.end(); ++cit){
                                                label = *cit;
                                                diff = target -label;
                                                (*states.output)[label] = (diff >> 8) & 0xFF;
                                                (*states.output)[label+1] = diff & 0xFF;
                                            }

                                            delete info.loopStatements;
                                            info.loopStatements = nullptr;
                                        }

                                        states.parStack.pop_back();
                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case SWITCH_HEAD:{
                                        if(states.wasStatementEmpty){
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected expression inside 'switch'.");
                                        } else if(++it == states.end){
                                            --it;
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected '{' before 'eof'.");
                                        } else if(it->type != TT_CURLY_OPEN){
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected '{' before " + representation(*it) + ".");
                                        }

                                        info.parType = SWITCH_BODY;

                                        if(++it == states.end){
                                            --it;
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected '}' before 'eof'.");
                                        } else if(it->type != TT_CASE_KW && it->type != TT_DEFAULT_KW && it->type != TT_CURLY_CLOSE){
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "code outside case in a switch is not allowed.");
                                        }

                                        if(it->type != TT_CURLY_CLOSE){ // if it->type is now case or default keyword
                                            --it;
                                        }

                                        states.output->push_back(START_BLOCK);
                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case IF_BODY:{
                                        if(++it != states.end && it->type == TT_ELSE_KW){
                                            if(++it == states.end){
                                                --it;
                                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                    "expected valid expression before 'eof'.");
                                            } else if(it->type == TT_CURLY_OPEN){
                                                info.parType = ELSE_BODY;
                                            } else {
                                                info.parType = ELSE_STATEMENT;
                                                repeat = false;
                                                --it;
                                            }

                                            states.output->insert(states.output->end(), {
                                                JUMP_F, 0, 0
                                            });
                                            info.arg1 = states.output->size() -2;

                                            size_t label = info.arg0;
                                            unsigned diff = states.output->size() - label -1;
                                            (*states.output)[label] = (diff >> 8) & 0xFF;
                                            (*states.output)[label+1] = diff & 0xFF;

                                            states.output->push_back(START_BLOCK);
                                            states.isStatementEmpty = true;
                                            states.isLastOperand = false;
                                        } else {
                                            --it;
                                            size_t label = info.arg0;
                                            unsigned diff = states.output->size() - label -1;

                                            (*states.output)[label] = (diff >> 8) & 0xFF;
                                            (*states.output)[label+1] = diff & 0xFF;
                                            states.parStack.pop_back();
                                        }
                                        break;
                                    }

                                    case ELSE_BODY:{
                                        size_t label = info.arg1;
                                        unsigned diff = states.output->size() - label -1;

                                        (*states.output)[label] = (diff >> 8) & 0xFF;
                                        (*states.output)[label+1] = diff & 0xFF;
                                        states.parStack.pop_back();
                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case WHILE_BODY:{
                                        size_t label = info.arg0;
                                        unsigned diff = states.output->size() - label +2;
                                        states.output->insert(states.output->end(), {
                                            JUMP_B, bc(diff >> 8), bc(diff & 0xFF)
                                        });

                                        label = info.arg1;
                                        diff = states.output->size() - label -1;
                                        (*states.output)[label] = (diff >> 8) & 0xFF;
                                        (*states.output)[label+1] = diff & 0xFF;

                                        if(info.loopStatements){
                                            size_t target = states.output->size() -1;
                                            IndexVector_t& bvec = info.loopStatements->breaks,
                                                cvec = info.loopStatements->continues;
                                            IndexVector_t::const_iterator cit;

                                            for(cit = bvec.begin(); cit != bvec.end(); ++cit){
                                                label = *cit;
                                                diff = target -label;
                                                (*states.output)[label] = (diff >> 8) & 0xFF;
                                                (*states.output)[label+1] = diff & 0xFF;
                                            }

                                            target -= 4; // END_BLOCK
                                            for(cit = cvec.begin(); cit != cvec.end(); ++cit){
                                                label = *cit;
                                                diff = target -label;
                                                (*states.output)[label] = (diff >> 8) & 0xFF;
                                                (*states.output)[label+1] = diff & 0xFF;
                                            }

                                            delete info.loopStatements;
                                            info.loopStatements = nullptr;
                                        }

                                        states.parStack.pop_back();
                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case DO_BODY: {
                                        if(++it == states.end){
                                            --it;
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected while before 'eof'.");
                                        } else if(it->type != TT_WHILE_KW){
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                std::string("expected while before ") + representation(*it) + ".");
                                        } else if(++it == states.end){
                                            --it;
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected '(' before 'eof'.");
                                        } else if(it->type != TT_ROUND_OPEN){
                                            --it;
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                std::string("expected '(' before ") + representation(*it) + ".");
                                        }

                                        if(info.loopStatements){
                                            size_t target = states.output->size() -1, label, diff;
                                            IndexVector_t& cvec = info.loopStatements->continues;
                                            IndexVector_t::const_iterator cit;

                                            for(cit = cvec.begin(); cit != cvec.end(); ++cit){
                                                label = *cit;
                                                diff = target -label;
                                                (*states.output)[label] = (diff >> 8) & 0xFF;
                                                (*states.output)[label+1] = diff & 0xFF;
                                            }
                                        }

                                        info.parType = DO_WHILE_HEAD;
                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case FOREACH_HEAD: {
                                        if(++it == states.end){
                                            --it;
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected valid expression before 'eof'.");
                                        } else if(it->type == TT_CURLY_OPEN){
                                            info.parType = FOR_BODY;
                                        } else {
                                            info.parType = FOR_STATEMENT;
                                            --it;
                                        }

                                        states.output->insert(states.output->end(), {
                                            ITERATE,
                                            IT_NEXT,
                                            FOREACH_CHECK, 0, 0,
                                            START_BLOCK
                                        });

                                        // FOREACH_CHECK's 'param' address
                                        info.arg1 = states.output->size() -3;

                                        // IT_NEXT address -2 (FOR_BODY adds 2)
                                        info.arg2 = states.output->size() -7;

                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case FOR_HEAD1: {
                                        info.parType = FOR_HEAD2;
                                        info.arg0 = states.output->size();
                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case FOR_HEAD2: {
                                        info.parType = FOR_HEAD3;
                                        if(!states.wasStatementEmpty) {
                                            states.output->insert(states.output->end(), {
                                                JUMP_IF_NOT_F, 0, 0
                                            });
                                            info.arg1 = states.output->size() -2;
                                        }

                                        states.output->insert(states.output->end(), {
                                            JUMP_F, 0, 0
                                        });
                                        info.arg2 = states.output->size() -2;

                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case FOR_HEAD3: {
                                        if(++it == states.end){
                                            --it;
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                "expected valid expression before 'eof'.");
                                        } else if(it->type == TT_CURLY_OPEN){
                                            info.parType = FOR_BODY;
                                        } else {
                                            info.parType = FOR_STATEMENT;
                                            --it;
                                        }

                                        size_t label = info.arg0;
                                        unsigned diff = states.output->size() - label +2;
                                        states.output->insert(states.output->end(), {
                                                JUMP_B, bc(diff >> 8), bc(diff & 0xFF)
                                        });

                                        label = info.arg2;
                                        diff = states.output->size() - label -1;
                                        (*states.output)[label] = (diff >> 8) & 0xFF;
                                        (*states.output)[label+1] = diff & 0xFF;

                                        states.output->push_back(START_BLOCK);

                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case FOR_BODY: {
                                        size_t label = info.arg2 + 2;
                                        unsigned diff = states.output->size() - label +2;

                                        states.output->insert(states.output->end(), {
                                            JUMP_B, bc(diff >> 8), bc(diff & 0xFF)
                                        });

                                        label = info.arg1;
                                        if(label){
                                            diff = states.output->size() - label -1;
                                            (*states.output)[label] = (diff >> 8) & 0xFF;
                                            (*states.output)[label+1] = diff & 0xFF;
                                        }

                                        if(info.loopStatements){
                                            size_t target = states.output->size() -1;
                                            IndexVector_t& bvec = info.loopStatements->breaks,
                                                cvec = info.loopStatements->continues;
                                            IndexVector_t::const_iterator cit;

                                            for(cit = bvec.begin(); cit != bvec.end(); ++cit){
                                                label = *cit;
                                                diff = target -label;
                                                (*states.output)[label] = (diff >> 8) & 0xFF;
                                                (*states.output)[label+1] = diff & 0xFF;
                                            }

                                            target -= 4; // END_BLOCK
                                            for(cit = cvec.begin(); cit != cvec.end(); ++cit){
                                                label = *cit;
                                                diff = target -label;
                                                (*states.output)[label] = (diff >> 8) & 0xFF;
                                                (*states.output)[label+1] = diff & 0xFF;
                                            }

                                            delete info.loopStatements;
                                            info.loopStatements = nullptr;
                                        }

                                        states.output->push_back(END_BLOCK);
                                        states.parStack.pop_back();
                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case SUPER_EXPR: {
                                        states.output->push_back(MAKE_SUPER);
                                        unsigned supers = ++states.parStack.back().arg1;

                                        if(states.it->type != TT_ROUND_CLOSE){
                                            ++states.it; // skipping comma
                                            if(states.it->type == TT_TEXT){
                                                unsigned alias = runtime::genOrdinaryId(*_rt,states.it->content)
                                                        - runtime::idsStart;
                                                if(is_next(*this, states, TT_COLON)){
                                                    _classTemp.insert(_classTemp.end(), {
                                                        PUSH_INT_VALUE, bc(supers >> 8), bc(supers & 0xFF),
                                                        DEFINE_GLOBAL_VAR, bc(alias >> 8), bc(alias & 0xFF),
                                                        POP
                                                    });
                                                } else states.it -= 2;
                                            } else --states.it;

                                            states.isStatementEmpty = true;
                                            states.isLastOperand = false;
                                            // keeps SUPER_EXPR on the parStack
                                            break;
                                        } else ++states.it;

                                        if(states.it->type != TT_CURLY_OPEN){
                                            states.isClassStatement = true;
                                            --it;
                                        }

                                        states.currClass = states.parStack.back().classPtr;
                                        states.output->push_back(POP);
                                        states.output = &_rt->code;

                                        states.parStack.pop_back();

                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case SWITCH_BODY:{
                                        size_t label;
                                        unsigned diff;

                                        if(info.loopStatements){
                                            size_t breaksTarget = states.output->size() -2;
                                            IndexVector_t& ivec = info.loopStatements->breaks;

                                            for(IndexVector_t::const_iterator cit = ivec.begin(); cit != ivec.end(); ++cit){
                                                label = *cit;
                                                diff = breaksTarget -label;
                                                (*states.output)[label] = (diff >> 8) & 0xFF;
                                                (*states.output)[label+1] = diff & 0xFF;
                                            }

                                            delete info.loopStatements;
                                            info.loopStatements = nullptr;
                                        }

                                        label = info.arg1;
                                        if(label){
                                            diff = states.output->size() - label -2; // with -1 it will skip END_BLOCK
                                            (*states.output)[label] = (diff >> 8) & 0xFF;
                                            (*states.output)[label+1] = diff & 0xFF;
                                        }

                                        states.output->push_back(POP);
                                        states.parStack.pop_back();
                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case DEFAULT_ARGUMENT:{
                                        Function* fn = states.parStack.back().funcPtr;
                                        bool roundClose = it->type == TT_ROUND_CLOSE;

                                        if((it-1)->type != TT_ROUND_OPEN){
                                            states.output->push_back(POP);
                                            fn->arguments.push_back(std::make_pair(
                                                states.parStack.back().arg0,
                                                states.output->size()
                                            ));
                                        }

                                        if(!roundClose){
                                            while(1) {
                                                if(++it == states.end){
                                                    --it;
                                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                        "expected ')' or identifier before 'eof'.");
                                                } else if(it->type == TT_ROUND_CLOSE){
                                                    roundClose = true;
                                                    break;
                                                } else if(it->type == TT_TEXT){
                                                    size_t arg_id = runtime::genOrdinaryId(*_rt, it->content);
                                                    if(++it == states.end){
                                                        --it;
                                                        _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                            "expected ',' or '=' before 'eof'.");
                                                    } else if(it->type == TT_COMMA || (roundClose = (it->type == TT_ROUND_CLOSE))){
                                                        size_t name_id = arg_id - runtime::idsStart;
                                                        states.output->push_back(ASSIGN_NULL_POP);
                                                        states.output->push_back((name_id >> 8) & 0xFF);
                                                        states.output->push_back(name_id & 0xFF);
                                                        fn->arguments.push_back(std::make_pair(arg_id, states.output->size()));
                                                        if(roundClose)
                                                            break;
                                                    } else if(it->type == TT_ASSIGN){
                                                        states.parStack.back().arg0 = arg_id;
                                                        break;
                                                    } else if(it->type == TT_DOT){
                                                        for (int i = 0; i != 2; ++i){ /* three varargs dots!*/
                                                            if(++it == states.end){
                                                                --it;
                                                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                                    "expected '.' before 'eof'.");
                                                            } else if(it->type != TT_DOT){
                                                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                                    std::string("expected '.' before ")
                                                                    + representation(*it) + ".");
                                                            }
                                                        }

                                                        if(++it == states.end){
                                                            --it;
                                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                                "expected ')' before 'eof'.");
                                                        } else if (it->type != TT_ROUND_CLOSE){
                                                            if(it->type == TT_COMMA){
                                                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                                    "VARARG argument must be the last argument"
                                                                    " in the function definition.");
                                                            } else if(it->type == TT_ASSIGN){
                                                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                                    "VARARG argument cannot have a default value.");
                                                            } else {
                                                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                                    std::string("expected ')' before ")
                                                                    + representation(*it) + ".");
                                                            }
                                                        }

                                                        fn->arguments.push_back(std::make_pair(arg_id, states.output->size()));
                                                        fn->flags = FF_VARARGS;
                                                        roundClose = true;
                                                        break;
                                                    } else {
                                                        _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                            std::string("expected ',' or '=' before ")
                                                            + representation(*it) + ".");
                                                    }
                                                } else {
                                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                        std::string("expected ')' or identifier before ")
                                                        + representation(*it) + ".");
                                                }
                                            }
                                        }

                                        if(roundClose){
                                            if(++it == states.end){
                                                --it;
                                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                    "expected '{' before 'eof'.");
                                            } else if(it->type == TT_CURLY_OPEN) {
                                                states.parStack.back().parType = FUNCTION_BODY;
                                            } else {
                                                states.parStack.back().parType = FUNCTION_STATEMENT;
                                                --it;
                                            }
                                        } else {
                                            it -= 2;
                                        }

                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case FUNCTION_BODY: {
                                        states.output->push_back(RETURN_NULL);
                                        states.parStack.pop_back();
                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    case RETURN_STATEMENT:{
                                        states.output->push_back(states.wasStatementEmpty ? RETURN_NULL : RETURN);
                                        states.parStack.pop_back();
                                        states.isStatementEmpty = true;
                                        states.isLastOperand = false;
                                        break;
                                    }

                                    default: {
                                        _rt->sources.msg(error::BUG, _nfile, it->ln, it->ch,
                                            std::string("unsupported statement (with id = ")
                                            + std::to_string(static_cast<unsigned>(info.parType))
                                            + ") (err #3).");
                                        break;
                                    }
                                }
                                closingSpecial = 0;
                            } while (repeat && ((!states.parStack.empty() && states.parStack.back().isSpecialStatement())
                                    || (states.currClass && states.isClassStatement)));
                        }
                    } else {
                        _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                            std::string("expected valid operand before ")
                            + representation(*it) + ".");
                    }
                } else {
                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                        std::string("unexpected token: ")
                            + representation(*it) + ".");
                }
            }
        }
    }
}
