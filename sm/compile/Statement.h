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
 *      File compile/Statement.h
 *
*/

#ifndef _SM__COMPILE__STATEMENT_H
#define _SM__COMPILE__STATEMENT_H

#include <vector>
#include <unordered_map>
#include <array>

#include "sm/typedefs.h"
#include "sm/parse/Token.h"

namespace sm{
    namespace compile{
        namespace test{
            void print(const ByteCode_t& code);
            std::string instRepr(std::array<uint8_t, 5> inst) noexcept;
        }

        enum OpCode{
            /*
             * tos = Top Of the Stack
             * TOS = the value/s on the Top Of the Stack
            */

            // *** STATEMENTS WITH LENGTH = 1 BYTE:

            // do nothing
            NOP = 0,

            // pop the TOS
            POP,

            /*
             * pops TOS and pushes true is TOS was null, false
             * otherwise.
            */
            IS_NULL,

            // TOS = TOS1 operator TOS (deleting TOS1)
            ADD, SUB, MUL, DIV, MOD, OR, AND, XOR,
            LEFT_SHIFT, RIGHT_SHIFT, EQUAL, NOT_EQUAL,
            GREATER, GREATER_OR_EQUAL, LESS,
            LESS_OR_EQUAL,

            ASSIGN,

            ASSIGN_ADD, ASSIGN_SUB, ASSIGN_MUL,
            ASSIGN_DIV, ASSIGN_MOD, ASSIGN_OR,
            ASSIGN_AND, ASSIGN_XOR, ASSIGN_LEFT_SHIFT,
            ASSIGN_RIGHT_SHIFT,

            MAKE_VOID_LIST,
            MAKE_VOID_TUPLE,
            MAKE_REF,

            // TOS = operator TOS
            COMPL, NOT, UNARY_PLUS, UNARY_MINUS,

            // ++TOS
            INC,

            // --TOS
            DEC,

            // TOS = TOS++
            POST_INC,

            // TOS = TOS--
            POST_DEC,

            /*
             * open a new scope, which can
             * have local objects.
            */
            START_BLOCK,

            /*
             * close the current scope,
             * deleting its local objects.
            */
            END_BLOCK,

            // throw TOS
            THROW_EXCEPTION,

            // return TOS
            RETURN,

            /*
             * return NULL (always
             * added at the end of
             * a function.)
            */
            RETURN_NULL,

            /*
             * push true/false/null values
             * into the stack.
            */
            PUSH_INT_0,
            PUSH_INT_1,
            PUSH_NULL,

            /*
             * Push THIS value on the top of the stack.
            */
            PUSH_THIS,

            /*
             * Push BOX value on the top of the stack.
            */
            PUSH_BOX,

            /*
             * Push CLASS value on the top of the stack.
            */
            PUSH_CLASS,

            /*
             * Set TOS = TOS.iterate().
            */
            ITERATE,

            /*
             * Push TOS.next() on the stack.
            */
            IT_NEXT,

            /*
             * Given TOS1 and TOS classes, sets TOS
             * as super class of TOS1 and pops TOS.
            */
            MAKE_SUPER,

            /*
             * pushes a copy of TOS on the stack.
            */
            DUP,

            /*
             * pushes a copy of TOS1 before TOS.
            */
            DUP1,

            MAX_OPCODE_1BYTE = DUP1,

            // *** STATEMENTS WITH LENGTH = 3 BYTE (1 + 2, opcode + param):

            /*
             * param = a 2-byte unsigned integer following the opcode
            */

            /*
            * close 'param' blocks.
            */
            END_BLOCKS = 0x40,

            /*
             * push values on the tos.
             * The values are taken from
             * a vector of constant values
             * (with index=param)
            */
            PUSH_INTEGER, PUSH_FLOAT, PUSH_STRING,

            /*
             * Push an integer with value = 'param'
             * (only for positive ints smaller than
             * 65536 excluded)
            */
            PUSH_INT_VALUE,

            /*
             * find a variable named 'names[param]'
             * and pushes its reference on the tos.
            */
            PUSH_REF,

            /*
             * SOURCE_N and SOURCE_LN are usually used for
             * the error message, but they can compromize a
             * lot the speed of the program.
             *
             * So, when evaluating code they are mandatories, but
             * when compiling optimized code they shouldn't be
             * used too much.
             *
            */

            // go to curr + 'param' byte of code
            JUMP_F,

            // go to curr - 'param' byte of code
            JUMP_B,

