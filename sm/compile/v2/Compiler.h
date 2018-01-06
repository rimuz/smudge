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
 *      File compile/v1/Compiler.h
 *
*/

#ifndef _SM__COMPILE__COMPILER_H
#define _SM__COMPILE__COMPILER_H

#include <vector>
#include <string>
#include <unordered_map>

#include "sm/error/error.h"
#include "sm/compile/Statement.h"
#include "sm/compile/defs.h"
#include "sm/runtime/Object.h"
#include "sm/typedefs.h"

namespace sm {
    namespace parse {
        struct Token;
    }
}

namespace compile {
    namespace v2 {
        class Label {
        public:
            unsigned id;
            explicit Label(unsigned val) : id(val) {}
        };

        class Jump {
        public:
            Label where;
            enum Condition : uint8_t {
                ALWAYS,
                IF_TRUE,
                IF_FALSE,
                IF_NULL
            } condition;

            explicit Jump(Label l, Condition c) : where(l), condition(c);
        };

        class Instr {
        public:
            enum Type : uint8_t {
                OPERATION,
                JUMP,
                MEASURE
            } type;

            union {
                struct {
                    Jump jump;
                };
                struct {
                    uint8_t opcode;
                    uint16_t arg0;
                    uint16_t arg1;
                };
            };
        };

        class CompilerOutput {
            std::vector<Instr> vec;

            CompilerOutput() = default;

            CompilerOutput(CompilerOutput&&) = default;
            CompilerOutput(const CompilerOutput&) = default;
            CompilerOutput& operator= (const CompilerOutput&) = default;
            CompilerOutput& operator= (CompilerOutput&&) = default;

            CompilerOutput& operator<< (Label) noexcept;
            CompilerOutput& operator<< (Jump) noexcept;
            CompilerOutput& operator<< (uint8_t) noexcept; // normal instruction

            ~CompilerOutput() = default;
        };

        class Compiler {
            private:
                CompilerOutput _output;
                runtime::Runtime_t& _rt;
                unsigned _nbox;

                void compile_box(unsigned nbox);
            public:
                Compiler(runtime::Runtime_t& rt);

                Compiler(const Compiler&) = default;
                Compiler(Compiler&&) = default;
                Compiler& operator=(const Compiler&) = default;
                Compiler& operator=(Compiler&&) = default;

                void source(string_t filePath);
                void source(string_t name, error::CodeSource* source);
                void code(string_t name, string_t* code);

                void path(const string_t& path);
                void start();
                void end();
                bool next();

                ~Compiler() = default;

                friend void expect_next(Compiler& cp, CompilerStates& states,
                    enum_t expected, const char* custom = nullptr) noexcept;
                friend bool is_next(Compiler& cp, CompilerStates& states,
                    enum_t expected, const char* custom = nullptr) noexcept;

                    static bool load_native(const char* path, runtime::Runtime_t& rt,
                    unsigned id, Box*& box) noexcept;
                static error::CodeSource* readf(const string_t& filePath);
        };
    }
}

#endif
