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
 *      File runtime/casts.h
 *
*/

#ifndef _SM__RUNTIME__CASTS_H
#define _SM__RUNTIME__CASTS_H

#include "runtime/Object.h"
#include "exec/Interpreter.h"
#include "utils/String.h"
#include "typedefs.h"

namespace sm{
    namespace runtime{
        /*
         * perform cast then return true if it was successful, false otherwise.
         * If function returns false, object 'out' could have been modified or not..
        */
        bool implicitToInt(const Object& in, integer_t& out);
        bool implicitToInt(const Object& in, Object& out);

        bool implicitToFloat(const Object& in, float_t& out);
        bool implicitToFloat(const Object& in, Object& out);

        Object implicitToString(exec::Interpreter& intp, const Object& in);
        string_t errorString(exec::Interpreter& intp, const Object& in);

        bool implicitToBool(const Object& in);
        bool of_type(const Object& in, Class* type);

        /*
         * Check if 'in' could be called as a function (if it has operator ())
         * and, it 'callable' returns true, set out to a valid Function ptr
         * and set self to the object to pass as 'this' to 'makeCall'
         * (or 'callFunction').
        */
        bool callable(const Object& in, Object& self, Function*& out);

    }
}

#endif
