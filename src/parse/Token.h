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
 *      File parse/Token.h
 *
*/

#ifndef _SM__PARSE__TOKEN_H
#define _SM__PARSE__TOKEN_H

#include <list>

#include "typedefs.h"

namespace sm{
    namespace runtime{
        class Runtime_t;
    }

    namespace parse{
        enum TokenType {
            // null (not parsed yet)
            TT_UNPARSED,
            /* a space (with 'space' I mean a character less or equal than ' '), or a group of near spaces
            * Tokenizer doesn't generate tokens with type == TT_USELESS but it's used as
            * argument of function submitToken()
            */
            TT_USELESS,
            // a text token
            TT_TEXT,
            // literials
            TT_INTEGER, TT_FLOAT, TT_STRING,

            // syntactical operators (treated differently than the others)
            TT_CURLY_OPEN, TT_CURLY_CLOSE, TT_SQUARE_OPEN, TT_SQUARE_CLOSE, TT_COLON, TT_COMMA,
                TT_DOT, TT_QUESTION_MARK, TT_ROUND_OPEN, TT_ROUND_CLOSE, TT_SEMICOLON,

            // pre operators
            TT_PRE_DEC, TT_PRE_INC, TT_PRE_MINUS, TT_PRE_PLUS, TT_COMPL, TT_NOT,

            // post operators
            TT_POST_DEC, TT_POST_INC,

            // math operators
            TT_DIV, TT_LEFT_SHIFT, TT_MINUS, TT_MULT, TT_MOD, TT_PLUS, TT_RIGHT_SHIFT, TT_OR, TT_AND,
                TT_XOR,

            // logic operators
            TT_LOGIC_AND, TT_LOGIC_OR,

            // compare operators
            TT_EQUAL, TT_GREATER, TT_GREATER_OR_EQUAL, TT_LESS, TT_LESS_OR_EQUAL, TT_NOT_EQUAL, TT_ELVIS,

            // assign operators
            TT_ASSIGN, TT_AND_ASSIGN, TT_DIV_ASSIGN, TT_LEFT_SHIFT_ASSIGN, TT_MINUS_ASSIGN, TT_MULT_ASSIGN,
                TT_MOD_ASSIGN, TT_OR_ASSIGN, TT_XOR_ASSIGN, TT_PLUS_ASSIGN, TT_RIGHT_SHIFT_ASSIGN,

            // instructions followed by '@' (not supported yet)
            TT_DIRECTIVE,

            // TT_UNPARSEDs are used internally the tokenizer to recognize correctly all the tokens
            TT_UNPARSED_OPERATOR, TT_UNPARSED_NUMBER,
            TT_UNPARSED_DIRECTIVE,

            // keywords
            TT_BREAK_KW, TT_BOX_KW, TT_CASE_KW, TT_CATCH_KW, TT_CLASS_KW,
            TT_CONTINUE_KW, TT_DEFAULT_KW, TT_DO_KW, TT_ELSE_KW,
            TT_ENUM_KW, TT_FALSE_KW, TT_FOR_KW, TT_FUNC_KW, TT_IF_KW,
            TT_NULL_KW, TT_PRIVATE_KW, TT_RETURN_KW, TT_REF_KW, TT_STATIC_KW,
            TT_SWITCH_KW, TT_SUPER_KW, TT_SYNC_KW, TT_THIS_KW, TT_THROW_KW, TT_TRUE_KW,
            T_TRY_KW, TT_USING_KW, TT_VAR_KW, TT_WHILE_KW,

            // shortcuts used by the tokenizer
            TT_KEYWORDS_START = TT_BREAK_KW,
            TT_OPERATORS_START = TT_CURLY_OPEN,
            TT_SYNTACTICAL_OPERATORS_START = TT_CURLY_OPEN,
            TT_NORMAL_OPERATORS_START = TT_PRE_DEC,
            TT_OPERATORS_END = TT_RIGHT_SHIFT_ASSIGN,
            TT_ASSIGN_START = TT_ASSIGN,
            TT_ASSIGN_END = TT_RIGHT_SHIFT_ASSIGN,
            TT_MAX = TT_WHILE_KW,
        };

        enum UnparsedNumberHints {
            UNH_FLOATING_POINT = 0x1,
            UNH_OPEN_EXPONENT = 0x2,
            UNH_ALREDY_OPEN_EXPONENT = 0x4,
            UNH_HEXADECIMAL = 0x8,
            UNH_OCTAL = 0x10,
            UNH_BINARY = 0x20,
        };

