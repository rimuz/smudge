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
 *      File typedefs.h
 *
*/

#ifndef _SM__TYPEDEFS_H
#define _SM__TYPEDEFS_H

#include <string>
#include <vector>

#include "require_cpp11.h"

#define _SM_INT_VERSION 0002
#define _SM_STR_VERSION "0.2"
#define _SM_DATE_VERSION "04.2017"
#define _SM_EXECUTABLE_NAME "smudge"

#define _SM_PTR_SIZE sizeof(void*)

#   if defined(_WIN32) || defined(__CYGWIN__)
#       if defined(_WIN64)
#           define _SM_OS_WIN64
#       endif
#       define _SM_OS_WIN32
#       define _SM_OS_WINDOWS
#   elif defined(__linux__)
#      define _SM_OS_LINUX
#   elif defined(__APPLE__) && defined(__MACH__)
#       define _SM_OS_MACOSX
#   elif defined(Macintosh)
#       define _SM_OS_MACOS9
#   else
#       warning "Compiling for an unsupported Operating System."
#   endif

#ifdef _SM_OS_WINDOWS
#define _SM_FILE_SEPARATOR "\\"
#define _SM_PATH_SEPARATOR ";"
#else
#define _SM_FILE_SEPARATOR "/"
#define _SM_PATH_SEPARATOR ":"
#endif

namespace sm{
    using enum_t = unsigned char;
    using ascii_t = char;
    using byte_t = /* uint8_t */ unsigned char;

    // a type big enough to contain any unicode character
    using unicode_t = uint32_t;

    using integer_t = long;
    using float_t = double;
    using string_t = std::string;
    using NameVec_t = std::vector<unsigned>;
    using ByteCode_t = std::vector<unsigned char>;
    using ByteCodeVec_t = std::vector<ByteCode_t>;
    using IndexVector_t = std::vector<size_t>;
    using StringCharType_t = char;

    constexpr unsigned uintMSB = 1u << (8 * sizeof(unsigned) -1);
    constexpr ascii_t fileSeparator =
    #ifdef _SM_OS_WINDOWS
        '\\';
    #else
        '/';
    #endif

    constexpr ascii_t pathSeparator =
    #ifdef _SM_OS_WINDOWS
        ';';
    #else
        ':';
    #endif

    template <typename Tp, size_t Sz>
    constexpr size_t arraySize(Tp (&) [Sz]) { return Sz; }

    template <typename UInt>
    constexpr UInt fillLSB(UInt i, size_t n_bits){
        return n_bits ? (fillLSB(i, n_bits -1) | (1 << n_bits)) : (i | 1);
    }

    constexpr const char* searchPaths [] = {
        #ifdef _SM_OS_WINDOWS
            "C:\\smudge\\" _SM_STR_VERSION "\\lib\\",
            "C:\\smudge\\" _SM_STR_VERSION "\\",
            "C:\\smudge\\lib\\",
            "C:\\smudge\\"
        #else
            "/usr/lib/smudge/" _SM_STR_VERSION "/",
            "/usr/lib/smudge/",
        #endif
    };
    constexpr size_t searchPathsLen = arraySize(searchPaths);
}

#endif // _SM__TYPEDEFS_H
