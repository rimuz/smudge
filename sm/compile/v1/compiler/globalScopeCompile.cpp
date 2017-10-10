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
 *      File compile/v1/compiler/globalScopeCompile.cpp
 *
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <tuple>

#include "sm/compile/v1/Compiler.h"
#include "sm/compile/Statement.h"
#include "sm/parse/Tokenizer.h"
#include "sm/runtime/gc.h"
#include "sm/runtime/id.h"
#include "sm/lib/stdlib.h"

using namespace sm::parse;
using namespace sm::compile;

namespace sm{
    namespace lib {
        smLibDecl(lang);
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
                        if(states.isLastOperand){
                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                "expected operator before 'import'.");
                        } else if(states.currClass){
                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                "cannot import inside a class.");
                        }

                        bool repeat;
                        do {
                            bool isAlredyImported = false;
                            repeat = false;

                            if(++it != states.end){
                                if(it->type == TT_TEXT){
                                    bool dot = false, first = false, second = false;
                                    states.toImport->emplace_back();
                                    std::string imported(it->content);

                                    while(true){
                                        if(++it == states.end){
                                            --it;
                                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                                "expected valid expression before 'eof'.");
                                        }

                                        if(dot){
                                            if(it->type == TT_TEXT){
                                                imported += it->content;
                                                dot = false;
                                            } else {
                                                _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                                    std::string("expected identifier before ") + representation(*it) + ".");
                                            }
                                        } else if(first){
                                            if(it->type == TT_TEXT){
                                                std::get<1>(states.toImport->back()) = runtime::genOrdinaryId(*_rt, it->content) - runtime::idsStart;
                                                first = false;
                                                second = true;
                                            } else {
                                                _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                                    std::string("expected identifier before ") + representation(*it) + ".");
                                            }
                                        } else if(second){
                                            if(it->type == TT_SEMICOLON){
                                                break;
                                            } else if(it->type == TT_COMMA){
                                                repeat = true;
                                                break;
                                            } else {
                                                _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                                    std::string("expected ';' before ") + representation(*it) + ".");
                                            }
                                        } else {
                                            if(it->type == TT_DOT){
                                                imported.push_back('.');
                                                dot = true;
                                            } else if(it->type == TT_SEMICOLON || it->type == TT_COMMA){
                                                size_t dot_pos = imported.find_last_of('.');
                                                unsigned id;
                                                if(dot_pos == std::string::npos)
                                                    id = runtime::genOrdinaryId(*_rt, imported)
                                                        - runtime::idsStart;
                                                else
                                                    id = runtime::genOrdinaryId(*_rt, imported.substr(dot_pos+1))
                                                        - runtime::idsStart;
                                                std::get<1>(states.toImport->back()) = id;
                                                repeat = it->type == TT_COMMA;
                                                break;
                                            } else if(it->type == TT_ASSIGN){
                                                first = true;
                                            } else {
                                                _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                                    std::string("expected '.' before ") + representation(*it) + ".");
                                            }
                                        }
                                    }

                                    {
                                        size_t match = 0;
                                        for(const string_t& imp : _rt->boxNames){
                                            if(imp == imported || (!imp.empty()
                                                    && imp.size() == imported.size()+1 && imp.back() == '!'
                                                    && std::equal(imp.begin(), imp.end()-1, imported.begin()))){
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
                                        unsigned id = _rt->boxNames.size()-1;
                                        bool found = false;

                                        for(const string_t& dir : _rt->paths){
                                            std::string path = dir + imported;
                                            error::CodeSource* src = readf(path + ".sm");
                                            if(src){
                                                _rt->sources.newSource(src);
                                                _rt->boxes.push_back(nullptr);
                                                found = true;
                                                break;
                                            } else {
                                                Box* box;
                                                path += _SM_DL_EXT;
                                                if(load_native(path.c_str(), *_rt, id, box)){
                                                    if(!box)
                                                        _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                                            std::string("dynamic library '") + path + "' is not a Smudge native box.");
                                                    _rt->boxNames.back().push_back('!');
                                                    _rt->boxes.push_back(box);
                                                    _rt->sources.newSource(nullptr);
                                                    found = true;
                                                    break;
                                                }
                                            }
                                        }

                                        if(!found){
                                            _rt->boxNames.back().push_back('!');
                                            lib::LibDict_t::const_iterator cit = lib::libs.find(_rt->boxNames.back());
                                            if(cit == lib::libs.end() || _rt->noStd){
                                                _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                                    std::string("can't import '") + imported + "'. Make sure the file exists.");
                                            } else {
                                                Box* box = cit->second(*_rt, id);
                                                _rt->boxes.push_back(box);
                                                _rt->sources.newSource(nullptr);
                                            }
                                        }
                                        std::get<0>(states.toImport->back()) = id;
                                    }
                                } else {
                                    _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                        std::string("expected identifier before ") + representation(*it) + ".");
                                }
                            } else {
                                --it;
                                _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                    "expected identifier before 'eof'.");
                            }
                        } while(repeat);
                        break;
                    }

                    case TT_FUNC_KW:{
                        if(states.isLastOperand){
                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                "expected operator before 'func'.");
                        } else if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                "expected identifier, new, delete or overloadable operator before 'eof'.");
                        }

                        unsigned id = 0;

                        switch(it->type){
                            case TT_COMPL:              case TT_PRE_DEC:
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
                                    _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                        "expected ')' before 'eof'.");
                                } else if(it->type != TT_ROUND_CLOSE){
                                    _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                        std::string("expected ')' before ")
                                        + representation(*it) + ".");
                                }
                                id = runtime::roundId;
                                break;

                            case TT_SQUARE_OPEN:
                                if(++it == states.end ){
                                    --it;
                                    _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                        "expected ']' before 'eof'.");
                                } else if(it->type != TT_SQUARE_CLOSE){
                                    _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                        std::string("expected ']' before ")
                                        + representation(*it) + ".");
                                }
                                id = runtime::squareId;
                                break;

                            default:
                                _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                    std::string("expected identifier, new, delete or overloadable operator before ")
                                    + representation(*it) + ".");
                                break;
                        }

                        Function* fn = new Function;
                        fn->address = states.output->size();
                        fn->boxName = states.currBox->name;
                        fn->fnName = id;

                        if(states.currClass)
                            states.currClass->objects.insert({id, RootObject(makeFunction(fn))});
                        else
                            states.currBox->objects.insert({id, makeFunction(fn)});

                        if(++it == states.end){
                            --it;
                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
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
                                _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                    "expected ')' or identifier before 'eof'.");
                            } else if(it->type == TT_ROUND_CLOSE){
                                roundClose = true;
                                break;
                            } else if(it->type == TT_TEXT){
                                size_t arg_id = runtime::genOrdinaryId(*_rt, it->content);
                                if(++it == states.end){
                                    --it;
                                    _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                        "expected ',' or '=' before 'eof'.");
                                } else if(it->type == TT_COMMA || (roundClose = it->type == TT_ROUND_CLOSE)){
                                    size_t name_id = arg_id - runtime::idsStart;
                                    states.output->insert(states.output->end(), {
                                        ASSIGN_NULL_POP, bc(name_id >> 8), bc(name_id & 0xFF)
                                    });
                                    fn->arguments.emplace_back(arg_id, states.output->size());
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
                                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                                "expected '.' before 'eof'.");
                                        } else if(it->type != TT_DOT){
                                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                                std::string("expected '.' before ")
                                                + representation(*it) + ".");
                                        }
                                    }

                                    if(++it == states.end){
                                        --it;
                                        _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                            "expected ')' before 'eof'.");
                                    } else if (it->type != TT_ROUND_CLOSE){
                                        if(it->type == TT_COMMA){
                                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                                "VARARG argument must be the last argument"
                                                " in the function definition.");
                                        } else if(it->type == TT_ASSIGN){
                                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                                "VARARG argument cannot have a default value.");
                                        } else {
                                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                                std::string("expected ')' before ")
                                                + representation(*it) + ".");
                                        }
                                    }

                                    fn->arguments.emplace_back(arg_id, states.output->size());
                                    fn->flags = FF_VARARGS;
                                    roundClose = true;
                                    break;
                                } else {
                                    _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                        std::string("expected ',' or '=' before ")
                                        + representation(*it) + ".");
                                }
                            } else {
                                _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                    std::string("expected ')' or identifier before ")
                                    + representation(*it) + ".");
                            }
                        }

                        if(roundClose){
                            if(++it == states.end){
                                --it;
                                _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
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

                    case TT_CLASS_KW: {
                        if(states.isLastOperand){
                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                "expected operator before 'class'.");
                        }

                        expect_next(*this, states, TT_TEXT);
                        unsigned id = runtime::genOrdinaryId(*_rt, states.it->content);

                        Class* cl = new Class;
                        cl->name = id;
                        cl->boxName = states.currBox->name;
                        states.currBox->objects.insert({id, makeClass(cl)});
                        states.isClassStatement = false;

                        if(is_next(*this, states, TT_ROUND_OPEN)){
                            if(!is_next(*this, states, TT_ROUND_CLOSE)){
                                unsigned nameId = id - runtime::idsStart;
                                _temp.insert(_temp.end(), {
                                    PUSH_REF, bc(nameId >> 8), bc(nameId & 0xFF)
                                });

                                if(states.it->type == TT_TEXT){
                                    unsigned alias = runtime::genOrdinaryId(*_rt,states.it->content) - runtime::idsStart;
                                    if(is_next(*this, states, TT_COLON)){
                                        _classTemp.insert(_classTemp.end(), {
                                            PUSH_INT_0,
                                            DEFINE_GLOBAL_VAR, bc(alias >> 8), bc(alias & 0xFF),
                                            POP
                                        });
                                    } else states.it -= 2;
                                } else --states.it;

                                states.parStack.emplace_back(SUPER_EXPR);
                                states.parStack.back().arg1 = 0;
                                states.parStack.back().classPtr = cl;
                                states.output = &_temp;

                                states.isStatementEmpty = true;
                                states.isLastOperand = false;
                                break;
                            } else ++states.it;
                            // ""fallthrough""
                        } /* not else if */

                        if(states.it->type != TT_CURLY_OPEN){
                            states.isClassStatement = true;
                            --it;
                        }

                        states.currClass = cl;
                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        break;
                    }

                    case TT_VAR_KW: {
                        _declareVar(states, states.currClass ? _classTemp : _temp, true);
                        break;
                    }

                    case TT_SEMICOLON:{
                        if(!states.isStatementEmpty){
                            if(!states.isLastOperand){
                                _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                    "expected operand before ';'.");
                            }

                            (states.currClass ? _classTemp : _temp).push_back(POP);
                        }

                        if(states.currClass && states.isClassStatement){
                            goto CloseClass; // see below
                        }

                        states.output = &_rt->code;
                        states.isStatementEmpty = true;
                        states.isLastOperand = false;
                        states.expectedLvalue = false;
                        break;
                    }

                    case TT_CURLY_CLOSE:{
                        if(states.isLastOperand){
                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                "expected operator before '}'.");
                        }

                        if(states.currClass && !states.isClassStatement){
                            CloseClass:
                            if(!_classTemp.empty()){
                                Function* fn = new Function;
                                fn->address = _rt->code.size();
                                fn->fnName = runtime::initId;
                                fn->boxName = states.currBox->name;

                                // inserting init code into bytecode and linking it to the class
                                states.currClass->objects.insert({runtime::initId, RootObject(makeFunction(fn))});
                                _rt->code.insert(_rt->code.end(), _classTemp.begin(), _classTemp.end());
                                _rt->code.push_back(RETURN_NULL);
                                _classTemp.clear();
                            }

                            states.currClass = nullptr;
                            states.isStatementEmpty = true;
                            states.isLastOperand = false;
                            states.expectedLvalue = false;
                            break;
                        }
                        // fallthrough
                    }

                    default: {
                        _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                            std::string("expected class, function, import, var before ")
                            + representation(*it) + ".");
                    }
                }
            }
        }
    }
}
