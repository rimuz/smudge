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

#include "typedefs.h"
#include "require_cpp11.h"
#include "parse/Token.h"

namespace sm{
    namespace compile{
        enum OpCode{
            /*
             * tos = Top Of the Stack
             * TOS = the value/s on the Top Of the Stack
            */

            // *** STATEMENTS WITH LENGTH = 1 BYTE:

            // do nothing
            NOP,

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
             * call new() if it exists (otherwise do nothing) without saving the
             * return value in the stack. This opcode is used only inside the
             * init function of each box.
            */
            CALL_NEW_FUNCTION,

            /*
             * push true/false/null values
             * into the stack.
            */
            PUSH_INT_0,
            PUSH_INT_1,
            PUSH_NULL,

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
            END_BLOCKS,

            /*
             * push values on the tos.
             * The values are taken from
             * a vector of constant values
             * (with index=param)
            */
            PUSH_INTEGER, PUSH_FLOAT, PUSH_STRING,

            /*
             * find a variable named 'names[param]'
             * and pushes its reference on the tos.
            */
            PUSH_REF,

            /*
             * find a global variable named 'names[param]'
             * from 'this' and push it on the tos.
            */
            PUSH_THIS,

            /*
             * find a variable named 'names[param]' from 'super'
             * and push it on the tos
            */
            PUSH_SUPER,

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
             * search a name into
             * the TOS. If found it's
             * added on the tos, otherwise
             * an exception is throwed.
            */
            FIND,

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
             * make a dictionary (hash table) with
             * 'param' size, and TOS, TOS2, TOS4, ...,
             * TOS'param-2' as keys and TOS1, TOS3, ...,
             * TOS'param-1' as values.
            */
            MAKE_DICT,


            /*
             * Compare TOS and TOS1 (then pop TOS, only),
             * and if they're not equals jump to curr + param,
            */
            SWITCH_CASE,

            MAX_OPCODE_3BYTE = SWITCH_CASE,

            // **** STATEMENTS WITH LENGTH = 5 Bytes (Opcode of 1 byte + param0 (2 bytes) + param1 (2 bytes))

            /*
            * imports 'param0' box with name 'param1'
            */
            IMPORT,

            INVALID_OPCODE,

            ASSIGN_START = ASSIGN_ADD,
            OPERATORS_START = ADD,
        };

        static_assert (INVALID_OPCODE <= 0x100, // not 0xff, because INVALID_OPCODE is equal to max +1
            "statement opcodes mustn't be longer than 1 Byte.");
    }
}

#endif // _SM__COMPILE__STATEMENT_H
