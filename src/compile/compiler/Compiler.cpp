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
 *      File compile/compiler/Compiler.cpp
 *
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <tuple>
#include <utility>

#include "compile/Compiler.h"
#include "compile/Statement.h"
#include "parse/Tokenizer.h"
#include "runtime/gc.h"
#include "runtime/id.h"

using namespace sm::parse;
using namespace sm::compile;

namespace sm{
    namespace compile{
        using namespace ParType;

        Compiler::Compiler(runtime::Runtime_t& rt)
            : _rt(&rt), _nfile(0){}

        void Compiler::source(const string_t& filePath){
            size_t lastSep;
            if((lastSep = filePath.find_last_of("/\\")) != std::string::npos){
                paths.push_back(filePath.substr(0, lastSep+1));
                // removes file path and extension
                size_t dot = filePath.find_last_of('.');
                _rt->boxNames.push_back(filePath.substr(lastSep+1, dot != std::string::npos ? dot-lastSep-1 : filePath.size()));
            } else {
                size_t dot = filePath.find_last_of('.');
                _rt->boxNames.push_back(filePath.substr(0, dot != std::string::npos ? dot : filePath.size()));
            }

            #if defined(_SM_OS_WINDOWS)
                std::string newPath(filePath);
                std::replace(newPath.begin(), newPath.end(), '/', '\\');
                error::CodeSource* src = readf(newPath);
                if(src){
                    _rt->sources.newSource(src);
                } else {
                    _rt->sources.msg(error::FATAL_ERROR, std::string("cannot find file '") + newPath + "'.");
                }
            #else
                error::CodeSource* src = readf(filePath);
                if(src){
                    _rt->sources.newSource(src);
                } else {
                    _rt->sources.msg(error::FATAL_ERROR, std::string("cannot find file '") + filePath + "'.");
                }
            #endif
        }

        void Compiler::source(error::CodeSource* source){
            _rt->sources.newSource(source);
        }

        void Compiler::path(const string_t& path){
            if(!path.empty()){
                if(path.back() != fileSeparator){
                    _rt->sources.msg(error::FATAL_ERROR, std::string("path must be followed by '")
                        + fileSeparator + "'.");
                }
                paths.push_back(path);
            }
        }

        bool Compiler::next(){
            std::string text;
            error::CodeSource* src;
            bool reset;

            if(_nfile >= _rt->sources._sources.size()){
                _rt->sources.msg(error::FATAL_ERROR, "no input files.");
                return false;
            }

            if(_rt->boxNames[_nfile].back() == '!')
                return ++_nfile < _rt->sources._sources.size();

            if((reset = !(src = _rt->sources.getSource(_nfile))->code)){
                std::string line;
                std::ifstream file(src->sourceName);

                while(std::getline(file, line)){
                    text += line;
                    text.push_back('\n');
                }
                text.pop_back();
                src->code = &text;
            }

            parse::TokenVec_t tokens = parse::tokenize(_rt, _nfile);
            if(_rt->showAll){
                std::cout << "TokenVec_t tokens (of file '" << _rt->sources.getSource(_nfile)->sourceName << "'):" << std::endl;
                for(const Token& tok : tokens){
                    std::cout << "  " << parse::test::to_string(tok) << std::endl;
                }
            }

            if(!tokens.empty()){
                _compile(tokens);
            }

            if(reset){
                src->code = nullptr;
            }
            return ++_nfile < _rt->sources._sources.size();
        }

        error::CodeSource* Compiler::readf(const string_t& filePath){
            error::CodeSource* src = new error::CodeSource;
            src->sourceName = filePath;
            std::ifstream file(filePath);
            return file ? src : nullptr;
        }

        void Compiler::code(string_t name, string_t* code){
            _rt->boxNames.emplace_back(name);
            error::CodeSource* src = new error::CodeSource;
            src->sourceName = std::move(name);
            src->code = code;
            _rt->sources.newSource(src);
        }

