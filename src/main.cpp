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
 *
 *
 *       .d8888b.                              888
 *      d88P  Y88b                             888
 *      Y88b.                                  888
 *       "Y888b.   88888b.d88b.  888  888  .d88888  .d88b.   .d88b.
 *          "Y88b. 888 "888 "88b 888  888 d88" 888 d88P"88b d8P  Y8b
 *            "888 888  888  888 888  888 888  888 888  888 88888888
 *      Y88b  d88P 888  888  888 Y88b 888 Y88b 888 Y88b 888 Y8b.
 *       "Y8888P"  888  888  888  "Y88888  "Y88888  "Y88888  "Y8888
 *                                                      888
 *                                                 Y8b d88P
 *                                                  "Y88P"
 *
 *
 *      ┌──────────────────────────────────┐
 *      │          Smudge Project          │
 *      ├──────────────────────────────────┤
 *      │    developed by Riccardo Musso   │
 *      │  since 23th of September 2016    │
 *      └──────────────────────────────────┘
 *
 * File main.cpp
 *
*/

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <chrono>

#include "compile/Compiler.h"
#include "compile/Statement.h"
#include "runtime/gc.h"
#include "runtime/Object.h"
#include "runtime/id.h"
#include "runtime/casts.h"
#include "exec/Interpreter.h"
#include "utils/String.h"
#include "typedefs.h"
#include "lib/stdlib.h"

void printUsage() noexcept;
void printLicense() noexcept;

constexpr const char* sm_version = _SM_EXECUTABLE_NAME "-" _SM_STR_VERSION " (" _SM_DATE_VERSION ")";

using namespace sm;

