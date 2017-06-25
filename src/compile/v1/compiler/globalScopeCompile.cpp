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
 *      File compile/v1/compiler/gloablScopeCompile.cpp
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
#include "lib/stdlib.h"

using namespace sm::parse;
using namespace sm::compile;

namespace sm{
    namespace lib {
        _LibDecl(lang);
    }

    namespace compile{
        namespace v1{
            using namespace ParType;

            void Compiler::_globalScopeCompile(CompilerStates& states){
                TokenVec_t::const_iterator& it = states.it;

                if(it->type != TT_SEMICOLON && it->type != TT_COMMA && it->type != TT_COLON
                        && it->type != TT_ROUND_CLOSE   && it->type != TT_SQUARE_CLOSE
                        && it->type != TT_CURLY_CLOSE){
                    states.isStatementEmpty = false;
                }

                switch(it->type){
                    case TT_USING_KW: {
                        bool isAlredyImported = false;
                        if(++it != states.end){
                            if(it->type == TT_TEXT){
                                bool dot = false, first = false, second = false;
                                states.toImport->emplace_back();
                                std::string imported(it->content);

                                while(true){
                                    if(++it == states.end){
                                        --it;
                                        _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                            "expected valid expression before 'eof'.");
                                    }

                                    if(dot){
                                        if(it->type == TT_TEXT){
                                            imported += it->content;
                                            dot = false;
                                        } else {
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                std::string("expected identifier before ") + representation(*it) + ".");
                                        }
                                    } else if(first){
                                        if(it->type == TT_TEXT){
                                            std::get<1>(states.toImport->back()) = runtime::genOrdinaryId(*_rt, it->content) - runtime::idsStart;
                                            first = false;
                                            second = true;
                                        } else {
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                std::string("expected identifier before ") + representation(*it) + ".");
                                        }
                                    } else if(second){
                                        if(it->type == TT_SEMICOLON){
                                            break;
                                        } else {
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                std::string("expected ';' before ") + representation(*it) + ".");
                                        }
                                    } else {
                                        if(it->type == TT_DOT){
                                            imported.push_back('.');
                                            dot = true;
                                        } else if(it->type == TT_ASSIGN){
                                            first = true;
                                        } else {
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                std::string("expected '.' before ") + representation(*it) + ".");
                                        }
                                    }
                                }

                                {
                                    size_t match = 0;
                                    for(const string_t& imp : _rt->boxNames){
                                        if(imp == imported){
                                            isAlredyImported = true;
                                            std::get<0>(states.toImport->back()) = match;
                                            break;
                                        }
                                        match ++;
                                    }
                                }

