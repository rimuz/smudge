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
 *      File runtime/gc.h
 *
*/

#ifndef _SM__RUNTIME__GC_H
#define _SM__RUNTIME__GC_H

#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <list>

#include "sm/typedefs.h"
#include "sm/runtime/Object.h"
#include "sm/error/error.h"
#include "sm/compile/v1/Compiler.h"

namespace sm{
    namespace exec{
        class Interpreter;
        class IntpData;

        struct TWrapper {
            std::thread th;
            std::atomic_flag to_delete = ATOMIC_FLAG_INIT;
        };

        struct ThreadData {
            TWrapper* wrapper;
            IntpData* data;
        };

        using ThreadVec_t = std::vector<ThreadData>;
    }


    namespace runtime {
        class GarbageCollector {
            friend Runtime_t;
            friend Instance;

        private:
            Runtime_t* _rt;

            void addTemporaryInstance(Instance* ptr);
            void removeTemporaryInstance(Instance* ptr);
            void addInstance(Instance* ptr);
            void removeInstance(Instance* ptr);
            void collect();

        public:
            InstanceList_t instances;
            std::mutex instances_m;
            std::atomic_uint allocs;
            unsigned threshold;
            bool gcWorking;

            GarbageCollector(Runtime_t* rt) : _rt(rt), allocs(0),
                threshold(garbageCollectorThreshold), gcWorking(false) {};

            Object makeTempInstance(exec::Interpreter& _intp,
                    Class* _base) noexcept;

            GarbageCollector(const GarbageCollector&) = delete;
            GarbageCollector(GarbageCollector&&) = delete;
            GarbageCollector& operator=(const GarbageCollector&) = delete;
            GarbageCollector& operator=(GarbageCollector&&) = delete;

            ~GarbageCollector() = default;
        };

        // one per Smudge process.
        class Runtime_t {
        public:
            static std::chrono::steady_clock::time_point* execStart;
            static void exit() noexcept;

            using LibHandle_t =
                #ifdef _SM_OS_WINDOWS
                    HANDLE
                #else
                    void*
                #endif
            ;

            GarbageCollector gc;

            BoxVec_t boxes;
            Sources sources;
            ByteCode_t code;

            std::mutex threads_m;
            exec::ThreadVec_t threads;

            exec::Interpreter* main_intp = nullptr;
            static std::thread::id main_id;
            std::atomic_uint n_threads; // main thread is not counted

            exec::Interpreter* getCurrentThread() noexcept;

            compile::NamesMap_t         nameIds;
            std::vector<string_t>       nameConstants;
            std::vector<string_t>       boxNames;
            std::vector<String>         stringConstants;

            std::vector<integer_t>      intConstants;
            std::vector<float_t>        floatConstants;

            std::vector<string_t>       paths; // each path must be followed by slash ('/')

            bool showAll = false, callInit = false,
                    callMain = false, callNew = false,
                    noStd = false, compileOnly = false;

            Runtime_t() : gc(this), n_threads(0) {};

            Runtime_t(const Runtime_t&) = delete;
            Runtime_t(Runtime_t&&) = default;
            Runtime_t& operator=(const Runtime_t&) = delete;
            Runtime_t& operator=(Runtime_t&&) = default;

            string_t nameFromId(unsigned id) const;

            static std::vector<LibHandle_t> sharedLibs;
            static void freeLibraries() noexcept;
            void freeData();

            ~Runtime_t();
        };

        namespace test{
            void print(const Runtime_t& rt);
        }
    }
}
#endif
