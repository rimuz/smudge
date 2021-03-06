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
 *      File lib/io.cpp
 *
*/

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "sm/lib/stdlib.h"
#include "sm/runtime/id.h"
#include "sm/runtime/casts.h"
#include "sm/runtime/Object.h"
#include "sm/osd/File.h"

namespace sm{
    namespace lib {
        enum StreamFlags {
            /*
            * Flags are packed into 5 bits (each letter is a bit): TRWAB
            * T = trunc, R = read, W = write, A = append, B = binary
            */
            TRUNC = 0x10, READ = 0x8, WRITE = 0x4, APPEND = 0x2, BINARY = 0x1,
            RW = 0xC, TA = 0x12
        };

        enum StreamPos {
            BEG, CURR, END,
        };

        namespace FileStreamClass {
            struct FSData {
                std::fstream stream;
                unsigned flags;
            };
        }

        namespace FileClass {}

        Class* cFileStream;
        Class* cFile;

        smNativeFunc(print){
            for(const RootObject& obj : args){
                Object str = runtime::implicitToString(intp, obj);
                std::cout << str.s_ptr->str;
            }
            return makeBox(intp.rt->boxes[thisFn->boxName]);
        }

        smLibDecl(io){
            smInitBox

            smVar(ln, makeString("\n"));
            smVar(RW, makeInteger(RW));
            smVar(READ, makeInteger(READ));
            smVar(WRITE, makeInteger(WRITE));
            smVar(BIN, makeInteger(BINARY));
            smVar(TRUNC, makeInteger(TRUNC));
            smVar(APP, makeInteger(APPEND));
            smVar(BEG, makeInteger(BEG));
            smVar(CURR, makeInteger(CURR));
            smVar(END, makeInteger(END));
            smVar(file_sep, makeString(_SM_FILE_SEPARATOR));
            smVar(path_sep, makeString(_SM_FILE_SEPARATOR));

            smFunc(print, print);
            smOperator(parse::TT_LEFT_SHIFT, print);

            smFunc(println, smLambda {
                for(const RootObject& obj : args){
                    Object str = runtime::implicitToString(intp, obj);
                    std::cout << str.s_ptr->str;
                }
                std::cout << '\n';
                return makeBox(intp.rt->boxes[thisFn->boxName]);
            })

            smFunc(e_print, smLambda {
                for(const RootObject& obj : args){
                    Object str = runtime::implicitToString(intp, obj);
                    std::cerr << str.s_ptr->str;
                }
                return makeBox(intp.rt->boxes[thisFn->boxName]);
            })

            smFunc(e_println, smLambda {
                for(const RootObject& obj : args){
                    Object str = runtime::implicitToString(intp, obj);
                    std::cerr << str.s_ptr->str;
                }
                std::cerr << '\n';
                return makeBox(intp.rt->boxes[thisFn->boxName]);
            })

            smFunc(line, smLambda {
                Object obj = makeString();
                String& line = obj.s_ptr->str;
                int ch;

                while(1) {
                    ch = std::getchar();
                    if(ch == '\n')
                        return obj;
                    else if(ch == EOF)
                        return Object();
                    line.push_back(static_cast<char>(ch));
                }

                return Object();
            })

            smFunc(int, smLambda {
                std::string input;
                std::getline(std::cin, input);
                try {
                    return makeInteger(std::stoll(input));
                } catch(...){
                    return Object();
                }
            })

            smFunc(float, smLambda {
                std::string input;
                std::getline(std::cin, input);
                try {
                    return makeFloat(std::stod(input));
                } catch(...){
                    return Object();
                }
            })

            smFunc(get, smLambda {
                int ch = std::getchar();
                if(ch == EOF)
                    return Object();
                char cch = static_cast<char>(ch);
                return makeString(&cch, &cch +1);
            })

            smFunc(getc, smLambda {
                int ch = std::getchar();
                if(ch == EOF)
                    return makeInteger(-1);
                return makeInteger(ch);
            })

            smFunc(next, smLambda {
                std::string str;
                std::cin >> str;
                return makeString(str.c_str());
            })

            smOperator(parse::TT_RIGHT_SHIFT, smLambda {
                std::string str;
                std::cin >> str;
                const RootObject& obj = args[0];

                if(obj->type == ObjectType::WEAK_REFERENCE){
                    obj->refSet(makeString(str.c_str()));
                }
                return makeBox(intp.rt->boxes[thisFn->boxName]);
            })

            smFunc(open, smLambda {
                RootObject func, objSelf, inst = newInstance(intp, cFileStream);
                Function* f_ptr;

                if(!runtime::find<ObjectType::CLASS_INSTANCE>(inst, func, smId("open")))
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("'open' not found in ")
                        + runtime::errorString(intp, inst));
                else if(!runtime::callable(func, objSelf = inst, f_ptr))
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("'open' is not a function in ")
                        + runtime::errorString(intp, inst));

                RootObject obj = intp.callFunction(f_ptr, args, objSelf, true);
                return runtime::implicitToBool(obj) ? inst : RootObject();
            })

