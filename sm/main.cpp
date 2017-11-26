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
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <thread>

#include "sm/compile/v1/Compiler.h"
#include "sm/compile/Statement.h"
#include "sm/compile/defs.h"

#include "sm/runtime/gc.h"
#include "sm/runtime/Object.h"
#include "sm/runtime/id.h"
#include "sm/runtime/casts.h"

#include "sm/io/RW.h"
#include "sm/io/smc.h"

#include "sm/exec/Interpreter.h"
#include "sm/utils/String.h"
#include "sm/typedefs.h"

#ifndef _SM_WIN_EMBED
    void printUsage() noexcept;
    void printLicense() noexcept;
#endif

constexpr const char* sm_version = _SM_EXECUTABLE_NAME "-" _SM_STR_VERSION
    " (" _SM_DATE_VERSION ")";

using namespace sm;

int main(int argc, char** argv){
    runtime::Runtime_t rt;
    exec::Interpreter intp(rt);
    compile::v1::Compiler cp(rt);

    rt.main_intp = &intp;
    runtime::Runtime_t::main_id = std::this_thread::get_id();

    #ifndef _SM_WIN_EMBED
        std::string mainBox, precompiled;

        int firstArg = argc;
        bool printPaths = false, callInit = true, callNew = true,
            callMain = true, readPrecompiled = false;

        for(int i = 1; i != argc; ++i){
            if(*argv[i] == '-'){
                argv[i]++;
                if(!std::strcmp(argv[i], "c")){
                    rt.compileOnly = true;
                } else if(!std::strcmp(argv[i], "D")){
                    if(++i == argc || *argv[i] == '-'){
                        printUsage();
                        return 0;
                    }
                    cp.path(argv[i]);
                } else if(!std::strcmp(argv[i], "e")){
                    if(++i == argc){
                        printUsage();
                        return 0;
                    }

                    try {
                        rt.stack_printed_elements = std::stoul(argv[i]);
                    } catch(...){
                        printUsage();
                        return 0;
                    }
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
                } else if(!std::strcmp(argv[i], "I")){
                    if(++i == argc || *argv[i] == '-'){
                        printUsage();
                        return 0;
                    }

                    firstArg = i+1;
                    precompiled = argv[i];
                    readPrecompiled = true;
                    break;
                } else if(!std::strcmp(argv[i], "l") || !std::strcmp(argv[i], "-license")){
                    printLicense();
                    return 0;
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
                    return 0;
                } else if(!std::strcmp(argv[i], "wi") || !std::strcmp(argv[i], "-without-init")){
                    callInit = false;
                } else if(!std::strcmp(argv[i], "wn") || !std::strcmp(argv[i], "-without-new")){
                    callNew = false;
                } else if(!std::strcmp(argv[i], "wm") || !std::strcmp(argv[i], "-without-main")){
                    callMain = false;
                } else if(!std::strcmp(argv[i], "x")){
                    if(++i == argc){
                        printUsage();
                        return 0;
                    }

                    try {
                        unsigned long size = std::stoul(argv[i]);
                        if(size){
                            if(size <= rt.max_ss)
                                rt.min_ss = size;
                        }
                    } catch(...){
                        printUsage();
                        return 0;
                    }
                } else if(!std::strcmp(argv[i], "X")){
                    if(++i == argc){
                        printUsage();
                        return 0;
                    }

                    try {
                        unsigned long size = std::stoul(argv[i]);
                        if(size){
                            rt.max_ss = size;
                            if(rt.min_ss > size)
                                rt.min_ss = size / 4;
                        }
                    } catch(...){
                        printUsage();
                        return 0;
                    }
                } else { // includes also -h & --help cases!
                    printUsage();
                    return 0;
                }
            } else {
                mainBox = argv[i];
                cp.source(mainBox);
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
                for(std::vector<std::string>::const_iterator it = rt.paths.begin();
                        it != rt.paths.end(); ++it){
                    std::cout << "\t" << *it << "" << std::endl;
                }
            }
        }
    #endif

    std::atexit(runtime::Runtime_t::exit);

    #ifndef _SM_WIN_EMBED
        if(readPrecompiled){
            sm::Reader<std::ifstream> rd (precompiled, std::ios::in | std::ios::binary);
            if(!rd.stream())
                rt.sources.msg(error::ET_FATAL_ERROR,
                    std::string("failed reading from SMK file '")
                    + precompiled + "'.");
            rd >> rt;
            cp.start();
        } else {
            cp.start();
            while(cp.next()); // compile
            cp.end();
        }

        if(rt.showAll){
            runtime::test::print(rt);
            std::cout << ".. Output:" << std::endl;
        }

        if(rt.compileOnly){
            if(mainBox.size() > 2 && std::equal(mainBox.end() - 3, mainBox.end(), ".sm"))
                mainBox.push_back('k');
            else
                mainBox += ".smk";

            Writer<std::ofstream> wr (mainBox, std::ios_base::out | std::ios_base::binary);
            wr << rt;
            return 0;
        }
    #else
        {
            HRSRC res = FindResource(nullptr, MAKEINTRESOURCE(_SM_WINRES_CODE_ID), RT_RCDATA);
            if(!res){
                rt.sources.msg(error::ET_FATAL_ERROR, "cannot find bytecode resource.");
                std::exit(1);
            }

            HGLOBAL handle = LoadResource(nullptr, res);
            if(!handle){
                rt.sources.msg(error::ET_FATAL_ERROR, "cannot load bytecode resource.");
                std::exit(1);
            }

            char* ch = (char*) LockResource(handle);
            sm::Reader<PtrInputStream<char>> rd (ch);
            rd >> rt;
            FreeResource(handle);
            cp.start();
        }
    #endif

    #ifndef _SM_WIN_EMBED
    if(callInit)
    #endif

    { // call init function of the main box.
        rt.boxes[0]->isInitialized = true;

        RootObjectDict_t::const_iterator it = rt.boxes[0]->objects.find(runtime::initId);
        if(it != rt.boxes[0]->objects.end()){
            Function* initFn;
            RootObject self;
            if(!runtime::callable(it->second, self, initFn)){
                rt.sources.msg(error::ET_ERROR, std::string("'<init>' is not a function in box '")
                    + rt.boxNames[0] + "'.");
            }

            intp.callFunction(initFn, RootObjectVec_t(), self);
        }
    }

    #ifndef _SM_WIN_EMBED
    if(callNew)
    #endif

    { // call new function of the main box.
        RootObjectDict_t::const_iterator it = rt.boxes[0]->objects.find(lib::idNew);
        if(it != rt.boxes[0]->objects.end()){
            Function* newFn;
            RootObject self;
            if(!runtime::callable(it->second, self, newFn)){
                rt.sources.msg(error::ET_ERROR, std::string("'new' is not a function in box '")
                    + rt.boxNames[0] + "'.");
            }

            intp.callFunction(newFn, {}, self);
        }
    }

    int return_value = 0;
    #ifndef _SM_WIN_EMBED
    if(callMain)
    #endif

    { // call main function.
        unsigned id = runtime::genOrdinaryId(rt, "main");
        RootObjectDict_t::const_iterator it = rt.boxes[0]->objects.find(id);
        RootObject args;
        {
            RootObjectVec_t arguments;
            #ifdef _SM_WIN_EMBED
                char** array = argv;
                int n_args = argc;
            #else
                char** array = argv + firstArg;
                int n_args = argc - firstArg;
            #endif

            arguments.reserve(n_args);
            for(int i = 0; i != n_args; ++i){
                arguments.push_back(makeString(array[i]));
            }
            args = makeList(intp, std::move(arguments));
        }

        if(it == rt.boxes[0]->objects.end()){
            rt.sources.msg(error::ET_ERROR, std::string("cannot find 'main()' function in box '")
                + rt.boxNames[0] + "'.");
        }

        Function* mainFn;
        RootObject self;
        if(!runtime::callable(it->second, self, mainFn)){
            rt.sources.msg(error::ET_ERROR, std::string("'main' is not a function in box '")
                + rt.boxNames[0] + "'.");
        }

        RootObject ret = intp.callFunction(mainFn, {args}, self);
        if(ret->type == ObjectType::INTEGER){
            return_value = ret->i;
        }
    }

    while(rt.n_threads.load())
        std::this_thread::yield();

    rt.freeData();
    rt.main_intp = nullptr;
    return return_value;
}

