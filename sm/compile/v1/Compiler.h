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
    namespace parse{
        struct Token;
    }

    namespace compile {
        namespace v1 {
            namespace ParType{
                enum {
                    // ROUND:
                    _ROUND_START,
                    EXPR_ROUND, FUNC_CALL, TUPLE, REF_CALL, IS_NULL_CALL,
                    SUPER_FIND_CALL, DEFAULT_ARGUMENT, SUPER_EXPR,

                    // ROUND.HEAD:
                    _HEAD_START,
                    IF_HEAD, WHILE_HEAD, DO_WHILE_HEAD, FOR_HEAD3, SWITCH_HEAD,
                    FOREACH_HEAD,
                    _HEAD_END,
                    _ROUND_END,

                    // SQUo install ARE:
                    _SQUARE_START,
                    BRACING, LIST,
                    _SQUARE_END,

                    // CURLY:
                    // order counts!
                    _CURLY_START,
                    IF_BODY, WHILE_BODY, FOR_BODY,
                    DO_BODY, ELSE_BODY, FUNCTION_BODY,
                    SWITCH_BODY, CODE_BLOCK,
                    _CURLY_END,

                    // DECLARATIONS:
                    _DECLARATIONS_START,
                    VAR_DECL, GLOBAL_VAR_DECL, CONDITIONAL_OPERATOR2, ELVIS_OPERATOR,
                    LOGIC_OR_OPERATOR, LOGIC_AND_OPERATOR,
                    _DECLARATIONS_END,

                    // FOR STATEMENTS:
                    _FOR_STATEMENTS_START,
                    FOR_HEAD1, FOR_HEAD2,
                    _FOR_STATEMENTS_END,

                    // STATEMENTS:
                    // order counts!
                    _STATEMENTS_START,
                    IF_STATEMENT, WHILE_STATEMENT, FOR_STATEMENT,
                    DO_STATEMENT, ELSE_STATEMENT, FUNCTION_STATEMENT,
                    _STATEMENTS_END,

                    // SPECIALS
                    CONDITIONAL_OPERATOR1, CASE_HEAD, RETURN_STATEMENT,
                };

                struct LoopStatements_t {
                    IndexVector_t breaks, continues;
                };

                struct ParInfo_t {
                    ParInfo_t(enum_t _parType) : parType(_parType), arg0(0), arg1(0), arg2(0), loopStatements(nullptr){}
                    ParInfo_t() : parType(0), arg0(0), arg1(0), arg2(0), loopStatements(nullptr){}

                    inline bool isHead() const noexcept;
                    inline bool isRound() const noexcept;
                    inline bool isSquare() const noexcept;
                    inline bool isCurly() const noexcept;
                    inline bool isDeclaration() const noexcept;
                    inline bool isForStatement() const noexcept;
                    inline bool isSpecialStatement() const noexcept;
                    inline bool canCommaIncrement() const noexcept;
                    inline bool isCodeBlock() const noexcept;

                    enum_t parType;
                    size_t arg0, arg1, arg2;
                    union {
                        LoopStatements_t* loopStatements;
                        Function* funcPtr;
                        Class* classPtr;
                    };
                };

                struct Operator_t {
                    template <typename A, typename B>
                    Operator_t (A a, B b) : type(a), pr(b) {}
                    Operator_t () : type(0), pr(0) {}

                    unsigned type, pr; // pr = priority
                };

                inline bool ParInfo_t::isHead() const noexcept {
                    return parType > _HEAD_START && parType < _ROUND_END;
                }

                inline bool ParInfo_t::isRound() const noexcept {
                    return parType > _ROUND_START && parType < _ROUND_END;
                }

                inline bool ParInfo_t::isSquare() const noexcept{
                    return parType > _SQUARE_START && parType < _SQUARE_END;
                }

                inline bool ParInfo_t::isCurly() const noexcept{
                    return parType > _CURLY_START && parType < _CURLY_END;
                }

                inline bool ParInfo_t::isDeclaration() const noexcept{
                    return parType > _DECLARATIONS_START && parType < _DECLARATIONS_END;
                }

                inline bool ParInfo_t::isForStatement() const noexcept{
                    return parType > _FOR_STATEMENTS_START && parType < _FOR_STATEMENTS_END;
                }

                inline bool ParInfo_t::isSpecialStatement() const noexcept{
                    return parType > _STATEMENTS_START && parType < _STATEMENTS_END;
                }

                inline bool ParInfo_t::canCommaIncrement() const noexcept{
                    return parType == FUNC_CALL || parType == TUPLE || parType == BRACING
                        || parType == LIST;
                }

                inline bool ParInfo_t::isCodeBlock() const noexcept{
                    return isCurly() || isSpecialStatement();
                }
            }

            using ParStack_t = std::vector<ParType::ParInfo_t>;

            struct CompilerStates{
                ParStack_t parStack;
                std::vector<ParType::Operator_t> operators;
                std::vector<unsigned> toImportAll;

                ByteCode_t* output;
                ImportsVec_t* toImport;
                Box* currBox = nullptr;
                Class* currClass = nullptr;
                parse::TokenVec_t::const_iterator it, begin, end;

                bool isLastOperand = false,      isLastDot = false,
                     expectedLvalue = false,     rvalue = false,
                     isStatementEmpty = false,   wasStatementEmpty = false,
                     isClassStatement = false;
            };

            class Compiler{
                friend void expect_next(Compiler& cp, CompilerStates& states,
                    enum_t expected) noexcept;
                friend bool is_next(Compiler& cp, CompilerStates& states,
                    enum_t expected) noexcept;
            private:
                runtime::Runtime_t* _rt;
                ByteCode_t _temp; // Used for <init>
                ByteCode_t _classTemp; // Used for classes' <init>


                StringsMap_t _strings;
                IntsMap_t _ints;
                FloatsMap_t _floats;

                unsigned _nfile;

                void _ultimateToken(CompilerStates& states);
                void _globalScopeCompile(CompilerStates& states);
                void _localScopeCompile(CompilerStates& states);
                void _operatorsCompile(CompilerStates& states);
                void _declareVar(CompilerStates& states, ByteCode_t& out, bool global);

                void _compile(const parse::TokenVec_t& tokens);
            public:
                Compiler(runtime::Runtime_t& rt);

                /*
                 * Usually, you don't need to copy or
                 * move a Compiler instance, so I
                 * decided to remove all the chances
                 * of the bugs to born and grow :P
                */
                Compiler(const Compiler&) = delete;
                Compiler(Compiler&&) = delete;

                Compiler& operator=(const Compiler&) = delete;
                Compiler& operator=(Compiler&&) = delete;

                void source(string_t filePath);
                void source(string_t name, error::CodeSource* source);
                void code(string_t name, string_t* code);

                void path(const string_t& path);
                void start();
                void end();
                bool next();

                /*
                 * It doesn't delete all data instanced because
                 * it will be surely used later at runtime.
                */
                ~Compiler() = default;

                /*
                 * if the box is not found,
                 * it returns false, if the dynamic
                 * library doesn't contain the smLibDecl
                 * box will be set to nullptr
                */
                static bool load_native(const char* path, runtime::Runtime_t& rt,
                    unsigned id, Box*& box) noexcept;
                static error::CodeSource* readf(const string_t& filePath);
            };
        }
    }
}

#endif