int main(int argc, char** argv){
    runtime::Runtime_t rt;
    compile::Compiler cp(rt);
    exec::Interpreter intp(rt);
    int firstArg = argc;
    bool printPaths = false;

    for(int i = 1; i != argc; ++i){
        if(*argv[i] == '-'){
            argv[i]++;
            if(!std::strcmp(argv[i], "D")){
                if(++i == argc || *argv[i] == '-'){
                    printUsage();
                }
                cp.path(argv[i]);
            } else if(!std::strcmp(argv[i], "i") || !std::strcmp(argv[i], "-stdin")){
                std::string* code = new std::string;
                std::string line;
                while(std::getline(std::cin, line)){
                    (*code) += line;
                    code->push_back('\n');
                }
                cp.code("<stdin>", code);
                firstArg = i+1;
                break;
            } else if(!std::strcmp(argv[i], "l") || !std::strcmp(argv[i], "-license")){
                printLicense();
            } else if(!std::strcmp(argv[i], "n") || !std::strcmp(argv[i], "-no-stdlib")){
                rt.noStd = true;
            } else if(!std::strcmp(argv[i], "s") || !std::strcmp(argv[i], "-show-paths")){
                printPaths = true;
            } else if(!std::strcmp(argv[i], "S") || !std::strcmp(argv[i], "-show-all")){
                rt.showAll = true;
            } else if(!std::strcmp(argv[i], "t") || !std::strcmp(argv[i], "-time")){
                rt.execStart = new std::chrono::steady_clock::time_point(std::chrono::steady_clock::now());
            } else if(!std::strcmp(argv[i], "v") || !std::strcmp(argv[i], "-version")){
                std::cout << sm_version << std::endl;
                runtime::Runtime_t::exit(0);
            } else { // includes also -h & --help cases!
                printUsage();
            }
        } else {
            cp.source(argv[i]);
            firstArg = i+1;
            break;
        }
    }

    {
        String envPaths(std::getenv("SMUDGE_PATH"));
        std::vector<String> paths = envPaths.split(";:");

        for(std::vector<String>::iterator it = paths.begin();
                it != paths.end(); ++it){
            if(!it->empty()){
                if(!it->endsWith(_SM_FILE_SEPARATOR)){
                    it->append(_SM_FILE_SEPARATOR);
                }

                cp.path(std::string(it->ptr(), it->ptr() + it->size()));
            }
        }

        for(size_t i = 0; i != searchPathsLen; ++i){
            cp.path(searchPaths[i]);
        }

        if(printPaths){
            std::cout << "Search paths:" << std::endl;
            for(std::vector<std::string>::const_iterator it = cp.paths.begin();
                    it != cp.paths.end(); ++it){
                std::cout << "\t" << *it << "" << std::endl;
            }
        }
    }

    while(cp.next()); // compile

    // import std.lang box
    rt.boxes.push_back(lib::import_lang(rt, rt.boxNames.size()-1));

    if(rt.showAll){
        runtime::test::print(rt);
        std::cout << ".. Output:" << std::endl;
    }

    { // call init function of the main box.
        ObjectDict_t::const_iterator it = rt.boxes[0]->objects.find(runtime::initId);
        if(it != rt.boxes[0]->objects.end()){
            Function* initFn;
            Object self;
            if(!runtime::callable(it->second, self, initFn)){
                rt.sources.msg(error::ERROR, std::string("'<init>' is not a function in box '")
                    + rt.boxNames[0] + "'.");
            }

            intp.callFunction(initFn, ObjectVec_t(), self);
        }
    }

    { // call new function of the main box.
        ObjectDict_t::const_iterator it = rt.boxes[0]->objects.find(runtime::newId);
        if(it != rt.boxes[0]->objects.end()){
            Function* newFn;
            Object self;
            if(!runtime::callable(it->second, self, newFn)){
                rt.sources.msg(error::ERROR, std::string("'new' is not a function in box '")
                    + rt.boxNames[0] + "'.");
            }

            intp.callFunction(newFn, ObjectVec_t(), self);
        }
    }

    { // call main function.
        unsigned id = runtime::genOrdinaryId(rt, "main");
        ObjectDict_t::const_iterator it = rt.boxes[0]->objects.find(id);
        Object args;
        {
            ObjectVec_t arguments;
            char** array = argv + firstArg;
            int n_args = argc - firstArg;

            arguments.reserve(n_args);
            for(int i = 0; i != n_args; ++i){
                arguments.push_back(makeString(array[i]));
            }
            args = makeList(rt.gc, false, std::move(arguments));
        }

        if(it == rt.boxes[0]->objects.end()){
            rt.sources.msg(error::ERROR, std::string("cannot find 'main()' function in box '")
                + rt.boxNames[0] + "'.");
        }

        Function* mainFn;
        Object self;
        if(!runtime::callable(it->second, self, mainFn)){
            rt.sources.msg(error::ERROR, std::string("'main' is not a function in box '")
                + rt.boxNames[0] + "'.");
        }

        Object ret = intp.callFunction(mainFn, {args}, self);

        if(ret.type == ObjectType::INTEGER){
            runtime::Runtime_t::exit(ret.i);
        }
    }

    runtime::Runtime_t::exit(0);
    return 0;
}

constexpr const char* license_str =
    #include "license.inc"
;

constexpr const char* usage_str =
    "Usage: " _SM_EXECUTABLE_NAME " options... <File> arguments...\n"
    "Interpret code contained in <File> with arguments and options given.\n"
    "Also, you can replace <File> with option -i to use stdin instead.\n\n"

    "Options:\n"
    "  -D <directory>           Add <directory> to the search paths.\n"
    "  -h, --help               Display this information.\n"
    "  -i, --stdin              Get code to interpret from stdin.\n"
    "  -l, --license            Display license.\n"
    "  -n, --no-stdlib          Don't import Native Standard Library.\n"
    "  -s, --show-paths         Display search paths.\n"
    "  -S, --show-all           Show all outputs.\n"
    "  -t, --time               Show total execution time before exiting.\n"
    "  -v, --version            Display version.\n\n"

    "Program '" _SM_EXECUTABLE_NAME "' is part of the Smudge Programming Language which "
    "is distributed under the Apache License 2.0 (type option '-l' to see the full license).\n"
    "Copyright 2016-2017 Riccardo Musso";

void printUsage() noexcept{
    std::cout << usage_str << std::endl;
    runtime::Runtime_t::exit(0);
}

void printLicense() noexcept{
    std::cout << license_str << std::endl;
    runtime::Runtime_t::exit(0);
}
