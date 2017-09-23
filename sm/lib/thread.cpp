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
#include <cmath>

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
        namespace AtomicIntClass{}
        namespace AtomicFloatClass{}
        namespace AtomicFlagClass{}

        Class* cThread;
        Class* cMutex;
        Class* cLock;
        Class* cAtomicInt;
        Class* cAtomicFloat;
        Class* cAtomicFlag;

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

                smClass(AtomicInt)
                    /*
                     *
                     *               d8888   8888888
                     *              d88888     888
                     *             d88P888     888
                     *            d88P 888     888
                     *           d88P  888     888
                     *          d88P   888     888
                     *         d8888888888     888
                     *        d88P     888   8888888
                     *
                    */

                    smMethod(new, smLambda {
                        integer_t value = 0;
                        if(!args.empty()){
                            if(args[0]->type == ObjectType::INTEGER)
                                value = args[0]->i;
                            else if(args[0]->type == ObjectType::FLOAT)
                                value = static_cast<integer_t>(args[0]->f);
                            else if(runtime::of_type(args[0], cAtomicInt))
                                value = data<std::atomic<integer_t>>(args[0])->load();
                            else
                                return Object();
                        }

                        smSetData(std::atomic<integer_t>) = new std::atomic<integer_t>(value);
                        return Object();
                    })

                    smMethod(delete, smLambda {
                        smDeleteData(std::atomic<integer_t>);
                        return Object();
                    })

                    smIdMethod(runtime::gcCollectId, smLambda {
                        smDeleteData(std::atomic<integer_t>);
                        return Object();
                    })

                    smMethod(get, smLambda {
                        return makeInteger(smGetData(std::atomic<integer_t>)->load());
                    })

                    #define __ATOMIC_OP(Name, Op, Default) \
                        smMethod(Name, smLambda { \
                            integer_t value; \
                            if(args.empty()) \
                                value = Default; \
                            else if(args[0]->type == ObjectType::INTEGER) \
                                value = args[0]->i; \
                            else if(args[0]->type == ObjectType::FLOAT) \
                                value = static_cast<integer_t>(args[0]->f); \
                            else if(runtime::of_type(args[0], cAtomicInt)) \
                                value = data<std::atomic<integer_t>>(args[0])->load(); \
                            else \
                                return Object(); \
                            return makeInteger(*smGetData(std::atomic<integer_t>) Op value); \
                        })

                    __ATOMIC_OP(set, =, 1)
                    __ATOMIC_OP(inc, +=, 1)
                    __ATOMIC_OP(dec, -=, 1)
                    __ATOMIC_OP(or_assign, |=, 1)
                    __ATOMIC_OP(and_assign, &=, 1)
                    __ATOMIC_OP(xor_assign, ^=, 1)
                    __ATOMIC_OP(and_assign, -=, 1)
                smEnd

                smClass(AtomicFloat)
                    /*
                     *
                     *              d8888   8888888888
                     *             d88888   888
                     *            d88P888   888
                     *           d88P 888   8888888
                     *          d88P  888   888
                     *         d88P   888   888
                     *        d8888888888   888
                     *       d88P     888   888
                     *
                    */

                    smMethod(new, smLambda {
                        float_t value = 0;
                        if(!args.empty()){
                            if(args[0]->type == ObjectType::INTEGER)
                                value = static_cast<float_t>(args[0]->i);
                            else if(args[0]->type == ObjectType::FLOAT)
                                value = args[0]->f;
                            else if(runtime::of_type(args[0], cAtomicFloat))
                                value = data<std::atomic<float_t>>(args[0])->load();
                            else
                                return Object();
                        }

                        smSetData(std::atomic<float_t>) = new std::atomic<float_t>(value);
                        return Object();
                    })

                    smMethod(delete, smLambda {
                        smDeleteData(std::atomic<float_t>);
                        return Object();
                    })

                    smIdMethod(runtime::gcCollectId, smLambda {
                        smDeleteData(std::atomic<float_t>);
                        return Object();
                    })

                    smMethod(get, smLambda {
                        return makeFloat(smGetData(std::atomic<float_t>)->load());
                    })

                    smMethod(set, smLambda {
                        float_t value;
                        if(args.empty())
                            value = 0.f;
                        else if(args[0]->type == ObjectType::INTEGER)
                            value = args[0]->i;
                        else if(args[0]->type == ObjectType::FLOAT)
                            value = static_cast<integer_t>(args[0]->f);
                        else if(runtime::of_type(args[0], cAtomicFloat))
                            value = data<std::atomic<float_t>>(args[0])->load();
                        else
                            return Object();
                        return makeFloat(smGetData(std::atomic<float_t>)->operator=(value));
                    })
                smEnd

                smClass(AtomicFlag)
                    /*
                     *
                     *             d8888   .d888
                     *            d88888  d88P"
                     *           d88P888  888
                     *          d88P 888  888888  .d88b.
                     *         d88P  888  888    d88P"88b
                     *        d88P   888  888    888  888
                     *       d8888888888  888    Y88b 888
                     *      d88P     888  888     "Y88888
                     *                                888
                     *                           Y8b d88P
                     *                            "Y88P"
                     *
                    */

                    smMethod(new, smLambda {
                        auto* ptr = new std::atomic_flag;
                        ptr->clear();
                        smSetData(std::atomic_flag) = ptr;
                        return Object();
                    })

                    smMethod(delete, smLambda {
                        smDeleteData(std::atomic_flag);
                        return Object();
                    })

                    smIdMethod(runtime::gcCollectId, smLambda {
                        smDeleteData(std::atomic_flag);
                        return Object();
                    })

                    smMethod(clear, smLambda {
                        smGetData(std::atomic_flag)->clear();
                        return Object();
                    })

                    smMethod(test_and_set, smLambda {
                        return makeBool(smGetData(std::atomic_flag)->test_and_set());
                    })
                smEnd

            smReturnBox
        }
    }
}
