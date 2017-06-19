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
 *      File parse/Tokenizer.h
 *
*/

#ifndef _SM__PARSE__TOKENIZER_HPP
#define _SM__PARSE__TOKENIZER_HPP

#include <vector>

#include "typedefs.h"
#include "parse/Token.h"
#include "runtime/Object.h"

namespace sm{
    namespace parse{
        /*
         * creates a TokenVec_t { aka std::vector<sm::parse::Token> } from
         * a string_t { aka std::string } that represents the code.
         *
         * The code have to be saved as CodeSource in sources (see ns/error/error.h).
         * Below is a simple example of the use of tokenize(unsigned):
         *
         *      sm::CodeSource source = {
         *              "<Code here>", "./path/to/file.ns"
         *      };
         *
         *      auto id = sm::newSource(source);
         *      auto res = sm::parse::tokenize(id);
         *
         * Now, res (of type sm::parse::TokenVec_t) contains
         * all the tokens needed for the compilation (and then,
         * the execution) of the code.
         *
         */
        TokenVec_t tokenize(runtime::Runtime_t* rt, unsigned source) noexcept;

        /*
         * the following functions checks if the
         * character ('ascii') belongs to some
         * token types.
        */
        bool isAsciiValidLetter(ascii_t ascii) noexcept;
        bool isAsciiValidDigit(ascii_t ascii) noexcept;
        bool isAsciiValidOperator(ascii_t ascii) noexcept;
    }
}

#endif // SM__PARSE__TOKENIZER_HPP
