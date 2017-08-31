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
 *      File utils/Thread.h
 *
*/

#ifndef _SM__UTILS__THREAD_H
#define _SM__UTILS__THREAD_H

#include "sm/typedefs.h"
#ifdef _SM_OS_WINDOWS
#include <utility>
#else
#include <pthread.h>
#include <signal.h>
#endif

namespace sm{
    namespace thread{
        #ifdef _SM_OS_WINDOWS
        template <typename TData>
        DWORD WINAPI wrapper (LPVOID arg);
        #else
        template <typename TData>
        void* wrapper (void* arg);
        #endif
    }

    template <typename TData>
    class Thread {
    private:
        bool _initialized;
    public:
        #ifdef _SM_OS_WINDOWS
        HANDLE handle;
        #else
        pthread_t handle;
        #endif
        TData* data;
        uint32_t (*func) (TData&);

        template <typename... Tp>
        Thread(uint32_t (*_func) (TData&), Tp&&... args) noexcept
            : _initialized(false), data(new TData(std::forward<Tp>(args)...)), func(_func) {}
        Thread(const Thread<TData>&) = delete;
        Thread(Thread<TData>&&) noexcept;

        #ifdef _SM_OS_WINDOWS
        Thread(HANDLE _handle)
            : _initialized(true), handle(_handle), data(nullptr) {}
        #else
        Thread(pthread_t _handle)
            : _initialized(true), handle(_handle), data(nullptr) {}
        #endif

        bool start() noexcept;
        bool join(uint32_t& ret) noexcept;
        bool kill() noexcept;
        bool is_running() noexcept;

        inline bool was_started() const noexcept{
            return _initialized;
        }

        static Thread<TData> current() noexcept;

        Thread<TData>& operator=(const Thread<TData>&) = delete;
        Thread<TData>& operator=(Thread<TData>&&) noexcept;
        ~Thread() noexcept;
    };

    class Mutex {
    private:
        #ifdef _SM_OS_WINDOWS
        HANDLE _handle;
        #else
        pthread_mutex_t _handle;
        #endif
        bool _busy = false;
        bool valid = true;
    public:
        Mutex() noexcept;
        Mutex(const Mutex&) = delete;
        Mutex(Mutex&&) noexcept;

        bool lock() noexcept;
        bool unlock() noexcept;
        inline bool is_locked() const noexcept;

        Mutex& operator=(const Mutex&) = delete;
        Mutex& operator=(Mutex&&) noexcept;
        ~Mutex();
    };

    class Lock {
    private:
        Mutex* mutex;
    public:
        Lock(Mutex& _m)  noexcept;

        Lock(const Lock&) = delete;
        Lock(Lock&&) = default;
        Lock& operator=(const Lock&) = delete;
        Lock& operator=(Lock&&) noexcept;

        ~Lock();
    };

    template <typename Tp>
    bool Thread<Tp>::start() noexcept {
        if(_initialized)
            return false;
        _initialized = true;

        #ifdef _SM_OS_WINDOWS
        handle = CreateThread(nullptr, 0, &thread::wrapper<Tp>, this, 0, nullptr);
        return handle;
        #else
        return !pthread_create(&handle, nullptr, &thread::wrapper<Tp>, this);
        #endif
    }

    template <typename Tp>
    bool Thread<Tp>::join(uint32_t& ret) noexcept{
        if(!_initialized)
            return false;
        #ifdef _SM_OS_WINDOWS
        DWORD code = WaitForSingleObject(handle, INFINITE);
        GetExitCodeThread(handle, reinterpret_cast<LPDWORD>(&ret));
        return code != WAIT_FAILED;
        #else
        return !pthread_join(handle, reinterpret_cast<void**>(&ret));
        #endif
    }

    template <typename Tp>
    bool Thread<Tp>::kill() noexcept {
        if(!_initialized)
            return false;
        #ifdef _SM_OS_WINDOWS
        return TerminateThread(handle, 1);
        #else
        return !pthread_cancel(handle);
        #endif
    }

    template <typename Tp>
    Thread<Tp> Thread<Tp>::current() noexcept {
       #ifdef _SM_OS_WINDOWS
       return Thread<Tp>(GetCurrentThread());
       #else
       return Thread<Tp>(pthread_self());
       #endif
    }

    template <typename Tp>
    bool Thread<Tp>::is_running() noexcept {
       #ifdef _SM_OS_WINDOWS
       DWORD i;
       GetExitCodeThread(handle, &i);
       return i == STILL_ACTIVE;
       #else
       return pthread_kill(handle, 0) == 0;
       #endif
    }

    template <typename Tp>
    Thread<Tp>::Thread(Thread<Tp>&& th) noexcept {
        _initialized = th._initialized;
        handle = th.handle;
        data = th.data;
        func = th.func;

        th._initialized = false;
        th.data = nullptr;
    }

    template <typename Tp>
    Thread<Tp>& Thread<Tp>::operator=(Thread<Tp>&& rhs) noexcept {
        #ifdef _SM_OS_WINDOWS
        if(_initialized){
            CloseHandle(handle);
        }
        #endif

        if(data != nullptr)
            delete data;

        _initialized = rhs._initialized;
        handle = rhs.handle;
        data = rhs.data;
        func = rhs.func;

        rhs._initialized = false;
        rhs.data = nullptr;
    }

    template <typename Tp>
    Thread<Tp>::~Thread() noexcept {
        #ifdef _SM_OS_WINDOWS
        if(_initialized){
            CloseHandle(handle);
        }
        #endif

        if(data != nullptr)
            delete data;
    }

    inline bool Mutex::is_locked() const noexcept{
       return _busy;
    }

    namespace thread {
        #ifdef _SM_OS_WINDOWS
        template <typename TData>
        DWORD WINAPI wrapper (LPVOID arg){
        #else
        template <typename TData>
        void* wrapper (void* arg){
        #endif
            Thread<TData>* ptr = static_cast<Thread<TData>*>(arg);
            uint32_t ret = ptr->func(*ptr->data);
            #ifdef _SM_OS_WINDOWS
            return ret;
            #else
            return reinterpret_cast<void*>(ret);
            #endif
        }
    }
}

#endif
