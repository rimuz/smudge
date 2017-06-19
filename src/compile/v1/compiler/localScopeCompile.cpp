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
 *      File compile/v1/compiler/localScopeCompile.cpp
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

            void Compiler::_localScopeCompile(CompilerStates& states){
                TokenVec_t::const_iterator& it = states.it;

                ParInfo_t& info = states.parStack.back();
                states.wasStatementEmpty = states.isStatementEmpty;
                if(it->type != TT_SEMICOLON && it->type != TT_COMMA && it->type != TT_COLON
                        && it->type != TT_ROUND_CLOSE   && it->type != TT_SQUARE_CLOSE
                        && it->type != TT_CURLY_CLOSE){
                    states.isStatementEmpty = false;
                } // if it->type == TT_SEMICOLON or TT_COMMA etc.., states.isStatementEmpty keep its value.

                switch(it->type){
                    case TT_TEXT: {
                        unsigned idx = runtime::genOrdinaryId(*_rt, it->content) - runtime::idsStart;

                        if(states.isLastDot){
                            states.output->push_back(FIND);
                            states.output->push_back(idx >> 8);
                            states.output->push_back(idx & 0xFF);
                            states.isLastDot = false;
                            break;
                        }

                        states.output->push_back(PUSH_REF);
                        states.output->push_back(idx >> 8);
                        states.output->push_back(idx & 0xFF);
                        states.isLastText = true;
                        break;
                    }

                    case TT_INTEGER:{
                        if(it->i == 0){
                            states.output->push_back(PUSH_INT_0);
                        } else if(it->i == 1){
                            states.output->push_back(PUSH_INT_1);
                        } else {
                            IntsMap_t::const_iterator n_it = _ints.find(it->i);
                            unsigned idx;
                            if(n_it == _ints.end()){
                                _rt->intConstants.push_back(it->i);
                                idx = _rt->intConstants.size() -1;
                                _ints[it->i] = idx;
                            } else {
                                idx = n_it->second;
                            }

                            states.output->push_back(PUSH_INTEGER);
                            states.output->push_back(idx >> 8);
                            states.output->push_back(idx & 0xFF);
                        }
                        break;
                    }

                    case TT_FLOAT:{
                        FloatsMap_t::const_iterator n_it = _floats.find(it->f);
                        unsigned idx;
                        if(n_it == _floats.end()){
                            _rt->floatConstants.push_back(it->f);
                            idx = _rt->floatConstants.size() -1;
                            _floats[it->f] = idx;
                        } else {
                            idx = n_it->second;
                        }

                        states.output->push_back(PUSH_FLOAT);
                        states.output->push_back(idx >> 8);
                        states.output->push_back(idx & 0xFF);
                        break;
                    }

                    case TT_STRING:{
                        StringsMap_t::const_iterator n_it = _strings.find(it->content);
                        unsigned idx;
                        if(n_it == _strings.end()){
                            _rt->stringConstants.emplace_back(it->content.c_str());
                            idx = _rt->stringConstants.size() -1;
                            _strings[it->content] = idx;
                        } else {
                            idx = n_it->second;
                        }

                        states.output->push_back(PUSH_STRING);
                        states.output->push_back(idx >> 8);
                        states.output->push_back(idx & 0xFF);
                        break;
                    }

                    case TT_VAR_KW:{
                        _declareVar(states, false);
                        break;
                    }

                    case TT_FALSE_KW:{
                        states.output->push_back(PUSH_INT_0);
                        break;
                    }

                    case TT_TRUE_KW:{
                        states.output->push_back(PUSH_INT_1);
                        break;
                    }

                    case TT_IF_KW:{
                        if(!states.wasStatementEmpty){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' before 'if'.");
                        }
                        if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected '(' before 'eof'.");
                        } else if(it->type != TT_ROUND_OPEN){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                std::string("expected '(' before ") + representation(*it) + ".");
                        }

                        states.parStack.emplace_back(IF_HEAD);
                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_ELSE_KW:{
                        _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                            "epected 'if' before 'else'.");
                        break;
                    }

                    case TT_WHILE_KW:{
                        if(!states.wasStatementEmpty){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' before 'while'.");
                        }
                        if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected '(' before 'eof'.");
                        } else if(it->type != TT_ROUND_OPEN){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                std::string("expected '(' before ") + representation(*it) + ".");
                        }

                        states.parStack.emplace_back(WHILE_HEAD);
                        states.parStack.back().arg0 = states.output->size();
                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_DO_KW:{
                        if(!states.wasStatementEmpty){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' before 'do'.");
                        }
                        if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected '{' or expression before 'eof'.");
                        }

                        if(it->type == TT_CURLY_OPEN){
                            states.parStack.emplace_back(DO_BODY);
                        } else {
                            --it;
                            states.parStack.emplace_back(DO_STATEMENT);
                        }

                        states.parStack.back().arg0 = states.output->size();
                        states.output->push_back(START_BLOCK);
                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_FOR_KW:{
                        if(!states.wasStatementEmpty){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' before 'for'.");
                        }
                        if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected '(' before 'eof'.");
                        } else if(it->type != TT_ROUND_OPEN){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                std::string("expected '(' before ") + representation(*it) + ".");
                        }

                        states.parStack.emplace_back(FOR_HEAD1);
                        states.output->push_back(START_BLOCK);
                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_SWITCH_KW:{
                        if(!states.wasStatementEmpty){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' before 'switch'.");
                        }
                        if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected '(' before 'eof'.");
                        } else if(it->type != TT_ROUND_OPEN){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                std::string("expected '(' before ") + representation(*it) + ".");
                        }

                        states.parStack.emplace_back(SWITCH_HEAD);
                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_CASE_KW:{
                        if(!states.wasStatementEmpty){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' before 'case'.");
                        }

                        if(info.parType != SWITCH_BODY){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "'case' allowed only inside a switch block.");
                        }

                        if(info.arg0){ // if this is not the first case/defualt
                            info.arg2 = states.output->size() +1;
                            states.output->push_back(JUMP_F);
                            states.output->push_back(0);
                            states.output->push_back(0);
                        }

                        size_t label = info.arg1;
                        if(label){
                            unsigned diff = states.output->size() - label -1;
                            (*states.output)[label] = (diff >> 8) & 0xFF;
                            (*states.output)[label+1] = diff & 0xFF;
                        }

                        info.arg0 = 1;
                        info.arg1 = 0;
                        states.parStack.emplace_back(CASE_HEAD);
                        states.operators.emplace_back(TT_CASE_KW, operatorPriorities[TT_ROUND_OPEN - TT_OPERATORS_START]);
                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_DEFAULT_KW:{
                        if(!states.wasStatementEmpty){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' before 'default'.");
                        }

                        if(info.parType != SWITCH_BODY){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "'default' allowed only inside a switch block.");
                        }

                        if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ':' before 'eof'.");
                        }

                        if(it->type != TT_COLON){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ':' after 'default'.");
                        }

                        size_t label = info.arg1;
                        if(label){
                            unsigned diff = states.output->size() - label -1;
                            (*states.output)[label] = (diff >> 8) & 0xFF;
                            (*states.output)[label+1] = diff & 0xFF;
                        }

                        info.arg0 = 1;
                        info.arg1 = 0;
                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_BREAK_KW:{
                        if(!states.wasStatementEmpty){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' before 'break'.");
                        }

                        if(!info.isCodeBlock()){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "'break' allowed only in a distinct statement inside a cycle.");
                        }

                        if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' after 'break'.");
                        } else if(it->type != TT_SEMICOLON){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' after 'break'.");
                        }
                        --it;

                        size_t blocksToClose = 0;
                        ParInfo_t* loop = nullptr;

                        for(ParStack_t::reverse_iterator rit = states.parStack.rbegin(); rit != states.parStack.rend(); ++rit){
                            if(rit->parType == EXECUTABLE_STATEMENT){
                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                    "'break' is not allowed outside loops.");
                            } else if(rit->parType == RETURN_STATEMENT){
                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                    "expected ';' before 'break'.");
                            } else if(rit->isCurly()){
                                if(rit->parType == WHILE_BODY || rit->parType == FOR_BODY
                                        || rit->parType == DO_BODY){
                                    ++blocksToClose;
                                    loop = &*rit;
                                    break;
                                } else if(rit->parType == SWITCH_BODY){
                                    loop = &*rit;
                                    break;
                                } else if(rit->parType == FUNCTION_BODY){
                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                        "'break' is not allowed outside loops.");
                                }
                                ++blocksToClose;
                            } if(rit->isSpecialStatement()){
                                if(rit->parType == WHILE_STATEMENT || rit->parType == FOR_STATEMENT
                                        || rit->parType == DO_STATEMENT){
                                    ++blocksToClose;
                                    loop = &*rit;
                                    break;
                                } else if(rit->parType == FUNCTION_STATEMENT){
                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                        "'break' is not allowed outside loops.");
                                }
                                ++blocksToClose;
                            } else {
                                _rt->sources.msg(error::BUG, _nfile, it->ln, it->ch,
                                    "expected only CODE_BLOCKs outside CODE_BLOCK (err #1)");
                            }
                        }

                        if(blocksToClose) {
                            if(blocksToClose == 1){
                                states.output->push_back(END_BLOCK);
                            } else if(blocksToClose == 2){
                                states.output->push_back(END_BLOCK);
                                states.output->push_back(END_BLOCK);
                            } else {
                                if(blocksToClose >> 16){
                                    _rt->sources.msg(error::BUG, _nfile, it->ln, it->ch,
                                        "cannot close more than 65535 scopes.");
                                }

                                states.output->push_back(END_BLOCK);
                                states.output->push_back((blocksToClose >> 8) & 0xFF);
                                states.output->push_back(blocksToClose & 0xFF);
                            }
                        }

                        if(!loop->loopStatements)
                            loop->loopStatements = new LoopStatements_t;

                        loop->loopStatements->breaks.push_back(states.output->size() +1);

                        states.output->push_back(JUMP_F);
                        states.output->push_back(0);
                        states.output->push_back(0);

                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_CONTINUE_KW:{
                        if(!states.wasStatementEmpty){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' before 'continue'.");
                        }

                        if(!info.isCodeBlock()){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "'continue' allowed only in a distinct statement inside a cycle.");
                        }

                        if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' after 'continue'.");
                        } else if(it->type != TT_SEMICOLON){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' after 'continue'.");
                        }
                        --it;

                        size_t blocksToClose = 0;
                        ParInfo_t* loop = nullptr;

                        for(ParStack_t::reverse_iterator rit = states.parStack.rbegin(); rit != states.parStack.rend(); ++rit){
                            if(rit->parType == EXECUTABLE_STATEMENT){
                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                    "'continue' is not allowed outside loops.");
                            } else if(rit->parType == RETURN_STATEMENT){
                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                    "expected ';' before 'continue'.");
                            } else if(rit->isCurly()){
                                if(rit->parType == WHILE_BODY || rit->parType == FOR_BODY
                                        || rit->parType == DO_BODY){
                                    loop = &*rit;
                                    break;
                                } else if(rit->parType == FUNCTION_BODY){
                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                        "'continue' is not allowed outside loops.");
                                }
                                ++blocksToClose;
                            } else if(rit->isSpecialStatement()){
                                if(rit->parType == EXECUTABLE_STATEMENT){
                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                        "'continue' is not allowed outside loops.");
                                } else if(rit->parType == WHILE_STATEMENT || rit->parType == FOR_STATEMENT
                                        || rit->parType == DO_STATEMENT){
                                    loop = &*rit;
                                    break;
                                } else if(rit->parType == FUNCTION_STATEMENT){
                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                        "'break' is not allowed outside loops.");
                                }
                                ++blocksToClose;
                            } else {
                                _rt->sources.msg(error::BUG, _nfile, it->ln, it->ch,
                                    "expected only CODE_BLOCKs outside CODE_BLOCK (err #2)");
                            }
                        }

                        if(blocksToClose) {
                            if(blocksToClose == 1){
                                states.output->push_back(END_BLOCK);
                            } else if(blocksToClose == 2){
                                states.output->push_back(END_BLOCK);
                                states.output->push_back(END_BLOCK);
                            } else {
                                if(blocksToClose >> 16){
                                    _rt->sources.msg(error::BUG, _nfile, it->ln, it->ch,
                                        "cannot close more than 65535 scopes.");
                                }

                                states.output->push_back(END_BLOCK);
                                states.output->push_back((blocksToClose >> 8) & 0xFF);
                                states.output->push_back(blocksToClose & 0xFF);
                            }
                        }

                        if(!loop->loopStatements)
                            loop->loopStatements = new LoopStatements_t;

                        loop->loopStatements->continues.push_back(states.output->size() +1);

                        states.output->push_back(JUMP_F);
                        states.output->push_back(0);
                        states.output->push_back(0);

                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_RETURN_KW:{
                        if(!states.wasStatementEmpty){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected ';' before 'return'.");
                        } else if(!info.isCodeBlock()){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "'return' is allowed only in a distinct statement inside function.");
                        }

                        states.parStack.emplace_back(RETURN_STATEMENT);
                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_REF_KW:{
                        if(states.isLastOperand){
                            _rt->sources.msg(error::ERROR, _nfile, states.it->ln, states.it->ch,
                                "expected operator before 'ref'.");
                        }

                        if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ERROR, _nfile, states.it->ln, states.it->ch,
                                "expected '(' before 'eof'.");
                        } else if(it->type != TT_ROUND_OPEN){
                            _rt->sources.msg(error::ERROR, _nfile, states.it->ln, states.it->ch,
                                std::string("expected '(' before ") + representation(*it) + ".");
                        }

                        states.operators.emplace_back(TT_ROUND_OPEN, parse::operatorPriorities[TT_ROUND_OPEN - TT_OPERATORS_START]);
                        states.parStack.emplace_back(REF_CALL);
                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_NULL_KW:{
                        if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ERROR, _nfile, states.it->ln, states.it->ch,
                                "expected '(' or ';' before 'eof'.");
                        } else if(it->type == TT_ROUND_OPEN){
                            states.operators.emplace_back(TT_ROUND_OPEN, parse::operatorPriorities[TT_ROUND_OPEN - TT_OPERATORS_START]);
                            states.parStack.emplace_back(IS_NULL_CALL);
                            states.isStatementEmpty = true;
                            states.isLastOperand = false;
                        } else {
                            --it;
                            states.output->push_back(PUSH_NULL);
                            states.isLastOperand = true;
                        }

                        break;
                    }

                    case TT_THIS_KW:{
                        states.output->push_back(PUSH_THIS);
                        states.isLastOperand = true;
                        break;
                    }

                    case TT_BOX_KW:{
                        states.output->push_back(PUSH_BOX);
                        states.isLastOperand = true;
                        break;
                    }

                    case TT_CLASS_KW:{
                        states.output->push_back(PUSH_CLASS);
                        states.isLastOperand = true;
                        break;
                    }

                    case TT_NOT:{
                        states.preOperators.push_back(NOT);
                        break;
                    }

                    case TT_COMPL:{
                        states.preOperators.push_back(COMPL);
                        break;
                    }

                    case TT_PRE_INC:{
                        states.preOperators.push_back(INC);
                        break;
                    }

                    case TT_POST_INC:{
                        states.output->push_back(POST_INC);
                        break;
                    }

                    case TT_POST_DEC:{
                        states.output->push_back(POST_DEC);
                        break;
                    }

                    case TT_PRE_DEC: {
                        states.preOperators.push_back(DEC);
                        break;
                    }

                    case TT_PRE_PLUS:{
                        states.preOperators.push_back(UNARY_PLUS);
                        break;
                    }

                    case TT_PRE_MINUS:{
                        states.preOperators.push_back(UNARY_MINUS);
                        break;
                    }

                    case TT_ROUND_OPEN:{
                        states.parStack.emplace_back(states.isLastOperand ? FUNC_CALL : EXPR_ROUND);
                        states.operators.emplace_back(it->type, it->i);
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_SQUARE_OPEN:{
                        states.parStack.emplace_back(states.isLastOperand ? BRACING : LIST);
                        states.operators.emplace_back(it->type, it->i);
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_CURLY_OPEN:{
                        if(info.isSpecialStatement()){
                            info.parType -= _STATEMENTS_START - _CURLY_START;
                        } else if(!info.isCurly()){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected valid expression before '{'.");
                        } else {
                            states.parStack.emplace_back(CODE_BLOCK);
                            states.isLastOperand = false;
                            states.isStatementEmpty = true;
                            states.output->push_back(START_BLOCK);
                        }
                        break;
                    }

                    case TT_ELVIS:{
                        if(!states.isLastOperand){
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected operand before '?:'.");
                        }

                        states.parStack.emplace_back(ELVIS_OPERATOR);
                        states.operators.emplace_back(it->type, it->i);
                        states.isLastOperand = false;

                        states.output->push_back(ELVIS);
                        states.output->push_back(0);
                        states.output->push_back(0);

                        states.parStack.back().arg0 = states.output->size() - 2;
                        break;
                    }

                    case TT_DOT:{
                        if(states.isLastOperand){
                            states.isLastDot = true;
                            // keeps isLastOperand to true
                        } else {
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected operand before '.'");
                        }
                        break;
                    }

                    default:{
                        _operatorsCompile(states);
                    } // end default
                } // end switch!
            }
        }
    }
}