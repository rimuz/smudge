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
#include <functional>
#include <thread>

#include "sm/typedefs.h"
#include "sm/runtime/casts.h"
#include "sm/lib/stdlib.h"

namespace sm{
    namespace lib{
        namespace ThreadClass{
            void wrapper_func(exec::ThreadData td){
                RootObject self;
                Function* f_ptr;

                if(!runtime::callable(td.data->func, self, f_ptr)){
                    td.data->intp.rt->sources.printStackTrace(td.data->intp, error::ET_ERROR,
                        std::string("given a not callable object to thread (")
                        + runtime::errorString(td.data->intp, td.data->func) + ")");
                }

                td.data->intp.callFunction(f_ptr, td.data->args, self, false);

                runtime::Runtime_t* rt = td.data->intp.rt;
                exec::TWrapper* wrapper = td.wrapper;
                std::lock_guard<std::mutex> lock(rt->threads_m);
                delete td.data;

                if(wrapper->to_delete.test_and_set()){
                    exec::ThreadVec_t& vec = rt->threads;
                    exec::ThreadVec_t::iterator it = std::find_if(vec.begin(), vec.end(),
                        [wrapper](exec::ThreadData& ref){
                            return ref.wrapper == wrapper;
                        }
                    );
                    if(wrapper->th.joinable())
                        wrapper->th.detach();
                    delete wrapper;
                    vec.erase(it);
                    --rt->n_threads;
                }
            }
        }

        namespace MutexClass{}
        namespace LockClass{}

        Class* cThread;
        Class* cMutex;
        Class* cLock;

        smLibDecl(thread){
            smInitBox
                smFunc(launch, smLambda {
                    return newInstance(intp, cThread, args);
                })

                smFunc(current, smLambda {
                    return newInstance(intp, cThread);
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
                        bool empty = args.empty();

                        ++intp.rt->n_threads;
                        std::lock_guard<std::mutex> lock(intp.rt->threads_m);
                        intp.rt->threads.emplace_back();

                        exec::ThreadData& back = intp.rt->threads.back();
                        back.wrapper = new exec::TWrapper;

                        back.data = new exec::IntpData (
                            *intp.rt,
                            empty ? Object() : Object(args.front().get()),
                            empty ? RootObjectVec_t() : RootObjectVec_t(args.begin() +1, args.end())
                        );

                        smSetData(exec::TWrapper) = back.wrapper;
                        back.wrapper->th = std::thread(wrapper_func, back);
                        return Object();
                    })

                    smMethod(join, smLambda {
                        std::thread& th = smGetData(exec::TWrapper)->th;
                        if(th.joinable() == false || th.get_id() == std::this_thread::get_id())
                            return makeFalse();
                        th.join();
                        return makeTrue();
                    })

                    #define __DESTROY \
                        exec::TWrapper* wrapper = smGetData(exec::TWrapper); \
                        if(wrapper->to_delete.test_and_set()){ \
                            std::lock_guard<std::mutex> lock(intp.rt->threads_m); \
                            exec::ThreadVec_t& vec = intp.rt->threads; \
                            exec::ThreadVec_t::iterator it = std::find_if(vec.begin(), vec.end(), \
                                [wrapper](exec::ThreadData& ref){ \
                                    return ref.wrapper == wrapper; \
                                } \
                            ); \
                            if(wrapper->th.joinable()) \
                                wrapper->th.detach(); \
                            delete wrapper; \
                            vec.erase(it); \
                            --intp.rt->n_threads; \
                        } \

                    smMethod(delete, smLambda {
                        __DESTROY
                        return Object();
                    })

                    smIdMethod(runtime::gcCollectId, smLambda {
                        __DESTROY
                        return Object();
                    })

                    #undef __DESTROY
                smEnd

                smClass(Mutex)
                    /*
                     *
                     *        888b     d888            888
                     *        8888b   d8888            888
                     *        88888b.d88888            888
                     *        888Y88888P888  888  888  888888  .d88b.   888  888
                     *        888 Y888P 888  888  888  888    d8P  Y8b  `Y8bd8P'
                     *        888  Y8P  888  888  888  888    88888888    X88K
                     *        888   "   888  Y88b 888  Y88b.  Y8b.      .d8""8b.
                     *        888       888   "Y88888   "Y888  "Y8888   888  888
                     *
                    */

                    smMethod(new, smLambda {
                        smSetData(std::mutex) = new std::mutex;
                        return Object();
                    })

                    smMethod(delete, smLambda {
                        smDeleteData(std::mutex);
                        return Object();
                    })

                    smMethod(lock, smLambda {
                        smGetData(std::mutex)->lock();
                        return Object();
                    })

                    smMethod(unlock, smLambda {
                        smGetData(std::mutex)->unlock();
                        return Object();
                    })

                    smMethod(try_lock, smLambda {
                        return makeBool(smGetData(std::mutex)->try_lock());
                    })
                smEnd

                smClass(Lock)
                    /*
                     *        888                          888
                     *        888                          888
                     *        888                          888
                     *        888       .d88b.    .d8888b  888  888
                     *        888      d88""88b  d88P"     888 .88P
                     *        888      888  888  888       888888K
                     *        888      Y88..88P  Y88b.     888 "88b
                     *        88888888  "Y88P"    "Y8888P  888  888
                    */

                    smMethod(new, smLambda {
                        Object& ref = (smRef(smId("object")) = args.empty() ? Object() : args.front().get());
                        RootObject selfObj = ref, func;
                        Function* ptr;

                        if(!runtime::find_any(ref, func, smId("lock"))){
                            intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                                std::string("cannot find 'lock' in ")
                                + runtime::errorString(intp, ref));
                        } else if(!runtime::callable(func, selfObj, ptr)){
                            intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                                std::string("'lock' is not a function in ")
                                + runtime::errorString(intp, ref));
                        }
                        intp.callFunction(ptr, {}, selfObj, true);
                        return Object();
                    })

                    smMethod(delete, smLambda {
                        Object& ref = smRef(smId("object"));
                        RootObject selfObj = ref, func;
                        Function* ptr;

                        if(!runtime::find_any(ref, func, smId("unlock"))){
                            intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                                std::string("cannot find 'unlock' in ")
                                + runtime::errorString(intp, ref));
                        } else if(!runtime::callable(func, selfObj, ptr)){
                            intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                                std::string("'unlock' is not a function in ")
                                + runtime::errorString(intp, ref));
                        }
                        intp.callFunction(ptr, {}, selfObj, true);
                        return Object();
                    })
                smEnd
            smReturnBox
        }
    }
}