            // jump if TOS evaluates to true
            JUMP_IF_F,
            JUMP_IF_B,

            // jump if TOS evaluates to false
            JUMP_IF_NOT_F,
            JUMP_IF_NOT_B,

            /*
             * if TOS evaluates to 'false' jump
             * to curr + 'param', otherwise pop TOS.
            */
            LOGIC_AND,


            /*
             * if TOS evaluates to 'true' jump
             * to curr + 'param', otherwise pop TOS.
            */
            LOGIC_OR,


            /*
             * If TOS is not null, jump to curr + 'param',
             * otherwise pop TOS.
            */
            ELVIS,

            /*
             * start a try-catch-finally block
             * 'param' contains how many bytes
             * you have to skip to reach 'catch'
            */
            TRY,
            /*
             * catch in the try-catch-finally block
             * started by TRY, 'param' contains how
             * many bytes you have to skip to
             * reach 'finally'
            */
            CATCH,
            /*
             * finally in the try-catch-finally block
             * started by TRY, 'param' contains how
             * many bytes is finally-block long
             * plus one.
            */
            FINALLY,

            /*
             * note: try-catch-finally block
             * implicitly opens and closes one
             * scope for each keyword like
             * START_BLOCK and CLOSE_BLOCK
            */

            /*
             * does TOS-'param'(TOS-'param-1',
             *   TOS-'param-2', ... TOS)
             * with arguments param-values below
             * TOS.
             * At the end, it removes TOS and
             * the arguments from the Stack;
             * the return value is finally
             * added on the tos.
            */
            CALL_FUNCTION,

            /*
             * do TOS-'param'[TOS-'param-1',
             *   TOS-'param-2', ... TOS]
             * working as CALL_FUNCTION
            */
            PERFORM_BRACING,

            /*
             * create a variable with name
             * defined by 'param' and valued
             * TOS or null.
            */
            DEFINE_VAR,
            DEFINE_GLOBAL_VAR,
            DEFINE_NULL_VAR,
            DEFINE_GLOBAL_NULL_VAR,

            /*
             * assign 'null' to variable named
             * 'param', without adding anything
             * to the stack.
            */
            ASSIGN_NULL_POP,

            /*
             * search a name into
             * the TOS. If found, it's
             * added on the tos, otherwise
             * an exception is throwed.
            */
            FIND,

            /*
             * finds function 'param' in super n. 'TOS'
             * and pushes a reference (see Method in runtime/Object.h)
             * of it.
            */
            FIND_SUPER,

            /*
             * make a list with 'param' size,
             * and TOS, TOS1, TOS2, .., TOS'param-1'
             * values.
            */
            MAKE_LIST,

            /*
             * make a tuple (immutable list)
             * with 'param' size, and TOS, TOS1,
             * TOS2, .., TOS'param-1' values.
            */
            MAKE_TUPLE,

            /*
             * Checks if foreach is out of range
             * (with the value 1 of the tuple returned
             * by TOS.next()).
             *
             * If not out of range, sets TOS2 = TOS[0].
             * (sets the foreach temp value to the value 0
             * of the tuple) It also pops TOS.
             *
             * If out of range, pops TOS2-TOS out from the stack
             * and jumps to curr + 'param'
             *
            */
            FOREACH_CHECK,

            /*
             * Compare TOS and TOS1 (then pop TOS, only),
             * and if they're not equals jump to curr + param,
            */
            SWITCH_CASE,

            MAX_OPCODE_3BYTE = SWITCH_CASE,

            // **** STATEMENTS WITH LENGTH = 5 Bytes (Opcode of 1 byte + param0 (2 bytes) + param1 (2 bytes))

            /*
            * imports 'param0' box with name 'param1':
            * if the box is unitialized, '<init>()' and 'new()' are called.
            */
            IMPORT = 0xC0,

            INVALID_OPCODE,

            ASSIGN_START = ASSIGN_ADD,
            OPERATORS_START = ADD,
        };

        static_assert (MAX_OPCODE_1BYTE < 0x40, "too many statements of size 1");
        static_assert (MAX_OPCODE_3BYTE >= 0x40 && MAX_OPCODE_3BYTE < 0x80, "too many statements of size 3");
        static_assert (INVALID_OPCODE <= 0x100, // not 0xff, because INVALID_OPCODE is equal to max +1
            "statement opcodes mustn't be longer than 1 Byte.");
    }
}

#endif // _SM__COMPILE__STATEMENT_H
