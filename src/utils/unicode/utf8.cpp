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
 *      File utils/unicode/utf8.cpp
 *
*/

#include <iostream>
#include "utils/unicode/utf8.h"

namespace sm {
    /*
     * How does UTF-8 work:
     * 1 byte:      0xxxxxxx (ASCII)
     * 2 bytes:     110xxxxx 10xxxxxx
     * 3 bytes:     1110xxxx 10xxxxxx 10xxxxxx
     * 4 bytes:     11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    */

    unsigned uGetSkip(ascii_t ch) noexcept {
        if((ch & UTF8_PREFIX) && (ch & UTF8_2B_PREFIX)){
            if(ch & UTF8_3B_PREFIX){
                if(ch & UTF8_4B_PREFIX){
                    return 3;
                } else {
                    return 2;
                }
            } else {
                return 1;
            }
        }
        return 0;
    }

    unsigned uGetSize(unicode_t ch) noexcept{
        return (ch & 0xFF00) ? ((ch & 0xFF0000) ? ((ch & 0xFF000000) ? 4 : 3) : 2) : 1;
    }

    unicode_t uGetCodepoint(unicode_t ch) noexcept{
        if(ch & 0xFF00){
            unicode_t cp = ch & 0x3F;
            std::cout << "ok:" << cp << std::endl;
            if(ch & 0xFF0000) {
                cp |= ((ch & 0x3F00) >> 2);
                if(ch & 0xFF000000)
                    return cp | ((ch & 0x3F0000) >> 4) | ((ch & 0x7000000) >> 6);
                else
                    return cp | ((ch & 0xF0000) >> 4);
            } else {
                return cp | ((ch & 0x1F00) >> 2);
            }
        }
        return ch;
    }
    
    unicode_t uByCodepoint(unicode_t cp) noexcept{
        if(cp < 0x80)
            return cp;
        else if (cp < 0x800)
            return ((0xc0 | (cp >> 6)) << 8) | (0x80 | (cp & 0x3f));
        else if(cp < 0x10000)
            return ((0xe0 | (cp >> 12)) << 16) | ((0x80 | ((cp >> 6) & 0x3f)) << 8)
                | (0x80 | (cp & 0x3f));
        else if(cp < 0x110000)
            return  ((0xf0 | (cp >> 18)) << 24) | ((0x80 | ((cp >> 12) & 0x3f)) << 16)
                | ((0x80 | ((cp >> 6) & 0x3f)) << 8) | (0x80 | (cp & 0x3f));
        else
            return UTF8_ERROR;
    }

    struct AlphaCharU8 {
        unicode_t upper, lower;
    };

    constexpr AlphaCharU8 chars_2_2[] = {
        #include "utils/unicode/chars_2_2.inc"
    };

    constexpr AlphaCharU8 chars_2_3[] = {
        #include "utils/unicode/chars_2_3.inc"
    };

    constexpr AlphaCharU8 chars_3_2[] = {
        #include "utils/unicode/chars_3_2.inc"
    };

    constexpr AlphaCharU8 chars_3_3[] = {
        #include "utils/unicode/chars_3_3.inc"
    };

    constexpr AlphaCharU8 chars_4_3[] = {
        #include "utils/unicode/chars_4_3.inc"
    };

    constexpr AlphaCharU8 chars_4_4[] = {
        #include "utils/unicode/chars_4_4.inc"
    };

    constexpr size_t    chars_2_2_sz = arraySize(chars_2_2),
                        chars_2_3_sz = arraySize(chars_2_3),
                        chars_3_2_sz = arraySize(chars_3_2),
                        chars_3_3_sz = arraySize(chars_3_3),
                        chars_4_3_sz = arraySize(chars_4_3),
                        chars_4_4_sz = arraySize(chars_4_4);

    unicode_t uUpper(unicode_t ch) noexcept {
        switch(uGetSize(ch)){
            case 1: {
                if(ch >= ascii_firstLow && ch <= ascii_lastLow)
                    return ch - ascii_caseDiff;
                return ch;
            }

            case 2: {
                size_t idx = 0;
                for(; idx != chars_2_2_sz; ++idx)
                    if(chars_2_2[idx].lower == ch)
                        return chars_2_2[idx].upper;
                for(idx = 0; idx != chars_3_2_sz; ++idx)
                    if(chars_3_2[idx].lower == ch)
                        return chars_3_2[idx].upper;
                return ch;
            }

            case 3: {
                size_t idx = 0;
                for(; idx != chars_2_3_sz; ++idx)
                    if(chars_2_3[idx].lower == ch)
                        return chars_2_3[idx].upper;
                for(idx = 0; idx != chars_3_3_sz; ++idx)
                    if(chars_3_3[idx].lower == ch)
                        return chars_3_3[idx].upper;
                for(idx = 0; idx != chars_4_3_sz; ++idx)
                    if(chars_4_3[idx].lower == ch)
                        return chars_4_3[idx].upper;
                return ch;
            }

            case 4: {
                for(size_t idx = 0; idx != chars_4_4_sz; ++idx)
                    if(chars_4_4[idx].lower == ch)
                        return chars_4_4[idx].upper;
                return ch;
            }
        }
        return UTF8_ERROR;
    }

    unicode_t uLower(unicode_t ch) noexcept {
        switch(uGetSize(ch)){
            case 1: {
                if(ch >= ascii_firstUp && ch <= ascii_lastUp)
                    return ch + ascii_caseDiff;
                return ch;
            }

            case 2: {
                size_t idx = 0;
                for(; idx != chars_2_2_sz; ++idx)
                    if(chars_2_2[idx].upper == ch)
                        return chars_2_2[idx].lower;
                for(idx = 0; idx != chars_2_3_sz; ++idx)
                    if(chars_2_3[idx].upper == ch)
                        return chars_2_3[idx].lower;
                return ch;
            }

            case 3: {
                size_t idx = 0;
                for(; idx != chars_3_2_sz; ++idx)
                    if(chars_3_2[idx].upper == ch)
                        return chars_3_2[idx].lower;
                for(idx = 0; idx != chars_3_3_sz; ++idx)
                    if(chars_3_3[idx].upper == ch)
                        return chars_3_3[idx].lower;
                return ch;
            }

            case 4: {
                size_t idx = 0;
                for(; idx != chars_4_3_sz; ++idx)
                    if(chars_4_3[idx].upper == ch)
                        return chars_4_3[idx].lower;
                for(idx = 0; idx != chars_4_4_sz; ++idx)
                    if(chars_4_4[idx].lower == ch)
                        return chars_4_4[idx].upper;
                return ch;
            }
        }
        return UTF8_ERROR;
    }
}
