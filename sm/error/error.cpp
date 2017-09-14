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
 *      File error/error.cpp
 *
*/

#include <istream>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstdlib>

#include "sm/error/error.h"
#include "sm/runtime/gc.h"
#include "sm/exec/Interpreter.h"

namespace sm{
    unsigned Sources::newSource(error::CodeSource* src) noexcept {
        _sources.push_back(src);
        return _sources.size() -1;
    }

    error::CodeSource* Sources::getSource(unsigned id) noexcept {
        return _sources[id];
    }

    void Sources::msg(const std::string& msg) noexcept{
        std::cerr << msg << std::endl;
    }

    void Sources::msg(enum_t errType, unsigned source, unsigned ln, unsigned ch, const std::string& msg) noexcept{
        unsigned line_n = 0;
        bool found = false;
        std::istream* stream;
        std::string* code = _sources[source]->code;
        std::string line;
        std::string spaces;

        if(code){
            stream = new std::istringstream(*code);
        } else {
            stream = new std::ifstream(_sources[source]->sourceName);
        }

        while(std::getline(*stream, line)){
            // ln starts from 1, not 0
            line_n++;
            if(line_n == ln){
                found = true;
                // ch starts from 1, not 0
                for(unsigned i = 1; i != ch && i != line.size(); i++){
                    if(line[i-1] == '\t'){
                        spaces.push_back('\t');
                    } else {
                        spaces.push_back(' ');
                    }
                }
                break;
            }
        }

        if(found){
            std::cerr << _sources[source]->sourceName << ":" << ln << ":" << ch << ": " << errorMessages[errType]
                << msg << std::endl << line << std::endl << spaces << "^" << std::endl;
        } else {
            std::cerr << _sources[source]->sourceName << ":?:?: " << errorMessages[errType]
                << msg << std::endl;
        }

        delete stream;

        if(errType == error::ET_FATAL_ERROR || errType == error::ET_SYNTAX_ERROR || errType == error::ET_ERROR
                || errType == error::ET_BUG){
            std::exit(1);
        }
    }

    void Sources::msg(enum_t errType, const std::string& msg) noexcept{
        std::cerr << errorMessages[errType] << msg << std::endl;

        if(errType == error::ET_FATAL_ERROR || errType == error::ET_SYNTAX_ERROR || errType == error::ET_ERROR
                || errType == error::ET_BUG){
            std::exit(1);
        }
    }

    void Sources::printStackTrace(exec::Interpreter& intp, enum_t errType, const std::string& msg) noexcept{
        std::cerr << errorMessages[errType] << msg << std::endl;

        std::lock_guard<std::mutex> lock(intp.stacks_m);
        for(exec::CallStack_t::const_reverse_iterator it = intp.funcStack.rbegin();
                it != intp.funcStack.rend(); ++it){
            std::cerr << "\tat ";
            std::cerr << intp.rt->boxNames[it->box->boxName] << "::";

            if(it->thisObject.type == ObjectType::CLASS_INSTANCE && it->thisObject.i_ptr->base)
                std::cerr << intp.rt->nameFromId(it->thisObject.i_ptr->base->name) << "::";
            std::cerr << intp.rt->nameFromId(it->function->fnName) << "()" << std::endl;

            if(it->inlined){
                std::cerr <<  "\tat <native>(...)" << std::endl;
            }
        }

        if(errType == error::ET_FATAL_ERROR || errType == error::ET_SYNTAX_ERROR || errType == error::ET_ERROR
                || errType == error::ET_BUG){
            std::exit(1);
        }
    }
}