        void Compiler::_ultimateToken(CompilerStates& states){
            switch(states.it->type){
                case TT_INTEGER:
                case TT_FLOAT:
                case TT_STRING:
                case TT_TRUE_KW:
                case TT_FALSE_KW:
                case TT_NULL_KW:
                    if(states.isLastOperand){
                        _rt->sources.msg(error::ERROR, _nfile, states.it->ln, states.it->ch,
                            std::string("expected operator before ")
                                + representation(*states.it) + ".");
                    }
                    states.isLastOperand = true;
                    states.rvalue = true;
                    break;

                case TT_TEXT:
                    if(!states.isLastDot && states.isLastOperand){
                        _rt->sources.msg(error::ERROR, _nfile, states.it->ln, states.it->ch,
                            std::string("expected operator before ")
                                + representation(*states.it) + ".");
                    }
                    states.rvalue = !states.preOperators.empty();
                    states.isLastOperand = true;
                    break;

                case TT_PRE_INC:
                case TT_PRE_DEC:
                    if(!states.preOperators.empty() && states.expectedLvalue)
                        _rt->sources.msg(error::ERROR, _nfile, states.it->ln, states.it->ch,
                            std::string("expected lvalue expression before ")
                                + representation(*states.it) + ".");
                    if(states.isLastOperand){
                        _rt->sources.msg(error::ERROR, _nfile, states.it->ln, states.it->ch,
                            std::string("expected operator before ")
                                + representation(*states.it) + ".");
                    }
                    states.expectedLvalue = true;
                    break;

                case TT_POST_INC:
                case TT_POST_DEC:
                    if(!states.isLastOperand){
                        _rt->sources.msg(error::ERROR, _nfile, states.it->ln, states.it->ch,
                            std::string("expected operand before ")
                                + representation(*states.it) + ".");
                    }
                    if(states.rvalue){
                        _rt->sources.msg(error::ERROR, _nfile, states.it->ln, states.it->ch,
                            std::string("expected lvalue expression before ")
                                + representation(*states.it) + ".");
                    }
                    states.rvalue = true;
                    break;
            }

            if(!states.parStack.empty() && states.it->type != TT_ROUND_CLOSE
                    && states.it->type != TT_SQUARE_CLOSE && states.it->type != TT_CURLY_CLOSE){
                ParInfo_t& backInfo = states.parStack.back();
                if(!backInfo.arg0 && ((backInfo.isRound() && !backInfo.isHead()) || backInfo.isSquare())){
                    backInfo.arg0 = 1;
                }
            }
        }

        void Compiler::_compile(const parse::TokenVec_t& tokens){
            ImportsVec_t toImport;
            CompilerStates states;
            parse::TokenVec_t::const_iterator& it = states.it;

            states.output = &_rt->code;
            states.toImport = &toImport;
            states.begin = tokens.begin();
            states.end = tokens.end();

            states.currBox = new Class;
            states.currBox->boxName = _rt->boxes.size();
            _rt->boxes.push_back(states.currBox);

            it = tokens.begin();

            while(true){
                _ultimateToken(states);
                if(states.parStack.empty()){
                    _globalScopeCompile(states);
                } else {
                    _localScopeCompile(states);

                    if(!states.preOperators.empty()){
                        if(it->type == TT_TEXT){
                            for(ByteCode_t::const_reverse_iterator cit = states.preOperators.rbegin();
                                    cit != states.preOperators.rend(); ++cit){
                                if(*cit == parse::TT_ROUND_OPEN){
                                    break;
                                }

                                states.output->push_back(*cit);
                                states.preOperators.pop_back();
                            }
                            states.expectedLvalue = false;
                        } else if(states.expectedLvalue){
                            switch(it->type){
                                // if curr is a pre operator
                                case TT_PRE_MINUS:
                                case TT_PRE_PLUS:
                                case TT_PRE_INC:
                                case TT_PRE_DEC:
                                case TT_DOT:
                                    break;

                                case TT_ROUND_OPEN:
                                case TT_SQUARE_OPEN:
                                case TT_CURLY_OPEN:
                                    states.preOperators.push_back(parse::TT_ROUND_OPEN);
                                    break;

                                case TT_ROUND_CLOSE:
                                case TT_SQUARE_CLOSE:
                                case TT_CURLY_CLOSE:{
                                    bool found = false;
                                    for(ByteCode_t::const_reverse_iterator cit = states.preOperators.rbegin();
                                            cit != states.preOperators.rend(); ++cit){
                                        if(*cit == parse::TT_ROUND_OPEN){
                                            if(found){
                                                break;
                                            } else {
                                                found = true;
                                                states.preOperators.pop_back();
                                            }
                                        } else {
                                            states.output->push_back(*cit);
                                            states.preOperators.pop_back();
                                        }
                                    }
                                    states.expectedLvalue = false;
                                    break;
                                }

                                case TT_COMPL:
                                case TT_NOT:
                                case TT_INTEGER:
                                case TT_FLOAT:
                                case TT_STRING:
                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                        std::string("expected lvalue expression before ")
                                            + representation(*it) + ".");
                                    break;

                                default:
                                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                                        std::string("expected operand before ")
                                            + representation(*it) + ".");
                                    break;
                            }
                        } else {
                          switch(it->type){
                              // if curr is a pre operator
                              case TT_COMPL:
                              case TT_NOT:
                              case TT_PRE_MINUS:
                              case TT_PRE_PLUS:
                              case TT_PRE_INC:
                              case TT_PRE_DEC:
                              case TT_DOT:
                                  break;

                              case TT_ROUND_OPEN:
                              case TT_CURLY_OPEN:
                              case TT_SQUARE_OPEN:
                                  states.preOperators.push_back(parse::TT_ROUND_OPEN);
                                  break;

                              case TT_ROUND_CLOSE:
                              case TT_SQUARE_CLOSE:
                              case TT_CURLY_CLOSE: {
                                  bool found = false;
                                  for(ByteCode_t::const_reverse_iterator cit = states.preOperators.rbegin();
                                          cit != states.preOperators.rend(); ++cit){
                                      if(*cit == parse::TT_ROUND_OPEN){
                                          if(found){
                                              break;
                                          } else {
                                              found = true;
                                              states.preOperators.pop_back();
                                          }
                                      } else {
                                          states.output->push_back(*cit);
                                          states.preOperators.pop_back();
                                      }
                                  }
                                  states.expectedLvalue = false;
                                  break;
                              }

                              case TT_INTEGER:
                              case TT_FLOAT:
                              case TT_STRING:
                                  for(ByteCode_t::const_reverse_iterator cit = states.preOperators.rbegin();
                                          cit != states.preOperators.rend(); ++cit){
                                      if(*cit == parse::TT_ROUND_OPEN){
                                          break;
                                      }

                                      states.output->push_back(*cit);
                                      states.preOperators.pop_back();
                                  }
                                  states.rvalue = false;
                                  break;

                              default:
                                  break;
                          }

                        }
                    }
                } // end else ( from if(states.parStack.empty()) )

