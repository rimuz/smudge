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
            RootObjectDictVec_t codeBlocks;
            RootObject thisObject; // 'this' object
            Function* function = nullptr; // for Stack Trace
            Box* box = nullptr;
            unsigned pc;
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
            RootObjectVec_t exprStack;
            CallStack_t funcStack;
            // funcStack is shared between GC and Interpreter
            runtime::Runtime_t* rt;
            unsigned pc;
            bool doReturn;

            explicit Interpreter(runtime::Runtime_t& _rt) : rt(&_rt), pc(0), doReturn(false) {}
            Interpreter(const Interpreter&) = delete;
            Interpreter(Interpreter&&) = default;

            Interpreter& operator=(const Interpreter&) = delete;
            Interpreter& operator=(Interpreter&&) = default;

            RootObject callFunction(Function* fn, const RootObjectVec_t& args = RootObjectVec_t(),
                const RootObject& self = RootObject(), bool inlined = false);
            void makeCall(Function* fn, const RootObjectVec_t& args = RootObjectVec_t(),
                const RootObject& self = RootObject(), bool inlined = false);
            RootObject start();
            void printStackContent();

            inline std::array<uint8_t, 5> fetch(unsigned& addr){
                std::array<uint8_t, 5> ret {};
                uint8_t opcode = ret[0] = rt->code[addr++];
                if(opcode & 0x40){
                    ret[1] = rt->code[addr++];
                    ret[2] = rt->code[addr++];
                    if(opcode & 0x80){
                        ret[3] = rt->code[addr++];
                        ret[4] = rt->code[addr++];
                    }
                }
                return ret;
            }

            ~Interpreter();
        };

        class IntpData {
        public:
            exec::Interpreter intp;
            RootObject func;
            RootObjectVec_t args;

            IntpData(runtime::Runtime_t& rt, RootObject _func, RootObjectVec_t _args)
                : intp(rt), func(std::move(_func)), args(std::move(_args)) {}
        };
    }
}

#endif
