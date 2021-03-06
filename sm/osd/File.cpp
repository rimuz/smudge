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
 *      File osd/File.cpp
 *
*/

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "sm/osd/File.h"
#include "sm/typedefs.h"

#ifdef _SM_OS_WINDOWS
    #include "accctrl.h"
    #include "aclapi.h"
    #include "shlwapi.h"
#else
    #include <libgen.h>
    #include <stdio.h>
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <dirent.h>
#endif

namespace sm{
    #ifdef _SM_OS_WINDOWS
        namespace {
            integer_t getUnixTime(FILETIME& ft){
                ULARGE_INTEGER val;
                val.LowPart = ft.dwLowDateTime;
                val.HighPart = ft.dwHighDateTime;

                integer_t time = static_cast<integer_t>(val.QuadPart);
                time /= 10000000;         // 100-nanos to seconds
                time -= 11644473600;      // 1601 to 1970
                return time;
            }
        }
    #endif

    Directory::Directory(Directory&& rhs) noexcept
            : handle(rhs.handle), closed(rhs.closed), valid(rhs.valid){
        rhs.valid = false;
    }

    Directory& Directory::operator=(Directory&& rhs) noexcept{
        handle = rhs.handle;
        valid = rhs.valid;
        closed = rhs.closed;
        rhs.valid = false;
        return *this;
    }

    bool Directory::next(std::string& file) noexcept{
        if(!valid || closed)
            return false;

        #ifdef _SM_OS_WINDOWS
            WIN32_FIND_DATAW data;
            if(!FindNextFileW(handle, &data))
                return false;

            int req_size = WideCharToMultiByte(CP_UTF8, 0, data.cFileName, -1, NULL, 0, NULL, NULL);
            file.resize(req_size);
            WideCharToMultiByte(CP_UTF8, 0, data.cFileName, -1, &file[0], req_size, NULL, NULL);
            file.pop_back();
        #else
            struct dirent* e = readdir(handle);
            if(!e)
                return false;
            file = e->d_name;
        #endif
        return true;
    }

    bool Directory::is_valid() noexcept{
        return valid;
    }

    void Directory::close() noexcept{
        if(valid && !closed){
            #ifdef _SM_OS_WINDOWS
                FindClose(handle);
            #else
                closedir(handle);
            #endif
        }
    }