                                if(!isAlredyImported){
                                    _rt->boxNames.emplace_back(imported);
                                    std::replace(imported.begin(), imported.end(), '.', fileSeparator);
                                    imported.append(".sm");
                                    unsigned id = _rt->boxNames.size()-1;
                                    bool found = false;

                                    for(const string_t& dir : paths){
                                        std::string path = dir + imported;
                                        error::CodeSource* src = readf(path);
                                        if(src){
                                            _rt->sources.newSource(src);
                                            found = true;
                                            break;
                                        }
                                    }

                                    if(!found){
                                        _rt->boxNames.back().push_back('!');
                                        lib::LibDict_t::const_iterator cit = lib::libs.find(_rt->boxNames.back());
                                        if(cit == lib::libs.end() || _rt->noStd){
                                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                                std::string("can't import '") + imported + "'. Make sure the file exists.");
                                        } else {
                                            Class* box = cit->second(*_rt, id);
                                            _rt->boxes.push_back(box);
                                            _rt->sources.newSource(nullptr);
                                        }
                                    }
                                    std::get<0>(states.toImport->back()) = id;
                                }
                            } else {
                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                    std::string("expected identifier before ") + representation(*it) + ".");
                            }
                        } else {
                            --it;
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected identifier before 'eof'.");
                        }
                        break;
                    }

                    case TT_FUNC_KW:{
                        if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected identifier, new, delete or overloadable operator before 'eof'.");
                        }

                        unsigned id = 0;

                        switch(it->type){
                            case TT_COMPL:              case TT_NOT:                case TT_PRE_DEC:
                            case TT_PRE_INC:            case TT_PRE_MINUS:          case TT_PRE_PLUS:
                            case TT_LEFT_SHIFT:         case TT_MINUS:              case TT_MULT:
                            case TT_DIV:                case TT_MOD:                case TT_PLUS:
                            case TT_RIGHT_SHIFT:        case TT_OR:                 case TT_AND:
                            case TT_XOR:                case TT_LOGIC_AND:          case TT_LOGIC_OR:
                            case TT_EQUAL:              case TT_GREATER:            case TT_GREATER_OR_EQUAL:
                            case TT_LESS:               case TT_LESS_OR_EQUAL:      case TT_NOT_EQUAL:
                                id = runtime::operatorId(it->type);
                                break;

                            case TT_TEXT:
                                id = runtime::genOrdinaryId(*_rt, it->content);
                                break;

                            case TT_ROUND_OPEN:
                                if(++it == states.end ){
                                    --it;
                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                        "expected ')' before 'eof'.");
                                } else if(it->type != TT_ROUND_CLOSE){
                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                        std::string("expected ')' before ")
                                        + representation(*it) + ".");
                                }
                                id = runtime::roundId;
                                break;

                            case TT_SQUARE_OPEN:
                                if(++it == states.end ){
                                    --it;
                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                        "expected ']' before 'eof'.");
                                } else if(it->type != TT_SQUARE_CLOSE){
                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                        std::string("expected ']' before ")
                                        + representation(*it) + ".");
                                }
                                id = runtime::squareId;
                                break;

                            default:
                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                    std::string("expected identifier, new, delete or overloadable operator before")
                                    + representation(*it) + ".");
                                break;
                        }

                        Function* fn = new Function;
                        fn->address = states.output->size();
                        fn->boxName = states.currBox->boxName;
                        fn->fnName = id;
                        states.currBox->objects.insert(std::make_pair(id, makeFunction(fn)));

                        if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                "expected '(' or '{' before 'eof'.");
                        }

                        if(it->type != TT_ROUND_OPEN){
                            if(it->type == TT_CURLY_OPEN){
                                states.parStack.emplace_back(FUNCTION_BODY);
                            } else {
                                states.parStack.emplace_back(FUNCTION_STATEMENT);
                                --it;
                            }

                            states.isStatementEmpty = true;
                            states.isLastOperand = false;
                            break;
                        }

                        size_t arg0 = 0;
                        bool roundClose = false;

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
                                } else if(it->type == TT_COMMA || (roundClose = it->type == TT_ROUND_CLOSE)){
                                    size_t name_id = arg_id - runtime::idsStart;
                                    states.output->push_back(ASSIGN_NULL_POP);
                                    states.output->push_back((name_id >> 8) & 0xFF);
                                    states.output->push_back(name_id & 0xFF);
                                    fn->arguments.push_back(std::make_pair(arg_id, states.output->size()));
                                    if(roundClose)
                                        break;
                                } else if(it->type == TT_ASSIGN){
                                    it -= 2;
                                    arg0 = arg_id;
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

                        if(roundClose){
                            if(++it == states.end){
                                --it;
                                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                    "expected '{' before 'eof'.");
                            } else if(it->type == TT_CURLY_OPEN) {
                                states.parStack.emplace_back(FUNCTION_BODY);
                            } else {
                                states.parStack.emplace_back(FUNCTION_STATEMENT);
                                --it;
                            }
                        } else {
                            states.parStack.emplace_back(DEFAULT_ARGUMENT);
                        }

                        states.parStack.back().funcPtr = fn;
                        states.parStack.back().arg0 = arg0;

                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_VAR_KW: {
                        _declareVar(states, true);
                        break;
                    }

                    case TT_SEMICOLON:{
                        if(!states.isStatementEmpty){
                            _temp.push_back(POP);
                            states.isStatementEmpty = true;
                        }
                        break;
                    }

                    default:{
                        _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                            std::string("expected class, function, import, var or const before ")
                            + representation(*it) + ".");
                    }
                }
            }
        }
    }
}
