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
                Object self;
                Function* f_ptr;

                if(!runtime::callable(td.data->func, self, f_ptr)){
                    td.data->intp.rt->sources.printStackTrace(td.data->intp, error::ET_ERROR,
                        std::string("given a not callable object to thread (")
                        + runtime::errorString(td.data->intp, td.data->func) + ")");
                }

                td.data->intp.callFunction(f_ptr, td.data->args, self, false);

                runtime::Runtime_t* rt = td.data->intp.rt;
                exec::TWrapper* wrapper = td.wrapper;
                delete td.data;

                if(wrapper->to_delete.test_and_set()){
                    std::lock_guard<std::mutex> lock(rt->threads_m);
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
                }
            }
        }

        Class* cThread;

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

                        std::lock_guard<std::mutex> lock(intp.rt->threads_m);
                        intp.rt->threads.emplace_back();

                        exec::ThreadData& back = intp.rt->threads.back();
                        back.wrapper = new exec::TWrapper;

                        back.data = new exec::IntpData (
                            *intp.rt,
                            empty ? Object() : Object(args.front()),
                            empty ? ObjectVec_t() : ObjectVec_t(args.begin() +1, args.end())
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

                    smMethod(delete, smLambda {
                        exec::TWrapper* wrapper = smGetData(exec::TWrapper);
                        if(wrapper->to_delete.test_and_set()){
                            std::lock_guard<std::mutex> lock(intp.rt->threads_m);
                            exec::ThreadVec_t& vec = intp.rt->threads;
                            exec::ThreadVec_t::iterator it = std::find_if(vec.begin(), vec.end(),
                                [wrapper](exec::ThreadData& ref){
                                    return ref.wrapper == wrapper;
                                }
                            );
                            if(wrapper->th.joinable())
                                wrapper->th.detach();
                            delete wrapper;
                            vec.erase(it);
                        }
                        return Object();
                    })
                smEnd
            smReturnBox
        }
    }
}
