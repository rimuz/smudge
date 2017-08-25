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
 *      File utils/unicode/utf8.h
 *
*/

#ifndef _SM__UTILS__UNICODE__UTF8_H
#define _SM__UTILS__UNICODE__UTF8_H

#include "sm/typedefs.h"

namespace sm{
    constexpr ascii_t ascii_firstUp = 'A';
    constexpr ascii_t ascii_lastUp = 'Z';
    constexpr ascii_t ascii_firstLow = 'a';
    constexpr ascii_t ascii_lastLow = 'z';
    constexpr ascii_t ascii_caseDiff = ascii_firstLow - ascii_firstUp;

    enum utf8flags{
        UTF8_PREFIX = 0x80,
        UTF8_2B_PREFIX = 0x40,
        UTF8_3B_PREFIX = 0x20,
        UTF8_4B_PREFIX = 0x10,
        UTF8_ERROR = 0xFFFFFFFF,
    };

    unicode_t uUpper(unicode_t) noexcept;
    unicode_t uLower(unicode_t) noexcept;
    unsigned uGetSkip(ascii_t) noexcept;
    unsigned uGetSize(unicode_t) noexcept;
    unicode_t uGetCodepoint(unicode_t ch) noexcept;
    unicode_t uByCodepoint(unicode_t cp) noexcept;
}

#endif
