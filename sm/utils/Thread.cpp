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
 *      File utils/Thread.cpp
 *
*/

#include "sm/utils/Thread.h"

namespace sm{
    Mutex::Mutex() noexcept{
        #ifdef _SM_OS_WINDOWS
        _handle = CreateMutex(nullptr, FALSE, nullptr);
        #else
        pthread_mutex_init(&_handle, nullptr);
        #endif
    }

    Mutex::Mutex(Mutex&& rhs) noexcept{
        _handle = rhs._handle;
        _busy = rhs._busy;
        valid = rhs.valid;

        rhs.valid = false;
    }

    bool Mutex::lock() noexcept{
        #ifdef _SM_OS_WINDOWS
        bool b = WaitForSingleObject(_handle, INFINITE) != WAIT_FAILED;
        #else
        bool b = !pthread_mutex_lock(&_handle);
        #endif
        if(b) _busy = true;
        return b;
    }

    bool Mutex::unlock() noexcept{
        _busy = false;
        #ifdef _SM_OS_WINDOWS
        return ReleaseMutex(_handle);
        #else
        return !pthread_mutex_unlock(&_handle);
        #endif
    }

    Mutex& Mutex::operator=(Mutex&& rhs) noexcept{
        #ifdef _SM_OS_WINDOWS
        CloseHandle(_handle);
        #else
        pthread_mutex_destroy(&_handle);
        #endif

        _handle = rhs._handle;
        _busy = rhs._busy;
        valid = rhs.valid;
        return *this;
    }

    Mutex::~Mutex(){
        #ifdef _SM_OS_WINDOWS
        CloseHandle(_handle);
        #else
        pthread_mutex_destroy(&_handle);
        #endif
    }

    Lock::Lock(Mutex& _m) noexcept{
        _m.lock();
        mutex = &_m;
    }

    Lock& Lock::operator=(Lock&& rhs) noexcept{
        mutex->unlock();
        mutex = rhs.mutex;
        return *this;
    }

    Lock::~Lock(){
        mutex->unlock();
    }
}