                if(++it == states.end){
                    if(states.parStack.empty()){
                        break;
                    } else {
                        --it;
                        _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                            "expected valid expression before 'eof'.");
                    }
                }
            }

            if(!_temp.empty() || !states.toImport->empty()){
                Function* fn = new Function;
                fn->address = _rt->code.size();
                fn->fnName = runtime::initId;
                fn->boxName = states.currBox->boxName;
                states.currBox->objects.insert(std::make_pair(runtime::initId, makeFunction(fn)));

                for(ImportsVec_t::const_iterator cit = states.toImport->begin();
                        cit != states.toImport->end(); ++cit){
                    _rt->code.push_back(IMPORT);
                    _rt->code.push_back((std::get<0>(*cit) >> 8) & 0xFF);
                    _rt->code.push_back(std::get<0>(*cit) & 0xFF);
                    _rt->code.push_back((std::get<1>(*cit) >> 8) & 0xFF);
                    _rt->code.push_back(std::get<1>(*cit) & 0xFF);
                }

                _rt->code.insert(_rt->code.end(), _temp.begin(), _temp.end()); // inserting <init> code
                _rt->code.push_back(RETURN_NULL);
                _temp.clear();
            }
        }

        void Compiler::_declareVar(CompilerStates& states, bool global){
            parse::TokenVec_t::const_iterator& it = states.it;
            ByteCode_t& out = global ? _temp : *states.output;

            if(states.isLastOperand){
                _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                    "expected operator before 'var'.");
            }

            while(1){
                if(++it == states.end){
                    --it;
                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                        "expected valid expression before 'eof'.");
                } else if(it->type != TT_TEXT){
                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                        "expected identifier after 'var'.");
                }

                unsigned idx = runtime::genOrdinaryId(*_rt,
                        it->content) - runtime::idsStart;

                if(++it == states.end){
                    --it;
                    _rt->sources.msg(error::ERROR, _nfile, it->ln, it->ch,
                        "expected valid expression before 'eof'.");
                } else if(it->type == TT_ASSIGN){
                    states.operators.emplace_back(TT_VAR_KW, it->i);
                    states.parStack.emplace_back(global ? GLOBAL_VAR_DECL
                        : VAR_DECL);

                    states.parStack.back().arg0 = idx;
                    states.output = &out;
                    states.isLastOperand = false;
                    break;
                } else if(it->type == TT_COMMA){
                    out.push_back(global ? DEFINE_GLOBAL_NULL_VAR
                        : DEFINE_NULL_VAR);
                    out.push_back((idx >> 8) & 0xFF);
                    out.push_back(idx & 0xFF);
                    out.push_back(POP);

                    states.isLastOperand = true;
                    continue;
                } else {
                    --it;
                    out.push_back(global ? DEFINE_GLOBAL_NULL_VAR
                        : DEFINE_NULL_VAR);
                    out.push_back((idx >> 8) & 0xFF);
                    out.push_back(idx & 0xFF);

                    states.isLastOperand = true;
                    break;
                }
            }
        }
    }
}