            smFunc(remove, smLambda {
                if(args.empty() || args[0]->type != ObjectType::STRING)
                    return Object();
                std::string filepath(args[0]->s_ptr->str.begin(), args[0]->s_ptr->str.end());
                return makeBool(!std::remove(filepath.c_str()));
            })

            smFunc(rename, smLambda {
                if(args.size() < 2 || args[0]->type != ObjectType::STRING
                        || args[1]->type != ObjectType::STRING)
                    return Object();
                std::string oldName (args[0]->s_ptr->str.begin(), args[0]->s_ptr->str.end());
                std::string newName (args[1]->s_ptr->str.begin(), args[1]->s_ptr->str.end());
                return makeBool(!std::rename(oldName.c_str(), newName.c_str()));
            })

            smClass(File)
                /*
                 *
                 *      8888888888  d8b  888
                 *      888         Y8P  888
                 *      888              888
                 *      8888888     888  888   .d88b.
                 *      888         888  888  d8P  Y8b
                 *      888         888  888  88888888
                 *      888         888  888  Y8b.
                 *      888         888  888   "Y8888
                 *
                */

                smMethod(new, smLambda {
                    std::string str(".");
                    if(!args.empty() && args[0]->type == ObjectType::STRING)
                        str.assign(
                            args[0]->s_ptr->str.begin(),
                            args[0]->s_ptr->str.end()
                        );

                    smSetData(File) = new File(File::path(str));
                    return Object();
                })

                smMethod(delete, smLambda {
                    smDeleteData(File);
                    return Object();
                })

                smIdMethod(runtime::gcCollectId, smLambda{
                    smDeleteData(File);
                    return Object();
                })

