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
 *      File osd/File.h
 *
*/

#include <string>

#include "sm/typedefs.h"
#include "sm/utils/String.h"

#ifndef _SM_OS_WINDOWS
    #include <dirent.h>
#endif

namespace sm{
    enum Permissions {
        PERM_READABLE = 0x1,
        PERM_WRITABLE = 0x2,
        PERM_EXECUTABLE = 0x4,
        PERM_ALL = 0x7,

        PERM_INVALID = 0x10,
    };

    class Directory {
        friend class File;
    private:
        #ifdef _SM_OS_WINDOWS
            HANDLE handle;
        #else
            DIR* handle;
        #endif
        bool closed, valid;
        Directory(bool v) : closed(false), valid(v) {}
    public:
        Directory(const Directory&) = delete;
        Directory(Directory&&) noexcept;
        Directory& operator=(const Directory&) = delete;
        Directory& operator=(Directory&&) noexcept;

        bool next(std::string& file) noexcept;
        bool is_valid() noexcept;
        void close() noexcept;

        ~Directory() { close(); }
    };

    class File {
    private:
        std::string name;
        #ifdef _SM_OS_WINDOWS
        std::wstring wname;
        #endif
        unsigned perm;
    public:
        File(std::string file) noexcept;
        File(const File&) = default;
        File(File&&) = default;

        File& operator= (const File&) = default;
        File& operator= (File&&) = default;
        void set(std::string file) noexcept;

        bool exists() noexcept;
        bool remove() noexcept;
        bool move(std::string newName) noexcept;
        bool copy(const std::string& newName) noexcept;

        bool is_dir() noexcept;
        bool is_file() noexcept;

        void check_permissions() noexcept;
        bool can_read() noexcept;
        bool can_write() noexcept;
        bool can_execute() noexcept;
        bool valid_permissions() noexcept;

        bool make_file() noexcept;
        bool make_dir() noexcept;

        bool last_modified(integer_t& when) noexcept;
        bool last_access(integer_t& when) noexcept;

        bool is_empty(bool& out) noexcept;
        bool size(size_t& size) noexcept;

        Directory list_dir(std::string& file) noexcept;

        std::string get_parent() noexcept;
        std::string get_name() noexcept;
        bool full_path(std::string& str) noexcept;
        std::string path() noexcept;

        #if fileSeparator == '/'
            static const std::string& path(const std::string&) noexcept;
        #else
            static std::string path(std::string) noexcept;
        #endif

        ~File() = default;
    };
}