#ifndef _SM_WIN_EMBED
    constexpr const char* license_str =
        #include "sm/license.inc"
    ;

    constexpr const char* usage_str =
        "Usage: " _SM_EXECUTABLE_NAME " options... [File] arguments...\n"
        "Interpret code contained in [File] with arguments and options given.\n"
        "[File] can contain bytecode instead of code (SMK file).\n"
        "Also, you can replace [File] with option -i to use stdin instead.\n\n"

        "Options:\n"
        "  -c, --compile            Output bytecode to file an SMK file.\n"
        "  -D <directory>           Add <directory> to the search paths.\n"
        "  -e <n>                   Display <n> elements when stack is printed.\n"
        "  -h, --help               Display this information.\n"
        "  -i, --stdin              Get code to interpret from stdin.\n"
        "  -I <File>                Read SMK file <File> and execute it.\n"
        "  -l, --license            Display license.\n"
        "  -n, --no-stdlib          Don't use native SSL (except for std.lang)\n"
        "  -s, --show-paths         Display search paths.\n"
        "  -S, --show-all           Show all outputs.\n"
        "  -t, --time               Show total execution time before exiting.\n"
        "  -v, --version            Display version.\n"
        "  -wi, --without-init      Do not run <init> function in main box.\n"
        "  -wm, --without-main      Do not run main function in main box.\n"
        "  -wn, --without-new       Do not run new function in main box.\n"
        "  -x <size>                Set <size> as the stack min size.\n"
        "  -X <size>                Set <size> as the stack max size.\n\n"

        "Program '" _SM_EXECUTABLE_NAME "' is part of the Smudge Programming Language which "
        "is distributed under the Apache License 2.0 (type option '-l' to see the full license).\n"
        "Copyright 2016-2017 Riccardo Musso";


    void printUsage() noexcept{
        std::cout << usage_str << std::endl;
    }

    void printLicense() noexcept{
        std::cout << license_str << std::endl;
    }
#endif
