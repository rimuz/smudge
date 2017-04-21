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

#include <iostream>
#include <fstream>
#include <cstdio>

#include "lib/stdlib.h"
#include "runtime/id.h"
#include "runtime/casts.h"
#include "runtime/Object.h"

namespace sm{
    namespace lib {
        enum StreamFlags {
            /*
            * Flags are packed into 5 bits (each letter is a bit): TRWAB
            * T = trunc, R = read, W = write, A = append, B = binary
            */
            TRUNC = 0x10, READ = 0x8, WRITE = 0x4, APPEND = 0x2, BINARY = 0x1,
            RW = 0xC
        };

        enum StreamPos {
            BEG, CURR, END,
        };

        _NativeFunc(print);
        _NativeFunc(println);
        _NativeFunc(e_print);
        _NativeFunc(e_println);
        _NativeFunc(line);
        _NativeFunc(i_int);
        _NativeFunc(i_float);
        _NativeFunc(getChar);
        _NativeFunc(next);
        _NativeFunc(nextOp);
        _NativeFunc(open);

        namespace FileStreamClass {
            class FileStream : public Instance {
            public:
                std::fstream stream;
                unsigned flags;

                FileStream(runtime::GarbageCollector& gc, bool temp, const char* filepath, unsigned _flags);
                _NativeMethod(close, 0);
                _NativeMethod(getc, 0);
                _NativeMethod(line, 0);
                _NativeMethod(peek, 0);
                _NativeMethod(read, 1);
                _NativeMethod(read_all, 0);
                _NativeMethod(write, 1);
                _NativeMethod(count, 0);
                _NativeMethod(seek, 2);
                _NativeMethod(tell, 0);
                _NativeMethod(good, 0);
            };

            _NativeFunc(close);
            _NativeFunc(getc);
            _NativeFunc(line);
            _NativeFunc(peek);
            _NativeFunc(read);
            _NativeFunc(read_all);
            _NativeFunc(write);
            _NativeFunc(count);
            _NativeFunc(skip);
            _NativeFunc(seek);
            _NativeFunc(tell);
            _NativeFunc(good);
        }

        Class* cFileStream;

        _LibDecl(io) {
            Class* box = new Class;
            box->boxName = nBox;

            setNativeFn(rt, box, "print", print);
            setNativeFn(rt, box, "println", println);
            setNativeFn(rt, box, "e_print", e_print);
            setNativeFn(rt, box, "e_println", e_println);
            setNativeFn(rt, box, "line", line);
            setNativeFn(rt, box, "int", i_int);
            setNativeFn(rt, box, "float", i_float);
            setNativeFn(rt, box, "getc", getChar);
            setNativeFn(rt, box, "next", next);

            setNativeFn(rt, box, "open", open);

            setNativeOp(rt, box, parse::TT_LEFT_SHIFT, print);
            setNativeOp(rt, box, parse::TT_RIGHT_SHIFT, nextOp);

            cFileStream = setClass(rt, box, "FileStream", {
                _MethodTuple(FileStreamClass, close),
                _MethodTuple(FileStreamClass, getc),
                _MethodTuple(FileStreamClass, line),
                _MethodTuple(FileStreamClass, peek),
                _MethodTuple(FileStreamClass, read),
                _MethodTuple(FileStreamClass, read_all),
                _MethodTuple(FileStreamClass, write),
                _MethodTuple(FileStreamClass, count),
                _MethodTuple(FileStreamClass, seek),
                _MethodTuple(FileStreamClass, tell),
                _MethodTuple(FileStreamClass, good),
                _OpTuple(FileStreamClass, parse::TT_LEFT_SHIFT, write),
            });

            setVar(rt, box, "ln", makeString("\n"));
            setVar(rt, box, "RW", makeInteger(RW));
            setVar(rt, box, "READ", makeInteger(READ));
            setVar(rt, box, "WRITE", makeInteger(WRITE));
            setVar(rt, box, "BIN", makeInteger(BINARY));
            setVar(rt, box, "TRUNC", makeInteger(TRUNC));
            setVar(rt, box, "APP", makeInteger(APPEND));
            setVar(rt, box, "BEG", makeInteger(BEG));
            setVar(rt, box, "CURR", makeInteger(CURR));
            setVar(rt, box, "END", makeInteger(END));
            return box;
        }

        _NativeFunc(print){
            Object str;
            for(ObjectVec_t::const_iterator it = args.begin(); it != args.end(); ++it){
                runtime::implicitToString(*intp.rt, *it, str);
                std::cout << str.s_ptr->str;
            }
            return makeBox(intp.rt->boxes[thisFn->boxName]);
        }

        _NativeFunc(println){
            Object str;
            for(ObjectVec_t::const_iterator it = args.begin(); it != args.end(); ++it){
                runtime::implicitToString(*intp.rt, *it, str);
                std::cout << str.s_ptr->str;
            }
            std::cout << std::endl;
            return makeBox(intp.rt->boxes[thisFn->boxName]);
        }

        _NativeFunc(e_print){
            Object str;
            for(ObjectVec_t::const_iterator it = args.begin(); it != args.end(); ++it){
                runtime::implicitToString(*intp.rt, *it, str);
                std::cerr << str.s_ptr->str;
            }
            return makeBox(intp.rt->boxes[thisFn->boxName]);
        }

