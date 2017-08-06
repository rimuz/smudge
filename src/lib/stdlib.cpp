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
 *      File lib/stdlib.cpp
 *
*/

#include "lib/stdlib.h"

namespace sm {
    namespace lib {
        smLibDecl(io);
        smLibDecl(lang);
        smLibDecl(system);
        smLibDecl(cast);

        const LibDict_t libs = {
            smLibTuple("std.io!", io),
            smLibTuple("std.lang!", lang),
            smLibTuple("std.system!", system),
            smLibTuple("std.cast!", cast),
        };
    }
}