                smMethod(reset, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return Object();

                    std::string newPath(
                        args[0]->s_ptr->str.begin(),
                        args[0]->s_ptr->str.end()
                    );
                    smGetData(File)->set(File::path(newPath));
                    return Object();
                })

                #define __BMETHOD(Name) \
                    smMethod(Name, smLambda { \
                        return makeBool(smGetData(File)->Name()); \
                    })

                __BMETHOD(exists)
                __BMETHOD(is_dir)
                __BMETHOD(is_file)
                __BMETHOD(remove)
                __BMETHOD(can_read)
                __BMETHOD(can_write)
                __BMETHOD(can_execute)
                __BMETHOD(make_file)
                __BMETHOD(make_dir)

                #undef __BMETHOD

                smMethod(move, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return Object();

                    std::string newName(
                        args[0]->s_ptr->str.begin(),
                        args[0]->s_ptr->str.end()
                    );

                    return makeBool(smGetData(File)->move(File::path(newName)));
                })

                smMethod(copy, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return Object();

                    std::string newName(
                        args[0]->s_ptr->str.begin(),
                        args[0]->s_ptr->str.end()
                    );

                    return makeBool(smGetData(File)->copy(File::path(newName)));
                })

                smMethod(last_access, smLambda {
                    integer_t value;
                    if(!smGetData(File)->last_access(value))
                        return Object();
                    return makeInteger(value);
                })

                smMethod(last_modified, smLambda {
                    integer_t value;
                    if(!smGetData(File)->last_modified(value))
                        return Object();
                    return makeInteger(value);
                })

                smMethod(is_empty, smLambda {
                    bool value;
                    if(!smGetData(File)->is_empty(value))
                        return Object();
                    return makeInteger(value);
                })

                smMethod(size, smLambda {
                    size_t value;
                    if(!smGetData(File)->size(value))
                        return Object();
                    return makeInteger(value);
                })

                smMethod(list, smLambda {
                    std::string name;
                    Directory dir = smGetData(File)->list_dir(name);

                    if(!dir.is_valid())
                        return makeList(intp);

                    RootObjectVec_t names;
                    do {
                        if(name != "." && name != "..")
                            names.emplace_back(makeString(name.begin(), name.end()));
                    } while(dir.next(name));

                    std::sort(names.begin(), names.end(),
                        [](const RootObject& lhs, const RootObject& rhs) -> bool {
                            return lhs->s_ptr->str < rhs->s_ptr->str;
                        }
                    );

                    return makeList(intp, std::move(names));
                })

                smMethod(perm_recheck, smLambda {
                    smGetData(File)->check_permissions();
                    return Object();
                })

                smMethod(perm_valid, smLambda {
                    return makeBool(smGetData(File)->valid_permissions());
                })

                smMethod(full_path, smLambda {
                    std::string str;
                    return
                        smGetData(File)->full_path(str) ?
                        makeString(str.begin(), str.end()) :
                        Object();
                })

                #define __SMETHOD(Name) \
                    smMethod(Name, smLambda { \
                        std::string str = smGetData(File)->Name(); \
                        return makeString(str.begin(), str.end()); \
                    })

                __SMETHOD(get_parent)
                __SMETHOD(get_name)
                __SMETHOD(path)

                #undef __SMETHOD
            smEnd

            smClass(FileStream)
                /*
                 *
                 *      8888888888  d8b  888             .d8888b.   888
                 *      888         Y8P  888            d88P  Y88b  888
                 *      888              888            Y88b.       888
                 *      8888888     888  888   .d88b.    "Y888b.    888888  888d888  .d88b.    8888b.   88888b.d88b.
                 *      888         888  888  d8P  Y8b      "Y88b.  888     888P"   d8P  Y8b      "88b  888 "888 "88b
                 *      888         888  888  88888888        "888  888     888     88888888  .d888888  888  888  888
                 *      888         888  888  Y8b.      Y88b  d88P  Y88b.   888     Y8b.      888  888  888  888  888
                 *      888         888  888   "Y8888    "Y8888P"    "Y888  888      "Y8888   "Y888888  888  888  888
                 *
                */

                smMethod(new, smLambda {
                    smSetData(FSData) = new FSData();
                    return Object();
                })

                smMethod(delete, smLambda {
                    smDeleteData(FSData);
                    return Object();
                })

                smIdMethod(runtime::gcCollectId, smLambda{
                    smDeleteData(FSData);
                    return Object();
                })

                smMethod(open, smLambda {
                    unsigned flags = READ | WRITE;

                    if(args.empty())
                        return makeFalse();
                    else if(args.size() > 1) {
                        if(args[1]->type != ObjectType::INTEGER)
                            return makeFalse();
                        flags = args[1]->i;
                    }

                    if(args[0]->type != ObjectType::STRING)
                        return makeFalse();

                    std::string filepath (args[0]->s_ptr->str.begin(), args[0]->s_ptr->str.end());
                    #if fileSeparator != '/'
                    std::replace(filepath.begin(), filepath.end(), '/', fileSeparator);
                    #endif
                    std::ios_base::openmode mode;
                    bool to_set = false;

                    if(flags & TRUNC)
                        mode = std::ios::trunc;
                    else if(flags & APPEND)
                        mode = std::ios::app;
                    else
                        to_set = true;

                    if(flags & RW){
                        if(flags & READ){
                            if(to_set){
                                mode = std::ios::in;
                                to_set = false;
                            } else
                                mode |= std::ios::in;
                        }

                        if(flags & WRITE){
                            if(to_set)
                                mode = std::ios::out;
                            else
                                mode |= std::ios::out;
                        }
                    } else {
                        if(to_set)
                            mode = std::ios::in | std::ios::out;
                        else
                            mode |= std::ios::in | std::ios::out;
                    }

                    if(flags & BINARY){
                        mode |= std::ios::binary;
                    }

                    FSData* ptr = smGetData(FSData);

                    ptr->flags = flags;
                    ptr->stream.open(filepath, mode);

                    return makeBool(static_cast<bool>(ptr->stream));
                })

                smMethod(close, smLambda {
                    smGetData(FSData)->stream.close();
                    return Object();
                })

                smMethod(get, smLambda {
                    int ch = smGetData(FSData)->stream.get();
                    if(ch == EOF)
                        return Object();
                    char cch = static_cast<char>(ch);
                    return makeString(&cch, &cch +1);
                })

                smMethod(getc, smLambda {
                    int ch = smGetData(FSData)->stream.get();
                    if(ch == EOF)
                        return makeInteger(-1);
                    return makeInteger(ch);
                })

                smMethod(line, smLambda {
                    FSData* ptr = smGetData(FSData);
                    if(ptr->stream.good()){
                        std::string str;
                        std::getline(ptr->stream, str);
                        return makeString(str.c_str());
                    }
                    return Object();
                })

                smMethod(peek, smLambda {
                    return makeInteger(smGetData(FSData)->stream.peek());
                })

                smMethod(read, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::INTEGER || args[0]->i < 0)
                        return Object();
                    unsigned long sz = args[0]->i;
                    Object str = makeString();
                    str.s_ptr->str.resize(sz);
                    smGetData(FSData)->stream.read(str.s_ptr->str.data(), sz);
                    return str;
                })

                smMethod(read_all, smLambda {
                    Object str = makeString();
                    std::string line;
                    FSData* ptr = smGetData(FSData);

                    while(1){
                        str.s_ptr->str.insert(str.s_ptr->str.end(), line.begin(), line.end());
                        if(!std::getline(ptr->stream, line))
                            break;
                        str.s_ptr->str.push_back('\n');
                    }
                    return str;
                })

                smMethod(write, smLambda {
                    RootObject obj = args.empty() ? RootObject() : args[0];
                    Object str = runtime::implicitToString(intp, obj);
                    smGetData(FSData)->stream.write(str.s_ptr->str.data(), str.s_ptr->str.size());
                    return self;
                })

                smMethod(count, smLambda {
                    return makeInteger(smGetData(FSData)->stream.gcount());
                })

                smMethod(seek, smLambda {
                    FSData* ptr = smGetData(FSData);
                    if(!args.empty() && args[0]->type == ObjectType::INTEGER){
                        if(args.size() == 1 || args[1]->type != ObjectType::INTEGER)
                            ptr->stream.seekg(args[0]->i, ptr->stream.beg);
                        else
                            ptr->stream.seekg(args[0]->i, args[1]->i == BEG
                                ? ptr->stream.beg : (args[1]->i == CURR ?
                                ptr->stream.cur : ptr->stream.end));
                    }
                    return Object();
                })

                smMethod(tell, smLambda {
                    return makeInteger(smGetData(FSData)->stream.tellg());
                })

                smMethod(good, smLambda {
                    return makeInteger(smGetData(FSData)->stream.good());
                })
            smEnd

            smReturnBox
        }
    }
}
