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
 *      File lib/system.cpp
 *
*/

#include <cstdio>
#include "sm/lib/stdlib.h"
#include "sm/runtime/casts.h"
#include "sm/runtime/utils.h"

namespace sm {
    namespace lib {

        Class* cChunk;
        Class* cChunkIterator;

        namespace ChunkClass {
            using UC = unsigned char;
            struct ChunkData {
                UC* data;
                size_t size;
            };
        }

        namespace ChunkIteratorClass {}

        smNativeFunc(system){
            if(args.empty() || args[0]->type != ObjectType::STRING)
                return Object();
            std::string cmd(args[0]->s_ptr->str.begin(), args[0]->s_ptr->str.end());
            return makeInteger(std::system(cmd.c_str()));
        }

        smLibDecl(system){
            smInitBox

            smVar(VERSION, makeInteger(_SM_INT_VERSION));
            smVar(STR_VERSION, makeString(_SM_STR_VERSION));
            smVar(DATE_VERSION, makeString(_SM_DATE_VERSION));

            smFunc(run, system);
            smIdFunc(runtime::roundId, system);

            smFunc(check, smLambda {
                return makeBool(std::system(nullptr));
            })

            smFunc(exit, smLambda {
                if(args.empty())
                    std::exit(0);
                else if(args[0]->type == ObjectType::INTEGER)
                    std::exit(args[0]->i);
                else
                    std::exit(runtime::implicitToBool(args[0]));
                return Object();
            })

            smFunc(abort, smLambda {
                std::abort();
                return Object();
            })

            smFunc(sterr, smLambda {
                RootObject obj = args.empty() ? RootObject() : args[0];
                Object str = runtime::implicitToString(intp, obj);
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string(str.s_ptr->str.begin(), str.s_ptr->str.end())
                );
                return Object();
            })

            smFunc(get, smLambda {
                if(args.empty() || args[0]->type != ObjectType::STRING)
                    return Object();
                std::string name(args[0]->s_ptr->str.begin(), args[0]->s_ptr->str.end());
                char* str = std::getenv(name.c_str());
                return str ? makeString(str) : Object();
            })

            smFunc(alloc, smLambda {
                RootObject inst = newInstance(intp, cChunk, args);
                return data<ChunkClass::ChunkData>(inst) ? inst : RootObject();
            })

            smClass(Chunk)

                /*
                 *
                 *           .d8888b.   888                           888
                 *          d88P  Y88b  888                           888
                 *          888    888  888                           888
                 *          888         88888b.   888  888  88888b.   888  888
                 *          888         888 "88b  888  888  888 "88b  888 .88P
                 *          888    888  888  888  888  888  888  888  888888K
                 *          Y88b  d88P  888  888  Y88b 888  888  888  888 "88b
                 *           "Y8888P"   888  888   "Y88888  888  888  888  888
                 *
                */

                smMethod(new, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::INTEGER)
                        return Object();

                    size_t size = args[0]->i;
                    bool zero = true;

                    if(args.size() >= 2)
                        zero = runtime::implicitToBool(args[0]);

                    smSetData(ChunkData) = nullptr;
                    size_t bytes = sizeof(ChunkData) + sizeof(UC) * size;
                    void* allocated = zero ? std::calloc(bytes, 1) : std::malloc(bytes);
                    if(allocated){
                        ChunkData* begin = static_cast<ChunkData*>(allocated);
                        data<ChunkData>(self) = begin;
                        begin->data = reinterpret_cast<UC*>(begin+1);
                        begin->size = size;
                    }
                    return Object();
                })

                smMethod(delete, smLambda {
                    UC* ptr = smGetData(UC);
                    if(ptr){
                        std::free(ptr);
                    }
                    return Object();
                })

                smIdMethod(runtime::gcCollectId, smLambda {
                    UC* ptr = smGetData(UC);
                    if(ptr){
                        std::free(ptr);
                    }
                    return Object();
                })

                smMethod(failed, smLambda {
                    return makeBool(!smGetData(ChunkData));
                })

                smMethod(get, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::INTEGER)
                        return Object();
                    integer_t idx = args[0]->i;
                    ChunkData* data = smGetData(ChunkData);
                    if(!data || !runtime::findIndex(idx, idx, data->size))
                        return Object();
                    return makeInteger(data->data[idx]);
                })

                smMethod(set, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::INTEGER)
                        return Object();
                    integer_t idx = args[0]->i;
                    ChunkData* data = smGetData(ChunkData);
                    if(!data || !runtime::findIndex(idx, idx, data->size))
                        return makeFalse();
                    data->data[idx] = args.size() == 1 ? 1 : args[1]->i;
                    return makeTrue();
                })

                smMethod(reset, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::INTEGER)
                        return Object();
                    integer_t idx = args[0]->i;
                    ChunkData* data = smGetData(ChunkData);
                    if(!data || !runtime::findIndex(idx, idx, data->size))
                        return makeFalse();
                    data->data[idx] = 0;
                    return makeTrue();
                })

                smMethod(iterate, smLambda {
                    return newInstance(intp, cChunkIterator, {self});
                })

            smEnd

            smClass(ChunkIterator)
                /*
                 *
                 *           .d8888b.   888                           888       8888888  888                                888
                 *          d88P  Y88b  888                           888         888    888                                888
                 *          888    888  888                           888         888    888                                888
                 *          888         88888b.   888  888  88888b.   888  888    888    888888  .d88b.   888d888  8888b.   888888  .d88b.   888d888
                 *          888         888 "88b  888  888  888 "88b  888 .88P    888    888    d8P  Y8b  888P"       "88b  888    d88""88b  888P"
                 *          888    888  888  888  888  888  888  888  888888K     888    888    88888888  888     .d888888  888    888  888  888
                 *          Y88b  d88P  888  888  Y88b 888  888  888  888 "88b    888    Y88b.  Y8b.      888     888  888  Y88b.  Y88..88P  888
                 *           "Y8888P"   888  888   "Y88888  888  888  888  888  8888888   "Y888  "Y8888   888     "Y888888   "Y888  "Y88P"   888
                 *
                */

                smMethod(new, smLambda {
                    if(args.empty() || !runtime::of_type(args[0], cChunkIterator))
                        return Object();

                    smRef(smId("chunk")) = args[0];
                    smRef(smId("idx")) = makeInteger(0);
                    return Object();
                })

                smMethod(delete, smLambda {
                    return Object();
                })

                smMethod(next, smLambda {
                    using namespace ChunkClass;

                    Object& chunk = smRef(smId("chunk"));
                    Object& idx = smRef(smId("idx"));

                    if(!runtime::of_type(idx, cChunk)){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                            "'chunk' is not an instance of Chunk in ChunkIterator");
                    }

                    if(idx.type != ObjectType::INTEGER){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                            "'idx' is not an integer in ChunkInteger");
                    }

                    ChunkData* ptr = data<ChunkData>(chunk);
                    if(!ptr)
                        return makeTuple(intp, {Object(), makeFalse()});

                    integer_t i = idx.i;
                    bool check = i < static_cast<integer_t>(ptr->size);

                    if(check){
                        return makeTuple(intp, {makeInteger(ptr->data[i++]), makeTrue()});
                    }
                    return makeTuple(intp, {Object(), makeFalse()});
                })

            smEnd

            smReturnBox
        }
    }
}