        constexpr const char* normalOperatorsPlain [] {
            "--", "++", "-", "+", "~", "!", "", "", "/", "<<", "-", "*", "%", "+", ">>", "|", "&", "^",
            "&&", "||", "==", ">", ">=", "<", "<=", "!=", "?:", "=", "&=", "/=", "<<=", "-=", "*=",
            "%=", "|=", "^=", "+=", ">>="
        };
        size_t constexpr normalOperatorsPLainLen = arraySize(normalOperatorsPlain);

        constexpr char singleCharOperators [] {
            '-', '+', '~', '!', '/', '*', '%', '|', '&', '^', '{', '}',
            '[', ']', ':', ',', '.', '?', '(', ')', ';', '>', '<', '='
        };
        size_t constexpr singleCharOperatorsLen = arraySize(singleCharOperators);

        constexpr const char* twoCharsOperators [] {
            "--", "++", "<<", ">>", "&&", "||", "==", ">=", "<=", "!=", "?:",
            "&=", "/=", "-=", "*=", "%=", "|=", "^=", "+="
        };
        size_t constexpr twoCharsOperatorsLen = arraySize(twoCharsOperators);

        const std::string threeCharsOperators [] {
            "<<=", ">>="
        };
        size_t constexpr threeCharsOperatorsLen = arraySize(threeCharsOperators);

        constexpr enum_t lengthOrderedOperatorIds [] {
            TT_MINUS, TT_PLUS, TT_COMPL, TT_NOT, TT_DIV, TT_MULT, TT_MOD,
            TT_OR, TT_AND, TT_XOR, TT_CURLY_OPEN, TT_CURLY_CLOSE, TT_SQUARE_OPEN,
            TT_SQUARE_CLOSE, TT_COLON, TT_COMMA, TT_DOT, TT_QUESTION_MARK,
            TT_ROUND_OPEN, TT_ROUND_CLOSE, TT_SEMICOLON,
            TT_GREATER, TT_LESS, TT_ASSIGN, TT_POST_DEC, TT_POST_INC, TT_LEFT_SHIFT,
            TT_RIGHT_SHIFT, TT_LOGIC_AND, TT_LOGIC_OR, TT_EQUAL, TT_GREATER_OR_EQUAL,
            TT_LESS_OR_EQUAL, TT_NOT_EQUAL, TT_ELVIS, TT_AND_ASSIGN, TT_DIV_ASSIGN,
            TT_MINUS_ASSIGN, TT_MULT_ASSIGN, TT_MOD_ASSIGN, TT_OR_ASSIGN,
            TT_XOR_ASSIGN, TT_PLUS_ASSIGN, TT_LEFT_SHIFT_ASSIGN, TT_RIGHT_SHIFT_ASSIGN,
        };
        size_t constexpr lengthOrderedOperatorIdsLen = arraySize(lengthOrderedOperatorIds);

        static_assert(lengthOrderedOperatorIdsLen == (singleCharOperatorsLen +
            twoCharsOperatorsLen + threeCharsOperatorsLen),
            "sizeof singleCharOperators + sizeof twoCharsOperators + sizeof threeCharsOperators "
            "must be equal to sizeof lengthOrderedOperatorIds (all std::string[] or char[])");

        constexpr unsigned operatorPriorities [] {
            // syntactical operators
            0, 0, 0, 0, 12, 14, 0, 12, 0, 0, 15,
            // pre operators
            1, 1, 1, 1, 1, 1,
            // post operators
            0, 0,
            // math operators
            2, 4, 3, 2, 2, 3, 4, 9, 7, 8,
            // logic operators
            10, 11,
            // compare operators
            6, 5, 5, 5, 5, 6, 12,
            // assign operators
            13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13

        };

        constexpr const char* keywords [] {
            // TODO: LOW remove some keywords here and above (from the enum TokenType) and in Tokenizer.cpp!!!
            "break", "box", "case", "catch", "class", "continue", "default",
            "do", "else", "enum", "false", "for", "func", "if", "null", "hidden",
            "return", "ref", "static", "switch", "super", "sync", "this", "throw", "true",
            "try", "import", "var", "while"
        };
        size_t constexpr keywordsLen = arraySize(keywords);

