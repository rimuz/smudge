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
#include "sm/require_cpp11.h"

#define _SM_INT_VERSION 400
#define _SM_STR_VERSION "0.4"
#define _SM_DATE_VERSION "12.2017"
#define _SM_EXECUTABLE_NAME "smudge"
#define _SM_DEFAULT_BOX_NAME "main.sm"

#define _SM_PTR_SIZE sizeof(void*)
#define _SM_DEFAULT_MAX_SS 100000
#define _SM_DEFAULT_MIN_SS   1000
#define _SM_DEFAULT_STACK_PRINTED_ELEMENTS 25

/*
 * On Windows you can build a special
 * version of the Smudge Interpeter which
 * reads the bytecode from the resources
 * of the PE file. From now on, we'll
 * call that feature 'Windows embedded mode'
 *
 * To enable it, you have to define the
 * macro _SM_WIN_EMBED
 *
*/
#ifdef _SM_WIN_EMBED
#   define _SM_WINRES_CODE_ID 201
#endif


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
#       define _SM_OS_MACOS
#   elif defined(Macintosh)
#       define _SM_OS_MACOS9
#       define _SM_OS_MACOS
#   elif defined(__unix__)
#       define _SM_OS_UNIX
#   else
#       warning "Compiling for an unsupported Operating System."
#   endif

#   ifdef _SM_OS_WINDOWS
#       define _SM_FILE_SEPARATOR "\\"
#       define _SM_PATH_SEPARATOR ";"
#       define _SM_DL_EXT ".dll"
#   else
#       define _SM_FILE_SEPARATOR "/"
#       define _SM_PATH_SEPARATOR ":"
#       ifdef _SM_OS_MACOS
#           define _SM_DL_EXT ".dylib"
#       else
#           define _SM_DL_EXT ".so"
#       endif
#   endif

#   ifdef _SM_OS_WINDOWS
#       include <windows.h>
#   endif

namespace sm{
    using enum_t = unsigned char;
    using ascii_t = char;
    using byte_t = /* uint8_t */ unsigned char;

    // a type big enough to contain any unicode character
    using unicode_t = uint32_t;

    using integer_t = long long;
    using float_t = double;
    using string_t = std::string;
    using oid_t = unsigned; // Object ID
    using NameVec_t = std::vector<unsigned>;
    using ByteCode_t = std::vector<unsigned char>;
    using ByteCodeVec_t = std::vector<ByteCode_t>;
    using IndexVector_t = std::vector<size_t>;
    using StringCharType_t = char;

    constexpr unsigned garbageCollectorThreshold = 100;
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

    /*
     * casts the integer given to an unsigned char, useful for
     * avoiding `Narrowing conversion` in compiler.
    */
    template <typename UInt>
    inline unsigned char bc(UInt i){
        return static_cast<unsigned char>(i);
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
