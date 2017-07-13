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
 *      File runtime/id.h
 *
*/

#ifndef _SM__RUNTIME__ID_H
#define _SM__RUNTIME__ID_H

#include "runtime/gc.h"
#include "runtime/id.h"
#include "typedefs.h"

#include <iostream>

namespace sm {
    namespace runtime {
        constexpr unsigned initId = 22;
        constexpr unsigned roundId = 23; // operator ()
        constexpr unsigned squareId = 24; // operator []
        constexpr unsigned dataId = 25;
        // ... all data ids ...
        constexpr unsigned idsStart = 30;

        constexpr unsigned operatorIds[] = {
            // pre operators
            0, 1, 2, 3, 4, 5,

            // post operators
            0, 1,

            // math operators
            6, 7, 2, 8, 9, 3, 10, 11, 12, 13,

            // logic operators
            14, 15,

            // compare operators
            16, 17, 18, 19, 20, 21,
        };

        inline constexpr unsigned operatorId (enum_t tokenType){
            return operatorIds[tokenType - parse::TT_NORMAL_OPERATORS_START];
        }

        unsigned genOrdinaryId(Runtime_t&, const string_t&);

        template <enum_t Type>
        bool find(const Object&, Object&, unsigned id){
            return false;
        }

        bool find_any(const Object&, Object&, unsigned id);

        //  Specializations of bool find()
        template <>
        bool find<ObjectType::CLASS_INSTANCE>(const Object& in, Object& out, unsigned id);

        template <>
        bool find<ObjectType::ENUM>(const Object& in, Object& out, unsigned id);

        template <>
        bool find<ObjectType::CLASS>(const Object& in, Object& out, unsigned id);

        template <>
        bool find<ObjectType::BOX>(const Object& in, Object& out, unsigned id);

        template <>
        bool find<ObjectType::STRING>(const Object& in, Object& out, unsigned id);
    }
}

#endif