        struct Token{
            enum_t type = TT_UNPARSED;
            unsigned ln = 0, ch = 0, source = 0;
            string_t content;
            union {
                float_t f = 0.f;
                integer_t i;
            };
        };

        struct TokenizerStates{
            unsigned file_line = 0, file_char = 0, source = 0;
            unsigned* idx = nullptr;
            bool double_quote = false, single_quote = false, back_slash = false,
                nullToken = false;
            ascii_t curr_ch = 0;
            Token** back_ptr = nullptr;
            runtime::Runtime_t* rt = nullptr;
            const string_t* str;
        };

        constexpr const char* representations[] = {
            "<nothing>", "useless-token", "identifier", "integer literal",
            "floating-point literal", "string literal", "'{'", "'}'", "'['",
            "']'", "':'", "','", "'.'", "'?'", "'('", "')'", "';'", "'--'",
            "'++'", "'-'", "'+'", "'~'", "'!'", "'--'", "'++'", "'/'", "'<<'",
            "'-'", "'*'", "'%'", "'+'", "'>>'", "'|'", "'&'", "'^'", "'&&'",
            "'||'", "'=='", "'>'", "'>='", "'<'", "'<='", "'!='", "'?:'", "'='",
            "'&='", "'/='", "'<<='", "'-='", "'*='", "'%='", "'|='", "'^='",
            "'+='", "'>>='", "'@'", "<operator>", "<number>", "<directive>",
            "'break'", "'box'", "'case'", "'catch'", "'class'", "'continue'",
            "'default'","'do'", "'else'", "'enum'", "'false'", "'for'",
            "'func'", "'if'", "'null'", "'hidden'", "'return'",
            "'ref'", "'static'", "'switch'", "'super'", "'sync'", "'this'", "'throw'",
            "'true'", "'try'", "'import'", "'var'", "'while'",
        };
        static_assert (arraySize(representations) == TT_MAX+1,
            "size of representations (string array) must be equal to "
            "size of TokenType (enum)");

        std::string representation(const Token& token) noexcept;

        namespace test{
            constexpr const char* tokenTypes[] = {
                "UNPARSED", "USELESS", "TEXT", "INTEGER", "FLOAT", "STRING",
                "CURLY_OPEN", "CURLY_CLOSE", "SQUARE_OPEN", "SQUARE_CLOSE",
                "COLON", "COMMA", "DOT", "QUESTION_MARK", "ROUND_OPEN",
                "ROUND_CLOSE", "SEMICOLON", "PRE_DEC", "PRE_INC", "PRE_MINUS",
                "PRE_PLUS", "COMPL", "NOT", "POST_DEC", "POST_INC", "DIV",
                "LEFT_SHIFT", "MINUS", "MULT", "MOD", "PLUS", "RIGHT_SHIFT",
                "OR", "AND", "XOR", "LOGIC_AND", "LOGIC_OR", "EQUAL", "GREATER",
                "GREATER_OR_EQUAL", "LESS", "LESS_OR_EQUAL", "NOT_EQUAL",
                "ELVIS", "ASSIGN", "AND_ASSIGN", "DIV_ASSIGN", "LEFT_SHIFT_ASSIGN",
                "MINUS_ASSIGN", "MULT_ASSIGN", "MOD_ASSIGN", "OR_ASSIGN",
                "XOR_ASSIGN", "PLUS_ASSIGN", "RIGHT_SHIFT_ASSIGN", "DIRECTIVE",
                "UNPARSED_OPERATOR", "UNPARSED_NUMBER", "UNPARSED_DIRECTIVE",
                "BREAK_KW", "BOX_KW", "CASE_KW", "CATCH_KW", "CLASS_KW",
                "CONTINUE_KW", "DEFAULT_KW", "DO_KW", "ELSE_KW",
                "ENUM_KW", "FALSE_KW", "FOR_KW", "FUNC_KW", "IF_KW",
                "NULL_KW", "PRIVATE_KW", "RETURN_KW", "REF_KW", "STATIC_KW",
                "SWITCH_KW", "SUPER_KW", "SYNC_KW", "THIS_KW", "THROW_KW", "TRUE_KW",
                "TRY_KW", "USING_KW", "VAR_KW", "WHILE_KW",
            };
            std::string to_string(const Token& token) noexcept;
        }

        using TokenVec_t = std::vector<Token>;
    }
}

#endif // _SM__PARSE__TOKEN_H
