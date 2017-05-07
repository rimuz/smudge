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
 *      File runtime/utils.h
 *
*/

#ifndef _SM__RUNTIME__UTILS_H
#define _SM__RUNTIME__UTILS_H

#include "runtime/id.h"
#include "runtime/casts.h"
#include "parse/Token.h"
#include "typedefs.h"

#include "exec/interpreter/defines.h"
#include "exec/Interpreter.h"

namespace sm {
    namespace runtime {
        enum CompareMode {
            INT_COMPARE, FLOAT_COMPARE, STRING_COMPARE, CALL_COMPARE_FUNCTION,
        };

        template <enum_t TT>
        class Comparator {
        private:
            static constexpr unsigned ID = runtime::operatorId(TT);

            exec::Interpreter& intp;
            Object self;
            Function* fn;
            unsigned char mode;

            template <typename Tp>
            inline bool compare(Tp lhs, Tp rhs) const noexcept;
        public:
            Comparator(exec::Interpreter&, Object);
            Comparator(const Comparator<TT>&) = default;
            Comparator(Comparator<TT>&&) = default;

            bool operator() (const Object& obj) const noexcept;

            Comparator<TT>& operator=(const Comparator<TT>&) = default;
            Comparator<TT>& operator=(Comparator<TT>&&) = default;

            ~Comparator() = default;
        };

        using Equal = Comparator<parse::TT_EQUAL>;
        using Greater = Comparator<parse::TT_GREATER>;
        using Less = Comparator<parse::TT_LESS>;
        using GreaterOrEqual = Comparator<parse::TT_GREATER_OR_EQUAL>;
        using LessOrEqual = Comparator<parse::TT_LESS_OR_EQUAL>;
        using NotEqual = Comparator<parse::TT_NOT_EQUAL>;

        template<enum_t TT>
        Comparator<TT>::Comparator(exec::Interpreter& _intp, Object _self)
                : intp(_intp) {
            _OcValue(_self);
            switch(_self.type){
                case ObjectType::INTEGER:
                    mode = INT_COMPARE;
                    self = std::move(_self);
                    return;

                case ObjectType::FLOAT:
                    mode = FLOAT_COMPARE;
                    self = std::move(_self);
                    return;

                case ObjectType::CLASS_INSTANCE: {
                    mode = CALL_COMPARE_FUNCTION;
                    self = _self; // not move!

                    Object func;
                    if(!runtime::find<ObjectType::CLASS_INSTANCE>(_self, func, ID)){
                        intp.rt->sources.printStackTrace(intp, error::ERROR,
                            std::string("can't find operator") +
                            parse::normalOperatorsPlain[TT - parse::TT_NORMAL_OPERATORS_START]
                            + "() in " + runtime::errorString(intp, _self));
                    } else if(!runtime::callable(func, self, fn)){
                        intp.rt->sources.printStackTrace(intp, error::ERROR,
                            std::string("operator") +
                            parse::normalOperatorsPlain[TT - parse::TT_NORMAL_OPERATORS_START]
                            + "() is not a function in " + runtime::errorString(intp, _self));
                    }
                    return;
                }

                case ObjectType::BOX: {
                    Object func;
                    mode = CALL_COMPARE_FUNCTION;
                    if(!runtime::find<ObjectType::BOX>(_self, func, ID)){
                        intp.rt->sources.printStackTrace(intp, error::ERROR,
                            std::string("can't find operator") +
                            parse::normalOperatorsPlain[TT - parse::TT_NORMAL_OPERATORS_START]
                            + "() in " + runtime::errorString(intp, _self));
                    } else if(!runtime::callable(func, self, fn)){
                        intp.rt->sources.printStackTrace(intp, error::ERROR,
                            std::string("operator") +
                            parse::normalOperatorsPlain[TT - parse::TT_NORMAL_OPERATORS_START]
                            + "() is not a function in " + runtime::errorString(intp, _self));
                    }
                    return;
                }

                default:
                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("can't find operator") +
                        parse::normalOperatorsPlain[TT - parse::TT_NORMAL_OPERATORS_START]
                        + "() in " + runtime::errorString(intp, self));
            }
        }

        template <enum_t TT>
        bool Comparator<TT>::operator() (const Object& obj) const noexcept{
            switch(mode){
                case INT_COMPARE: {
                    switch(obj.type){
                        case ObjectType::INTEGER:
                            return compare<integer_t>(self.i, obj.i);
                        case ObjectType::FLOAT:
                            return compare<float_t>(self.i, obj.f);
                        default:
                            intp.rt->sources.printStackTrace(intp, error::ERROR,
                                std::string("can't perform operator") +
                                parse::normalOperatorsPlain[TT - parse::TT_NORMAL_OPERATORS_START]
                                + "() between <int> and " + runtime::errorString(intp, self));
                    }
                }

                case FLOAT_COMPARE: {
                    switch(obj.type){
                        case ObjectType::INTEGER:
                            return compare<float_t>(self.f, obj.i);
                        case ObjectType::FLOAT:
                            return compare<float_t>(self.f, obj.f);
                        default:
                            intp.rt->sources.printStackTrace(intp, error::ERROR,
                                std::string("can't perform operator") +
                                parse::normalOperatorsPlain[TT - parse::TT_NORMAL_OPERATORS_START]
                                + "() between <float> and " + runtime::errorString(intp, self));
                    }
                }

                case STRING_COMPARE: {
                    if(obj.type == ObjectType::STRING)
                        return compare<const String&>(self.s_ptr->str, obj.s_ptr->str);
                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("can't perform operator") +
                        parse::normalOperatorsPlain[TT - parse::TT_NORMAL_OPERATORS_START]
                        + "() between <string> and " + runtime::errorString(intp, self));
                }

                case CALL_COMPARE_FUNCTION: {
                    return runtime::implicitToBool(intp.callFunction(fn, {obj}, self, true));
                }

                default:
                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("compare mode ") + std::to_string(TT) +
                        " not supported by sm::runtime::Comparator (err #4)");
                    return false;
            }
        }

        template <enum_t TT>
        template <typename Tp>
        inline bool Comparator<TT>::compare(Tp lhs, Tp rhs) const noexcept{
            intp.rt->sources.printStackTrace(intp, error::ERROR,
                std::string("operator (value = ") + std::to_string(TT) +
                ") not supported by sm::runtime::Comparator (err #5)");
            return false;
        }

        template <>
        template <typename Tp>
        inline bool Comparator<parse::TT_EQUAL>::compare(Tp lhs, Tp rhs) const noexcept{
            return lhs == rhs;
        }

        template <>
        template <typename Tp>
        inline bool Comparator<parse::TT_GREATER>::compare(Tp lhs, Tp rhs) const noexcept{
            return lhs > rhs;
        }

        template <>
        template <typename Tp>
        inline bool Comparator<parse::TT_GREATER_OR_EQUAL>::compare(Tp lhs, Tp rhs) const noexcept{
            return lhs >= rhs;
        }

        template <>
        template <typename Tp>
        inline bool Comparator<parse::TT_LESS>::compare(Tp lhs, Tp rhs) const noexcept{
            return lhs < rhs;
        }

        template <>
        template <typename Tp>
        inline bool Comparator<parse::TT_LESS_OR_EQUAL>::compare(Tp lhs, Tp rhs) const noexcept{
            return lhs <= rhs;
        }

        template <>
        template <typename Tp>
        inline bool Comparator<parse::TT_NOT_EQUAL>::compare(Tp lhs, Tp rhs) const noexcept{
            return lhs != rhs;
        }
    }
}

#endif
