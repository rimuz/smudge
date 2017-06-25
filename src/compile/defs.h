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
 *      File compile/defs.h
 *
*/

#include <string>
#include <vector>

#include "runtime/Object.h"
#include "typedefs.h"

namespace sm {
    namespace lib {
        extern oid_t idNew;
        extern oid_t idDelete;
    }

    namespace compile {
        using NamesMap_t = Map_t<string_t, uint16_t>;
        using StringsMap_t = Map_t<std::string, uint16_t>;
        using IntsMap_t = Map_t<integer_t, uint16_t>;
        using FloatsMap_t = Map_t<float_t, uint16_t>;
        using ImportedBox_t = std::tuple<unsigned, unsigned>;
        using ImportsVec_t = std::vector<ImportedBox_t>;
    }
}
