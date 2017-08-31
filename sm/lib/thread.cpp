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
 *      File lib/thread.cpp
 *
*/

#include <algorithm>
#include "sm/typedefs.h"
#include "sm/utils/Thread.h"
#include "sm/runtime/casts.h"
#include "sm/lib/stdlib.h"

namespace sm{
    namespace lib{
        namespace ThreadClass{
            struct TData {
                Instance* ptr;
                runtime::Runtime_t& rt;
            };

            uint32_t run(TData& data){
                exec::Interpreter intp(data.rt);
                Object self;
                Function* f_ptr;

                {
                    Lock lock(data.rt.threads_m);
                    data.rt.threads.emplace_back(&intp);
                }

                Object func = data.ptr->objects[smId("func")];
                if(!runtime::callable(func, self, f_ptr)){
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("given a not callable object to thread (")
                        + runtime::errorString(intp, func) + ")");
                }

                Object vecArgs = data.ptr->objects[smId("args")];
                ObjectVec_t* vec;
                hasVector(vecArgs, vec);

                intp.callFunction(f_ptr, *vec, self, false);
                {
                    Lock lock(data.rt.threads_m);
                    exec::ThreadVec_t& vec = data.rt.threads;
                    vec.erase(std::remove(vec.begin(), vec.end(), &intp), vec.end());
                }
                return 0;
            }
        }

        Class* cThread;

        smLibDecl(thread){
            smInitBox
                smFunc(launch, smLambda {
                    return newInstance(intp, cThread, false, args);
                })

                smFunc(current, smLambda {
                    return newInstance(intp, cThread, false);
                })

                smClass(Thread)
                    /*
                     *
                     *     88888888888  888                                        888
                     *         888      888                                        888
                     *         888      888                                        888
                     *         888      88888b.   888d888  .d88b.    8888b.    .d88888
                     *         888      888 "88b  888P"   d8P  Y8b      "88b  d88" 888
                     *         888      888  888  888     88888888  .d888888  888  888
                     *         888      888  888  888     Y8b.      888  888  Y88b 888
                     *         888      888  888  888      "Y8888   "Y888888   "Y88888
                     *
                    */
                    smMethod(new, smLambda {
                        Object& objFunc = smRef(smId("func"));
                        Object& objArgs = smRef(smId("args"));

                        ObjectVec_t* argsVec;
                        objArgs = makeList(intp, false);
                        hasVector(objArgs, argsVec);
                        bool empty = args.empty();

                        if(empty || args.front().type == ObjectType::NONE){
                            smSetData(Thread<TData>) = new Thread<TData>(Thread<TData>::current());
                            return Object();
                        } else {
                            objFunc = args.front();
                            *argsVec = ObjectVec_t(args.begin()+1, args.end());
                        }

                        smSetData(Thread<TData>) = new Thread<TData> (
                            run, TData{self.i_ptr, *intp.rt}
                        );
                        smGetData(Thread<TData>)->start();
                        return Object();
                    })

                    #define smSimpleBind(Name) \
                        smMethod(Name, smLambda { \
                            return makeBool(smGetData(Thread<TData>)->Name()); \
                        })

                    smSimpleBind(kill)
                    smSimpleBind(is_running)
                    #undef smSimpleBind

                    smMethod(join, smLambda {
                        uint32_t ret = 0;
                        return makeTuple(intp, false, ObjectVec_t {
                            makeBool(smGetData(Thread<TData>)->join(ret)),
                            makeInteger(ret)
                        });
                    })

                    smMethod(delete, smLambda {
                        smDeleteData(Thread<TData>);
                        return Object();
                    })
                smEnd
            smReturnBox
        }
    }
}
