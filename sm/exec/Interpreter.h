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
 *      File exec/Interpreter.h
 *
*/

#ifndef _SM__EXEC__INTERPRETER_H
#define _SM__EXEC__INTERPRETER_H

#include "sm/runtime/Object.h"
#include "sm/runtime/gc.h"

namespace sm{
    namespace exec{
        struct CallInfo_t {
            /*
            * note: for each change in CallInfo_t, GC must be updated.
            */
            ObjectDictVec_t codeBlocks;
            Object thisObject; // 'this' object
            ByteCode_t::const_iterator addr;
            Function* function = nullptr; // for Stack Trace
            Class* box = nullptr;
            /*
             * When you call a native function from Smudge, all works.
             * However, if in that native function you call another Smudge fucntion,
             * we'll mark this call as 'inlined' to tell the interpreter to return
             * when we return in Smudge.
             */
            bool inlined = false;
        };

        using CallStack_t = std::vector<CallInfo_t>;

        class Interpreter {
            friend runtime::GarbageCollector;
        public:
            ObjectVec_t exprStack;
            CallStack_t funcStack;
            runtime::Runtime_t* rt;

            Interpreter(runtime::Runtime_t& _rt) : rt(&_rt) {}
            Interpreter(const Interpreter&) = delete;
            Interpreter(Interpreter&&) = default;

            Object callFunction(Function* fn, const ObjectVec_t& args = ObjectVec_t(),
                const Object& self = Object(), bool inlined = false);
            void makeCall(Function* fn, const ObjectVec_t& args = ObjectVec_t(),
                const Object& self = Object(), bool inlined = false);
            Object start();
        };
    }
}

#endif
