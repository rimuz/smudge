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
 *      File compile/v2/types.h
 *
*/

#include "typedefs.h"

#define _rule_method(name) virtual unsigned name(root_t& root, \
    statement_t& curr, TokenVec_t::const_iterator& next_it, \
    TokenVec_t::const_iterator& end_it)

namespace sm {
    namespace compile {
        namespace v2 {
            struct statement_t;
            struct root_t;
            
            class rule_t {
                _rule_method(on_comma) {}

                virtual ~rule() {}
            };

            struct statement_t {
                std::vector<statement_t> children;
                statement_t* parent = nullptr;
                rule_t* rule = nullptr;
                unsigned data;
            };

            struct root_t {
                Map_t <std::string, std::vector<statement_t>> boxes;
            };

            /*
             * Creates a new statement_t and inserts it
             * into parent.
            */
            void addStatement(statement_t& parent, );

            /*
             * Quits the program with an error message if
             * the next token is not of the expected type.
             */
            void expect(TokenVec_t::const_iterator next, TokenVec_t::const_iterator end,
                enum_t expected);
        }
    }
}