        _NativeFunc(e_println){
            Object str;
            for(ObjectVec_t::const_iterator it = args.begin(); it != args.end(); ++it){
                runtime::implicitToString(*intp.rt, *it, str);
                std::cerr << str.s_ptr->str;
            }
            std::cerr << std::endl;
            return makeBox(intp.rt->boxes[thisFn->boxName]);
        }

        _NativeFunc(line){
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
        }

        _NativeFunc(i_int){
            std::string input;
            std::getline(std::cin, input);
            try {
                Object obj;
                obj.type = ObjectType::INTEGER;
                obj.i = std::stol(input);
                return obj;
            } catch(...){
                return Object();
            }
        }

        _NativeFunc(i_float){
            std::string input;
            std::getline(std::cin, input);
            try {
                Object obj;
                obj.type = ObjectType::FLOAT;
                obj.f = std::stod(input);
                return obj;
            } catch(...){
                return Object();
            }
        }

        _NativeFunc(getChar){
            Object obj;
            obj.type = ObjectType::STRING;
            obj.i = std::getchar();
            return obj;
        }

        _NativeFunc(next){
            std::string str;
            std::cin >> str;
            return makeString(str.c_str());
        }

        _NativeFunc(nextOp){
            std::string str;
            std::cin >> str;
            const Object& obj = args[0];

            if(obj.type == ObjectType::WEAK_REFERENCE){
                obj.refSet(makeString(str.c_str()));
            }
            return makeBox(intp.rt->boxes[thisFn->boxName]);
        }

        _NativeFunc(open){
            unsigned mode = READ | WRITE;
            if(args.empty())
                return Object();
            else if(args.size() > 1) {
                if(args[1].type != ObjectType::INTEGER)
                    return Object();
                mode = static_cast<unsigned>(args[1].i);
            }
            if(args[0].type != ObjectType::STRING && args[0].type != ObjectType::STRING)
                return Object();
            std::string path (args[0].s_ptr->str.begin(), args[0].s_ptr->str.end());
            Object instance = makeFastInstance<FileStreamClass::FileStream>(intp.rt->gc, cFileStream, false, path.c_str(), mode);
            if(!reinterpret_cast<FileStreamClass::FileStream*>(instance.i_ptr)->stream)
                return Object();
            return instance;
        }

        namespace FileStreamClass {
            _BindMethod(FileStream, close, 0);
            _BindMethod(FileStream, getc, 0);
            _BindMethod(FileStream, line, 0);
            _BindMethod(FileStream, peek, 0);
            _BindMethod(FileStream, read, 1);
            _BindMethod(FileStream, read_all, 0);
            _BindMethod(FileStream, write, 1);
            _BindMethod(FileStream, count, 0);
            _BindMethod(FileStream, seek, 2);
            _BindMethod(FileStream, tell, 0);
            _BindMethod(FileStream, good, 0);

            FileStream::FileStream(runtime::GarbageCollector& gc, bool temp, const char* filepath,
                    unsigned _flags) : Instance(gc, temp), flags(_flags){
                std::ios_base::openmode mode;

                if(flags & TRUNC){
                    mode |= std::ios::trunc;
                } else if(flags & APPEND){
                    mode |= std::ios::app;
                }

                if(flags & RW){
                    if(flags & READ){
                        mode |= std::ios_base::in;
                    }
                    if(flags & WRITE){
                        mode |= std::ios::out;
                    }
                } else {
                    mode |= std::ios::in | std::ios::out;
                }

                if(flags & BINARY){
                    mode |= std::ios::binary;
                }

                stream.open(filepath, mode);
            }

            _NativeMethod(FileStream::close, 0){
                stream.close();
                return Object();
            }

            _NativeMethod(FileStream::getc, 0){
                return makeInteger(stream.get());
            }

            _NativeMethod(FileStream::line, 0){
                if(stream.good()){
                    std::string str;
                    std::getline(stream, str);
                    return makeString(str.c_str());
                }
                return Object();
            }

            _NativeMethod(FileStream::peek, 0){
                return makeInteger(stream.peek());
            }

            _NativeMethod(FileStream::read, 1){
                if(args[0].type != ObjectType::INTEGER || args[0].i < 0)
                    return Object();
                unsigned long sz = args[0].i;
                Object str = makeString();
                str.s_ptr->str.resize(sz);
                stream.read(str.s_ptr->str.data(), sz);
                return str;
            }

            _NativeMethod(FileStream::read_all, 0){
                Object str = makeString();
                std::string line;
                while(1){
                    str.s_ptr->str.insert(str.s_ptr->str.end(), line.begin(), line.end());
                    if(!std::getline(stream, line))
                        break;
                    str.s_ptr->str.push_back('\n');
                }
                return str;
            }

            _NativeMethod(FileStream::write, 1){
                Object str;
                runtime::implicitToString(*intp.rt, args[0], str);
                stream.write(str.s_ptr->str.data(), str.s_ptr->str.size());
                return self;
            }

            _NativeMethod(FileStream::count, 0){
                return makeInteger(stream.gcount());
            }

            _NativeMethod(FileStream::seek, 2){
                if(args[0].type == ObjectType::INTEGER){
                    if(args[1].type != ObjectType::INTEGER)
                        stream.seekg(args[0].i, stream.beg);
                    else
                        stream.seekg(args[0].i, args[1].i == BEG ? stream.beg : (args[1].i == CURR ? stream.cur : stream.end));
                }
                return Object();
            }

            _NativeMethod(FileStream::tell, 0){
                return makeInteger(stream.tellg());
            }

            _NativeMethod(FileStream::good, 0){
                return makeInteger(stream.good());
            }
        }
    }
}
