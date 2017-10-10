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
 *      File compile/v1/compiler/Compiler.cpp
 *
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <tuple>
#include <utility>

#include "sm/compile/v1/Compiler.h"
#include "sm/compile/Statement.h"
#include "sm/parse/Tokenizer.h"

#include "sm/runtime/gc.h"
#include "sm/runtime/id.h"
#include "sm/lib/stdlib.h"

using namespace sm::parse;
using namespace sm::compile;

#ifdef _SM_OS_WINDOWS
// TODO
#else
#include <dlfcn.h>
#endif

namespace sm{
    namespace lib {
        oid_t idNew, idDelete, idToString, idHash, idIterate, idNext;
    }

    namespace compile{
        namespace v1 {
            using namespace ParType;

            Compiler::Compiler(runtime::Runtime_t& rt)
                : _rt(&rt), _nfile(0){}

            void Compiler::source(string_t filePath){
                // replaces slashes with OS-dependent fileSeparator character
                #if fileSeparator != '/'
                std::replace(filePath.begin(), filePath.end(), '/', fileSeparator);
                #endif

                size_t sep = filePath.find_last_of(fileSeparator);
                if(sep == std::string::npos){
                    sep = 0;
                    _rt->paths.emplace_back("." _SM_FILE_SEPARATOR);
                } else {
                    _rt->paths.emplace_back(filePath.begin(), filePath.begin()+sep+1);
                    ++sep;
                }

                std::string fileName(filePath.begin()+sep, filePath.end());

                // if the given path is a directory, looks for a file main.sm inside it.
                if(fileName.empty()){
                    filePath += (fileName = "main.sm");
                }

                size_t dot = fileName.find_last_of('.');
                if(dot != std::string::npos && fileName.size() > 2
                        && !std::strcmp(fileName.c_str()+dot, ".sm")){
                    _rt->boxNames.emplace_back(fileName.begin(), fileName.begin()+dot);
                } else {
                    _rt->boxNames.emplace_back(fileName);
                }

                error::CodeSource* src = readf(filePath);
                if(!src)
                    _rt->sources.msg(error::ET_FATAL_ERROR,
                        std::string("cannot open file '") + filePath + "'.");
                _rt->sources.newSource(src);
                _rt->boxes.emplace_back(nullptr);
            }

            void Compiler::source(string_t name, error::CodeSource* source){
                _rt->sources.newSource(source);
                _rt->boxNames.emplace_back(std::move(name));
                _rt->boxes.emplace_back(nullptr);
            }

            void Compiler::path(const string_t& path){
                if(!path.empty()){
                    if(path.back() != fileSeparator)
                        _rt->paths.emplace_back(path + fileSeparator);
                    else
                        _rt->paths.emplace_back(path);
                }
            }

