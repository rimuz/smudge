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
 *      File io/RW.h
 *
 *  This file provides some tools to safely write/read a file
 *  easily and without worrying about endianness.
 *
*/

#include <istream>
#include <ostream>
#include <ostream>
#include <algorithm>
#include <utility>

#include "sm/typedefs.h"

#ifndef _SM__IO__RW_H
#define _SM__IO__RW_H

namespace sm{
    bool isBigEndian() noexcept{
        static volatile uint32_t i = 0x01ABCDEF;
        static bool value = *reinterpret_cast<volatile byte_t*>(&i) == 0x01;
        return value;
    }

    template <typename _OStream>
    class Writer {
    private:
        _OStream owned;
        _OStream& os;
    public:
        template <typename... TArgs>
        Writer(TArgs&&... args) : owned(std::forward<TArgs>(args)...), os(owned){}
        Writer(_OStream& stream) : os(stream){}

        template <typename Tp>
        inline void write(const Tp& obj) {
            write_rev<sizeof(Tp)>(reinterpret_cast<const byte_t*>(&obj));
        }

        template <typename Tp>
        inline void write_str(const Tp* data, size_t size){
            static_assert(sizeof(Tp) == sizeof(byte_t), "String characters must have size of 1 byte.");
            os.write(reinterpret_cast<const char*>(data), size);
        }

        template <size_t size>
        inline void write_rev(const byte_t* data){
            if(isBigEndian()){
                os.write(reinterpret_cast<const char*>(data), size);
            } else {
                byte_t copy [size];
                std::copy (
                    std::reverse_iterator<const byte_t*> (data + size),
                    std::reverse_iterator<const byte_t*> (data),
                    copy
                );
                os.write(reinterpret_cast<const char*>(copy), size);
            }
        }
        _OStream& stream() const noexcept { return os; }
    };

    template <typename _IStream>
    class Reader {
    private:
        _IStream owned;
        _IStream& is;
    public:
        template <typename... TArgs>
        Reader(TArgs&&... args) : owned(std::forward<TArgs>(args)...), is(owned){}
        Reader(_IStream& stream) : is(stream){}

        template <typename Tp>
        Tp read(){
            Tp val{};
            read(val);
            return val;
        }

        template <typename Tp>
        inline void read(Tp& out) noexcept{
            read_rev<sizeof(Tp)>(reinterpret_cast<byte_t*>(&out));
        }

        template <typename Tp, typename Tp2>
        inline void read_cast(Tp2& out) noexcept{
            out = static_cast<Tp2>(read<Tp>());
        }

        inline void read_str(byte_t* data, size_t size) noexcept{
            is.read(reinterpret_cast<char*>(data), size);
        }

        template <size_t size>
        inline void read_rev(byte_t* data) noexcept{
            is.read(reinterpret_cast<char*>(data), size);
            if(!isBigEndian())
                std::reverse(data, data + size);
        }

        _IStream& stream() const noexcept { return is; }
    };

    template <typename Char>
    class PtrInputStream {
    private:
        Char* data;
    public:
        PtrInputStream(Char* ptr) : data(ptr){}

        void read(Char* out, size_t size){
            while(size--)
                *out++ = *data++;
        }
    };

    using StdWriter = Writer<std::ostream>;
    using StdReader = Writer<std::istream>;
    using FileWriter = Writer<std::ofstream>;
    using FileReader = Writer<std::ifstream>;
}

#endif