    File::File(std::string file) noexcept : name(std::move(file)){
        #ifdef _SM_OS_WINDOWS
            int sz = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), name.size(), NULL, 0);
            wname.resize(sz);
            MultiByteToWideChar(CP_UTF8, 0, name.c_str(), name.size(), &wname[0], sz);
        #endif

        check_permissions();
    }

    void File::set(std::string file) noexcept{
        name.swap(file);
        #ifdef _SM_OS_WINDOWS
            int sz = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), name.size(), NULL, 0);
            wname.resize(sz);
            MultiByteToWideChar(CP_UTF8, 0, name.c_str(), name.size(), &wname[0], sz);
        #endif

        check_permissions();
    }

    bool File::exists() noexcept {
        #ifdef _SM_OS_WINDOWS
            return PathFileExistsW(wname.c_str());
        #else
            return access(name.c_str(), F_OK) == 0;
        #endif
    }

    bool File::remove() noexcept {
        bool ret;
        if(is_dir()){
            #ifdef _SM_OS_WINDOWS
                ret = RemoveDirectoryW(wname.c_str());
            #else
                ret = !rmdir(name.c_str());
            #endif
        } else {
            ret = !std::remove(name.c_str());
        }

        if(ret)
            perm = PERM_INVALID;
        return ret;
    }

    bool File::move(std::string newName) noexcept {
        bool ret;
        #ifdef _SM_OS_WINDOWS
            std::wstring newNameW;
            int sz = MultiByteToWideChar(CP_UTF8, 0, newName.data(), newName.size(), nullptr, 0);
            newNameW.resize(sz);
            MultiByteToWideChar(CP_UTF8, 0, newName.data(), newName.size(), &newNameW[0], sz);

            ret = MoveFileW(wname.c_str(), newNameW.c_str());
        #else
            ret = !std::rename(name.c_str(), newName.c_str());
        #endif

        check_permissions();
        return ret;
    }

    bool File::copy(const std::string& newName) noexcept {
        if(is_dir())
            return false;

        std::ifstream from(name.c_str(), std::ios::binary);
        std::ofstream to(newName.data(), std::ios::binary);
        if(!from || !to)
            return false;
        return static_cast<bool>(to << from.rdbuf());
    }

    bool File::is_dir() noexcept {
        #ifdef _SM_OS_WINDOWS
            DWORD attr = GetFileAttributesW(wname.c_str());
            return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
        #else
            struct stat s;
            return stat(name.c_str(), &s) == 0 && S_ISDIR(s.st_mode);
        #endif
    }

    bool File::is_file() noexcept {
        #ifdef _SM_OS_WINDOWS
            DWORD attr = GetFileAttributesW(wname.c_str());
            return attr != INVALID_FILE_ATTRIBUTES;
        #else
            struct stat s;
            return stat(name.c_str(), &s) == 0 && S_ISREG(s.st_mode);
        #endif
    }

    void File::check_permissions() noexcept {
        perm = PERM_INVALID;
        #ifdef _SM_OS_WINDOWS
            DWORD req_size;
            GetFileSecurityW(wname.c_str(), DACL_SECURITY_INFORMATION, NULL, 0, &req_size);
            PSECURITY_DESCRIPTOR sd_ptr = (PSECURITY_DESCRIPTOR) LocalAlloc(LMEM_ZEROINIT, req_size);

            if(sd_ptr){
                if(GetFileSecurityW(wname.c_str(), DACL_SECURITY_INFORMATION, sd_ptr, req_size, &req_size)){
                    PACL dacl = NULL;
                    BOOL hasDacl = FALSE, defaulted = FALSE;
                    if(GetSecurityDescriptorDacl(sd_ptr, &hasDacl, &dacl, &defaulted)){
                        if(hasDacl){
                            TRUSTEE_W trustee;
                            wchar_t username[] = L"CURRENT_USER";
                            BuildTrusteeWithNameW(&trustee, username);//&username[0]);

                            if(dacl){
                                ACCESS_MASK mask = 0;
                                if(GetEffectiveRightsFromAclW(dacl, &trustee, &mask) == ERROR_SUCCESS){
                                    perm =
                                        (mask & FILE_READ_DATA ? PERM_READABLE : 0) |
                                        (mask & FILE_WRITE_DATA ? PERM_WRITABLE : 0) |
                                        (mask & FILE_EXECUTE ? PERM_EXECUTABLE : 0);
                                }
                            } else {
                                perm = PERM_ALL;
                            }
                        } else {
                            perm = 0;
                        }
                    }
                }
                LocalFree(sd_ptr);
            }
        #else
            struct stat s;
            if(!stat(name.c_str(), &s)){
                perm =  (s.st_mode & S_IRUSR ? PERM_READABLE : 0)  |
                        (s.st_mode & S_IWUSR ? PERM_WRITABLE : 0)  |
                        (s.st_mode & S_IXUSR ? PERM_EXECUTABLE : 0);
            }
        #endif
    }

    bool File::can_read() noexcept{
        return perm & PERM_READABLE;
    }

    bool File::can_write() noexcept{
        return perm & PERM_WRITABLE;
    }

    bool File::can_execute() noexcept{
        return perm & PERM_EXECUTABLE;
    }

    bool File::valid_permissions() noexcept{
        return perm ^ PERM_INVALID;
    }

    bool File::make_file() noexcept {
        std::ofstream file(name.c_str(), std::ios::out | std::ios::trunc);
        return static_cast<bool>(file);
    }

    bool File::make_dir() noexcept {
        #ifdef _SM_OS_WINDOWS
            return CreateDirectoryW(wname.c_str(), NULL);
        #else
            return mkdir(name.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        #endif
    }

    bool File::last_modified(integer_t& out) noexcept {
        #ifdef _SM_OS_WINDOWS
            FILETIME ft, localFt;
            HANDLE handle = CreateFileW(wname.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
            if(handle == INVALID_HANDLE_VALUE || !GetFileTime(handle, NULL, NULL, &ft))
                return false;

            SYSTEMTIME st, local;
            FileTimeToSystemTime(&ft, &st);
            SystemTimeToTzSpecificLocalTime(NULL, &st, &local);
            SystemTimeToFileTime(&local, &localFt);

            out = getUnixTime(localFt);
            CloseHandle(handle);
        #else
            struct stat s;
            if(stat(name.c_str(), &s))
                return false;
            out = static_cast<integer_t>(s.st_mtime);
        #endif
        return true;
    }

    bool File::last_access(integer_t& out) noexcept {
        #ifdef _SM_OS_WINDOWS
            FILETIME ft, localFt;
            HANDLE handle = CreateFileW(wname.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
            if(handle == INVALID_HANDLE_VALUE || !GetFileTime(handle, NULL, &ft, NULL))
                return false;

            SYSTEMTIME st, local;
            FileTimeToSystemTime(&ft, &st);
            SystemTimeToTzSpecificLocalTime(NULL, &st, &local);
            SystemTimeToFileTime(&local, &localFt);

            out = getUnixTime(localFt);
            CloseHandle(handle);
        #else
            struct stat s;
            if(stat(name.c_str(), &s))
                return false;
            out = static_cast<integer_t>(s.st_atime);
        #endif
        return true;
    }

    bool File::is_empty(bool& out) noexcept {
        if(is_dir()){
            #ifdef _SM_OS_WINDOWS
                return PathIsDirectoryEmptyW(wname.c_str());
            #else
                DIR* dp = opendir(name.c_str());
                if(!dp)
                    return false;
                out = !readdir(dp);
                closedir(dp);
            #endif
        } else if(is_file()){
            std::ifstream is(name.c_str(), std::ios::in);
            if(!is)
                return false;
            out = is.peek() == EOF;
            return true;
        }
        return false;
    }

    bool File::size(size_t& size) noexcept {
        #ifdef _SM_OS_WINDOWS
            HANDLE handle = CreateFileW(wname.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
            LARGE_INTEGER sz;
            if(handle == INVALID_HANDLE_VALUE || !GetFileSizeEx(handle, &sz))
                return false;
            size = static_cast<size_t>(size);
        #else
            struct stat s;
            if(stat(name.c_str(), &s))
                return false;
            if(!S_ISREG(s.st_mode))
                return false;
            size = static_cast<size_t>(s.st_size);
        #endif
        return true;
    }

    Directory File::list_dir(std::string& file) noexcept{
        #ifdef _SM_OS_WINDOWS
            WIN32_FIND_DATAW data;
            std::wstring copy(wname);
            copy += L"\\*";

            HANDLE handle = FindFirstFileW(copy.c_str(), &data);
            if(handle == INVALID_HANDLE_VALUE)
                return Directory(false);

            int req_size = WideCharToMultiByte(CP_UTF8, 0, data.cFileName, -1, NULL, 0, NULL, NULL);
            file.resize(req_size);
            WideCharToMultiByte(CP_UTF8, 0, data.cFileName, -1, &file[0], file.size(), NULL, NULL);
            file.pop_back();
        #else
            DIR* handle = opendir(name.c_str());
            if(!handle)
                return Directory(false);
            struct dirent* e = readdir(handle);
            if(!e)
                return Directory(false);
            file = e->d_name;
        #endif

        Directory dir(true);
        dir.handle = handle;
        return dir;
    }

    std::string File::get_parent() noexcept {
        #ifdef _SM_OS_WINDOWS
            std::wstring copy(wname);
            PathRemoveFileSpecW(&copy[0]);

            int req_size = WideCharToMultiByte(CP_UTF8, 0, copy.c_str(), -1, NULL, 0, NULL, NULL);
            std::string parent(req_size-1, 0);
            WideCharToMultiByte(CP_UTF8, 0, copy.c_str(), -1, &parent[0], parent.size(), NULL, NULL);
            if(parent.empty())
                return ".";
            return parent;
        #else
            std::string copy(name);
            return std::string(dirname(&copy[0]));
        #endif
    }

    std::string File::get_name() noexcept{
        #ifdef _SM_OS_WINDOWS
            LPWSTR str = PathFindFileNameW(wname.c_str());
            int req_size = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);

            std::string file(req_size, 0);
            WideCharToMultiByte(CP_UTF8, 0, str, -1, &file[0], file.size(), NULL, NULL);
            return file;
        #else
            std::string copy(name);
            return std::string(basename(&copy[0]));
        #endif
    }

    bool File::full_path(std::string& out) noexcept{
        #ifdef _SM_OS_WINDOWS
            int size = GetFullPathNameW(wname.c_str(), 0, NULL, NULL);
            if(!size)
                return false;
            std::wstring str(size, 0);
            GetFullPathNameW(wname.c_str(), size, &str[0], NULL);

            int req_size = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
            std::string file(req_size, 0);
            WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &file[0], file.size(), NULL, NULL);
            out = std::move(file);
        #else
            char* str = realpath(name.c_str(), NULL);
            if(!str)
                return false;
            out = str;
            free(str);
        #endif
        return true;
    }

    std::string File::path() noexcept {
        return name;
    }

    #if fileSeparator == '/'
        const std::string& File::path(const std::string& str) noexcept{
            return str;
        }
    #else
        std::string File::path(std::string str) noexcept{
            std::string copy (std::move(str));
            std::replace(copy.begin(), copy.end(), '/', fileSeparator);
            return copy;
        }
    #endif
}
