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
#include "lib/stdlib.h"
#include "runtime/casts.h"
#include "runtime/utils.h"

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

        namespace ChunkIteratorClass {
            struct CIData {
                Object chunk;
                unsigned idx;
            };
        }

        smNativeFunc(system){
            if(args.empty() || args[0].type != ObjectType::STRING)
                return Object();
            std::string cmd(args[0].s_ptr->str.begin(), args[0].s_ptr->str.end());
            return makeInteger(std::system(cmd.c_str()));
        }

        smLibDecl(system){
            smInitBox

            smFunc(run, system);
            smIdFunc(runtime::roundId, system);

            smFunc(check, smLambda {
                return makeBool(std::system(nullptr));
            })

            smFunc(exit, smLambda {
                if(args.empty())
                    std::exit(0);
                else if(args[0].type == ObjectType::INTEGER)
                    std::exit(args[0].i);
                else
                    std::exit(runtime::implicitToBool(args[0]));
                return Object();
            })

            smFunc(abort, smLambda {
                std::abort();
            })

            smFunc(get, smLambda {
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return Object();
                std::string name(args[0].s_ptr->str.begin(), args[0].s_ptr->str.end());
                char* str = std::getenv(name.c_str());
                return str ? makeString(str) : Object();
            })

            smFunc(alloc, smLambda {
                Object inst = newInstance(intp, cChunk, false, args);
                return data<ChunkClass::ChunkData>(inst) ? inst : Object();
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
                    if(args.empty() || args[0].type != ObjectType::INTEGER)
                        return Object();

                    size_t size = args[0].i;
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

                smMethod(failed, smLambda {
                    return makeBool(!smGetData(ChunkData));
                })

                smMethod(get, smLambda {
                    if(args.empty() || args[0].type != ObjectType::INTEGER)
                        return Object();
                    integer_t idx = args[0].i;
                    ChunkData* data = smGetData(ChunkData);
                    if(!data || !runtime::findIndex(idx, idx, data->size))
                        return Object();
                    return makeInteger(data->data[idx]);
                })

                smMethod(set, smLambda {
                    if(args.empty() || args[0].type != ObjectType::INTEGER)
                        return Object();
                    integer_t idx = args[0].i;
                    ChunkData* data = smGetData(ChunkData);
                    if(!data || !runtime::findIndex(idx, idx, data->size))
                        return makeFalse();
                    data->data[idx] = args.size() == 1 ? 1 : args[1].i;
                    return makeTrue();
                })

                smMethod(reset, smLambda {
                    if(args.empty() || args[0].type != ObjectType::INTEGER)
                        return Object();
                    integer_t idx = args[0].i;
                    ChunkData* data = smGetData(ChunkData);
                    if(!data || !runtime::findIndex(idx, idx, data->size))
                        return makeFalse();
                    data->data[idx] = 0;
                    return makeTrue();
                })

                smMethod(iterate, smLambda {
                    return newInstance(intp, cChunkIterator, false, {self});
                })

                smMethod(delete, smLambda {
                    UC* ptr = smGetData(UC);
                    if(ptr){
                        std::free(ptr);
                    }
                    return Object();
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

                    smSetData(CIData) = new CIData { args[0], 0 };
                    return Object();
                })

                smMethod(delete, smLambda {
                    delete smGetData(CIData);
                    return Object();
                })

                smMethod(next, smLambda {
                    using namespace ChunkClass;

                    CIData* ptr = smGetData(CIData);
                    ChunkData* chunkPtr = data<ChunkData>(ptr->chunk);
                    if(!chunkPtr)
                        return makeTuple(intp, false, {Object(), makeFalse()});

                    ChunkData& ref = *chunkPtr;
                    bool check = ptr->idx < ref.size;

                    if(check){
                        return makeTuple(intp, false, {makeInteger(ref.data[ptr->idx++]), makeTrue()});
                    }
                    return makeTuple(intp, false, {Object(), makeFalse()});
                })

            smEnd

            smReturnBox
        }
    }
}
