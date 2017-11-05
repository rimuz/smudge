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
 *      File compile/Statement.cpp
 *
*/

#include <iostream>
#include <iomanip>
#include <string>
#include <array>

#include "sm/typedefs.h"
#include "sm/compile/Statement.h"

namespace sm{
    namespace compile {
        namespace test{
            constexpr const char** opCodes[] = {
                (const char* []) {
                    "NOP", "POP", "IS_NULL", "ADD", "SUB", "MUL", "DIV", "MOD",
                    "OR", "AND", "XOR", "LEFT_SHIFT", "RIGHT_SHIFT",
                    "EQUAL", "NOT_EQUAL", "GREATER", "GREATER_OR_EQUAL", "LESS",
                    "LESS_OR_EQUAL", "ASSIGN", "ASSIGN_ADD", "ASSIGN_SUB", "ASSIGN_MUL",
                    "ASSIGN_DIV", "ASSIGN_MOD", "ASSIGN_OR", "ASSIGN_AND", "ASSIGN_XOR",
                    "ASSIGN_LEFT_SHIFT", "ASSIGN_RIGHT_SHIFT", "MAKE_VOID_LIST",
                    "MAKE_VOID_TUPLE", "MAKE_REF", "COMPL", "NOT", "UNARY_PLUS",
                    "UNARY_MINUS", "INC", "DEC", "POST_INC", "POST_DEC", "START_BLOCK",
                    "END_BLOCK", "THROW_EXCEPTION", "RETURN", "RETURN_NULL",
                    "PUSH_INT_0", "PUSH_INT_1", "PUSH_NULL", "PUSH_THIS", "PUSH_BOX",
                    "PUSH_CLASS", "ITERATE", "IT_NEXT", "MAKE_SUPER", "DUP",
                    "DUP1"
                },

                nullptr,

                (const char* []) {
                    "END_BLOCKS", "PUSH_INTEGER", "PUSH_FLOAT", "PUSH_STRING",
                    "PUSH_INT_VALUE", "PUSH_REF", "JUMP_F", "JUMP_B", "JUMP_IF_F",
                    "JUMP_IF_B", "JUMP_IF_NOT_F", "JUMP_IF_NOT_B", "LOGIC_AND",
                    "LOGIC_OR", "ELVIS", "TRY", "CATCH", "FINALLY", "CALL_FUNCTION",
                    "PERFORM_BRACING", "DEFINE_VAR", "DEFINE_GLOBAL_VAR", "DEFINE_NULL_VAR",
                    "DEFINE_GLOBAL_NULL_VAR", "ASSIGN_NULL_POP", "FIND", "FIND_SUPER",
                    "MAKE_LIST", "MAKE_TUPLE", "FOREACH_CHECK", "SWITCH_CASE"
                },

                nullptr,

                (const char* []) {
                    "IMPORT"
                }
            };

            void print(const ByteCode_t& code){
                byte_t byte;
                unsigned skip = 0;
                std::cout << std::hex;

                for(size_t i = 0; i != code.size(); ++i){
                    byte = code[i];
                    if(skip){
                        std::cout << " " << std::setfill('0') << std::setw(2)
                            << static_cast<unsigned>(byte);
                        if(!--skip){
                            std::cout << std::endl;
                        }
                    } else {
                        skip = byte & 0x40 ? (byte & 0x80 ? 4 : 2) : 0;
                        std::cout << "  " << opCodes[skip][byte & 0x3F];
                        if(!skip){
                            std::cout << std::endl;
                        }
                    }
                }

                std::cout << std::dec;
            }

            std::string instRepr(std::array<uint8_t, 5> inst) noexcept{
                unsigned skip = inst[0] & 0x40 ? (inst[0] & 0x80 ? 4 : 2) : 0;
                return std::string(opCodes[skip][inst[0] & 0x3F]);
            }
        }
    }
}
