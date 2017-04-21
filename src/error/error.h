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
 *      File error/error.h
 *
*/

#ifndef _SM__ERROR__ERROR_H
#define _SM__ERROR__ERROR_H

#include <vector>
#include <string>

#include "require_cpp11.h"
#include "typedefs.h"

namespace sm {
    namespace error{
        // usually a CodeSource is a file (or simplier, a string) where is contained the code
        struct CodeSource {
            std::string* code = nullptr;
            string_t sourceName;
        };

        enum ErrorType {
            NOTE, WARNING, ERROR, SYNTAX_ERROR, FATAL_ERROR, BUG
        };
    }

    namespace compile{
        class Compiler;
    }

    namespace exec{
        class Interpreter;
    }

    class Sources {
        friend compile::Compiler;
    private:
        std::vector<error::CodeSource*> _sources;
    public:
        unsigned newSource(error::CodeSource* src) noexcept;
        error::CodeSource* getSource(unsigned id) noexcept;

        void msg(const std::string& msg) noexcept;
        void msg(enum_t errType, unsigned source, unsigned ln, unsigned ch, const std::string& msg) noexcept;
        void msg(enum_t errType, const std::string& msg) noexcept;

        void printStackTrace(const exec::Interpreter& intp, enum_t errType, const std::string& msg) noexcept;
    };

    constexpr const char* errorMessages[]{
        "note: ", "warning: ", "error: ", "syntax error: ", "fatal error: ", "bug: "
    };
}

#endif // _SM__ERROR__ERROR_H
