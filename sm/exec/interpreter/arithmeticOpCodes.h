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
 *      File exec/interpreter/arithmeticOpCodes.h
 *
*/

#ifndef _SM__EXEC__INTERPRETER__ARITHMETICOPCODES_H
#define _SM__EXEC__INTERPRETER__ARITHMETICOPCODES_H

#include <functional>
#include <cmath>

#include "sm/exec/interpreter/defines.h"
#include "sm/runtime/id.h"
#include "sm/runtime/casts.h"

namespace sm{
    namespace exec{
        _OcFunc(Add){
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(+, std::plus<float_t>(), parse::TT_PLUS, false,);
        }

        _OcFunc(Sub){
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(-, std::minus<float_t>(), parse::TT_MINUS, false,);
        }

        _OcFunc(Mul){
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            _OcOp(*, std::multiplies<float_t>(), parse::TT_MULT, false,);
        }

        _OcFunc(Div){
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);
            if((tos->type == ObjectType::INTEGER && tos->i == 0)
                    || (tos->type == ObjectType::FLOAT && tos->f == 0.f))
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    "division by zero (by instruction DIV)");
            _OcOp(/, std::divides<float_t>(), parse::TT_DIV, false,);
        }

        _OcFunc(Mod){
            _OcPopStore(tos);
            _OcStore(tos1);
            _OcValue(tos1);
            _OcSimplifyRef(tos);

            if((tos->type == ObjectType::INTEGER && tos->i == 0)
                    || (tos->type == ObjectType::FLOAT && tos->f == 0.f))
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    "division by zero (by instruction MOD)");
            _OcOp(%, std::fmod, parse::TT_MOD, false,);
        }
    }
}

#endif
