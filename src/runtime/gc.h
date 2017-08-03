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
#include <mutex>
#include <chrono>
#include <list>

#include "typedefs.h"
#include "runtime/Object.h"
#include "error/error.h"
#include "compile/v1/Compiler.h"

namespace sm{
    namespace exec{
        class Interpreter;
        using ThreadVec_t = std::vector<Interpreter*>;
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
            InstanceList_t instances, tempInstances;
            unsigned allocs, threshold;
            bool gcWorking;

            GarbageCollector(Runtime_t* rt) : _rt(rt), allocs(0),
                threshold(garbageCollectorThreshold), gcWorking(false) {};

            Object instance(exec::Interpreter& _intp,
                    Class* _base, bool temp) noexcept;

            GarbageCollector(const GarbageCollector&) = delete;
            GarbageCollector(GarbageCollector&&) = delete;
            GarbageCollector& operator=(const GarbageCollector&) = delete;
            GarbageCollector& operator=(GarbageCollector&&) = delete;

            ~GarbageCollector() = default;
        };

        // one instance per Smudge process.
        class Runtime_t {
        public:
            static std::chrono::steady_clock::time_point* execStart;
            static void exit() noexcept;

            GarbageCollector gc;
            std::mutex globalMutex, threadsDataMutex;

            BoxVec_t boxes;
            Sources sources;
            ByteCode_t code;
            exec::ThreadVec_t threads;

            bool showAll = false, noStd = false;

            std::vector<integer_t>      intConstants;
            std::vector<float_t>        floatConstants;
            std::vector<string_t>       nameConstants;
            std::vector<string_t>       boxNames;
            std::vector<String>         stringConstants;
            compile::NamesMap_t         nameIds;

            Runtime_t() : gc(this) {};

            Runtime_t(const Runtime_t&) = delete;
            Runtime_t(Runtime_t&&) = default;
            Runtime_t& operator=(const Runtime_t&) = delete;
            Runtime_t& operator=(Runtime_t&&) = default;

            string_t nameFromId(unsigned id) const;

            ~Runtime_t();
        };

        namespace test{
            void print(const Runtime_t& rt);
        }
    }
}
#endif