            bool Compiler::next(){
                std::string text;
                error::CodeSource* src;
                bool reset;

                if(_nfile >= _rt->sources._sources.size()){
                    _rt->sources.msg(error::ET_FATAL_ERROR, "no input files.");
                    return false;
                }

                if(_rt->boxNames[_nfile].back() == '!')
                    return ++_nfile < _rt->sources._sources.size();

                src = _rt->sources.getSource(_nfile);
                if((reset = !src->code)){
                    std::string line;
                    std::ifstream file(src->sourceName);

                    if(!file.is_open())
                        _rt->sources.msg(error::ET_FATAL_ERROR,
                            std::string("cannot open file '") + src->sourceName + "'.");

                    while(std::getline(file, line)){
                        text += line;
                        text.push_back('\n');
                    }

                    if(file.bad()){
                        _rt->sources.msg(error::ET_FATAL_ERROR,
                            std::string("unable to read file '")
                            + src->sourceName + "'.");
                    }

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
                std::ifstream file(filePath);
                if(file){
                    error::CodeSource* src = new error::CodeSource;
                    src->sourceName = filePath;
                    return src;
                }
                return nullptr;
            }

            void Compiler::start(){
                // setting OIDs
                lib::idNew = runtime::genOrdinaryId(*_rt, "new");
                lib::idDelete = runtime::genOrdinaryId(*_rt, "delete");
                lib::idToString = runtime::genOrdinaryId(*_rt, "to_string");
                lib::idHash = runtime::genOrdinaryId(*_rt, "hash");
                lib::idIterate = runtime::genOrdinaryId(*_rt, "iterate");
                lib::idNext = runtime::genOrdinaryId(*_rt, "next");
            }

            void Compiler::end(){
                // import box std.lang
                if(std::find(_rt->boxNames.begin(), _rt->boxNames.end(), "std.lang!") == _rt->boxNames.end()){
                    _rt->boxNames.emplace_back("std.lang!");
                    _rt->boxes.push_back(lib::import_lang(*_rt, _rt->boxNames.size()-1));
                }
            }

            void Compiler::code(string_t name, string_t* code){
                error::CodeSource* src = new error::CodeSource;
                src->sourceName = std::move(name);
                src->code = code;

                _rt->boxNames.emplace_back(name);
                _rt->boxes.emplace_back(nullptr);
                _rt->sources.newSource(src);
            }

            void Compiler::_ultimateToken(CompilerStates& states){
                switch(states.it->type){
                    case TT_INTEGER:        case TT_FLOAT:
                    case TT_STRING:         case TT_TRUE_KW:
                    case TT_FALSE_KW:       case TT_NULL_KW:
                        if(states.isLastOperand){
                            _rt->sources.msg(error::ET_ERROR, _nfile, states.it->ln, states.it->ch,
                                std::string("expected operator before ")
                                    + representation(*states.it) + ".");
                        }
                        states.isLastOperand = true;
                        break;

                    case TT_TEXT:
                        if(!states.isLastDot && states.isLastOperand){
                            _rt->sources.msg(error::ET_ERROR, _nfile, states.it->ln, states.it->ch,
                                std::string("expected operator before ")
                                    + representation(*states.it) + ".");
                        }
                        states.isLastOperand = true;
                        break;

                    case TT_PRE_INC:
                    case TT_PRE_DEC:
                        if(states.isLastOperand){
                            _rt->sources.msg(error::ET_ERROR, _nfile, states.it->ln, states.it->ch,
                                std::string("expected operator before ")
                                    + representation(*states.it) + ".");
                        }
                        break;

                    case TT_POST_INC:
                    case TT_POST_DEC:
                        if(!states.isLastOperand){
                            _rt->sources.msg(error::ET_ERROR, _nfile, states.it->ln, states.it->ch,
                                std::string("expected operand before ")
                                    + representation(*states.it) + ".");
                        }
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

                states.currBox = new Box;
                states.currBox->name = _nfile;
                _rt->boxes[_nfile] = states.currBox;

                it = tokens.begin();

                while(true){
                    _ultimateToken(states);
                    if(states.parStack.empty()){
                        _globalScopeCompile(states);
                    } else {
                        _localScopeCompile(states);
                    }

                    if(++it == states.end){
                        if(states.parStack.empty()){
                            break;
                        } else {
                            --it;
                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                "expected valid expression before 'eof'.");
                        }
                    }
                }

                if(!_temp.empty() || !states.toImport->empty()){
                    Function* fn = new Function;
                    fn->address = _rt->code.size();
                    fn->fnName = runtime::initId;
                    fn->boxName = states.currBox->name;
                    states.currBox->objects.insert({runtime::initId, makeFunction(fn)});

                    for(ImportsVec_t::const_iterator cit = states.toImport->begin();
                            cit != states.toImport->end(); ++cit){
                        _rt->code.insert(_rt->code.end(), {
                            IMPORT, bc(std::get<0>(*cit) >> 8), bc(std::get<0>(*cit) & 0xFF),
                                    bc(std::get<1>(*cit) >> 8), bc(std::get<1>(*cit) & 0xFF)
                        });
                    }

                    _rt->code.insert(_rt->code.end(), _temp.begin(), _temp.end()); // inserting <init> code
                    _rt->code.push_back(RETURN_NULL);
                    _temp.clear();
                }
            }

            void Compiler::_declareVar(CompilerStates& states, ByteCode_t& out, bool global){
                parse::TokenVec_t::const_iterator& it = states.it;

                if(states.isLastOperand){
                    _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                        "expected operator before 'var'.");
                }

                bool empty = states.wasStatementEmpty;

                while(1){
                    expect_next(*this, states, TT_TEXT);
                    unsigned idx = runtime::genOrdinaryId(*_rt,
                            it->content) - runtime::idsStart;

                    if(is_next(*this, states, TT_ASSIGN)){
                        states.operators.emplace_back(TT_VAR_KW, it->i);
                        states.parStack.emplace_back(global ? GLOBAL_VAR_DECL
                            : VAR_DECL);

                        states.parStack.back().arg0 = idx;
                        states.output = &out;
                        states.isLastOperand = false;
                        break;
                    } else if(it->type == TT_COMMA){
                        if(!empty){
                            _rt->sources.msg(error::ET_ERROR, _nfile, it->ln, it->ch,
                                "multiple var definition is not allowed in bigger expressions.");
                        }

                        out.insert(out.end(), {
                            bc(global ? DEFINE_GLOBAL_NULL_VAR : DEFINE_NULL_VAR),
                            bc(idx >> 8), bc(idx & 0xFF), POP
                        });
                        states.isLastOperand = true;
                        continue;
                    } else {
                        --it;
                        out.insert(out.end(), {
                            bc(global ? DEFINE_GLOBAL_NULL_VAR : DEFINE_NULL_VAR),
                            bc(idx >> 8), bc(idx & 0xFF)
                        });
                        states.isLastOperand = true;
                        break;
                    }
                }
            }

            bool Compiler::load_native(const char* path, runtime::Runtime_t& rt, unsigned id, Box*& box) noexcept{
                #ifdef _SM_OS_WINDOWS
                    HMODULE library = LoadLibrary(path);
                    if(!library)
                        return false;
                    rt.sharedLibs.emplace_back(library);

                    FARPROC func = GetProcAddress(library, "import_library");
                    if(!func){
                        box = nullptr;
                        return true;
                    }

                    box = reinterpret_cast<lib::DynInitFunc_t>(func)(rt, id);
                    return true;
                #else
                    void* library = dlopen(path, RTLD_LAZY);
                    if(!library)
                        return false;
                    rt.sharedLibs.emplace_back(library);

                    void* func = dlsym(library, "import_library");
                    if(!func){
                        box = nullptr;
                        return true;
                    }

                    box = reinterpret_cast<lib::DynInitFunc_t>(func)(rt, id);
                    return true;
                #endif
            }

            void expect_next(Compiler& cp, CompilerStates& states,
                    enum_t expected) noexcept{
                if(++states.it == states.end)
                    cp._rt->sources.msg(error::ET_ERROR, cp._nfile, states.it->ln, states.it->ch,
                        std::string("expected ") + parse::representations[expected]
                        + " before 'eof'.");
                else if(states.it->type != expected)
                    cp._rt->sources.msg(error::ET_ERROR, cp._nfile, states.it->ln, states.it->ch,
                        std::string("expected ") + parse::representations[expected]
                        + " before " + representation(*states.it));
            }

            bool is_next(Compiler& cp, CompilerStates& states,
                    enum_t expected) noexcept{
                if(++states.it == states.end)
                    cp._rt->sources.msg(error::ET_ERROR, cp._nfile, states.it->ln, states.it->ch,
                        std::string("expected ") + parse::representations[expected]
                        + " before 'eof'.");
                return states.it->type == expected;
            }
        }
    }
}
