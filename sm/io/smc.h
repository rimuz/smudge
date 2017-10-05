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
 *      File io/smc.h
 *
*/

#ifndef _SM__IO__SMC_H
#define _SM__IO__SMC_H

#include "sm/io/RW.h"
#include "sm/runtime/Object.h"
#include "sm/runtime/gc.h"
#include "sm/typedefs.h"

template <typename Tp>
sm::Writer<Tp>& operator<<(sm::Writer<Tp>&, sm::ByteCode_t&) noexcept;
template <typename Tp>
sm::Writer<Tp>& operator<<(sm::Writer<Tp>&, sm::runtime::Runtime_t&) noexcept;

namespace sm {
    namespace io {
        constexpr char magic [] = "\xC0\xDE\xC0\x00L";
        constexpr size_t magic_size = arraySize(magic);

        // Sz -> For serialize purposes
        using SzBool_t = uint8_t;
        using SzSize_t = uint32_t;

        template <typename Tp>
        void serialize(Writer<Tp>& out, Function& func, runtime::Runtime_t& rt) noexcept;
        template <typename Tp>
        void serialize(Writer<Tp>& out, Class& cl, runtime::Runtime_t& rt) noexcept;

        template <typename Tp>
        void serialize(Writer<Tp>& out, Function& f, runtime::Runtime_t& rt) noexcept{
            out.template write<uint8_t>(0xFF);
            if(f.flags & FF_NATIVE)
                rt.sources.msg(error::ET_FATAL_ERROR, "cannot generate "
                    "compiled file because function or method is "
                    "native (in non-native box) (err #5)");

            out.template write<uint32_t>(f.boxName);
            out.template write<uint32_t>(f.fnName);
            out.template write<SzSize_t>(f.address);
            out.template write<uint8_t>(f.flags);
            out.template write<SzSize_t>(f.arguments.size());

            for(auto& x : f.arguments){
                out.template write<uint32_t>(x.first);
                out.template write<SzSize_t>(x.second);
            }
        }

        template <typename Tp>
        void serialize(Writer<Tp>& out, Class& c, runtime::Runtime_t& rt) noexcept{
            out.template write<uint8_t>(0xFE);
            out.template write<SzSize_t>(c.objects.size());

            for(auto& x : c.objects){
                switch(x.second.type){
                    case ObjectType::FUNCTION:
                        out.template write<uint32_t>(x.first);
                        serialize(out, *x.second.f_ptr, rt);
                        break;

                    default:
                        rt.sources.msg(error::ET_FATAL_ERROR, "cannot generate "
                            "compiled file because class contains unsupported "
                            "objects (err #5)");
                }
            }
        }
    }
}

template <typename Tp>
sm::Writer<Tp>& operator<<(sm::Writer<Tp>& out, sm::ByteCode_t& code) noexcept{
    using namespace sm;
    using namespace sm::io;

    out.write(static_cast<SzSize_t>(code.size()));
    out.write_str(&code[0], code.size());
    return out;
}

template <typename Tp>
sm::Writer<Tp>& operator<<(sm::Writer<Tp>& out, sm::runtime::Runtime_t& rt) noexcept{
    using namespace sm;
    using namespace sm::io;

    uint16_t options =
        rt.noStd                |
        (rt.callInit << 1)      |
        (rt.callMain << 2)      |
        (rt.callNew << 3);


    // magic code
    out.write_str(magic, magic_size);

    // options
    out.write(options);

    // symbol tables
    out.template write<SzSize_t>(rt.nameIds.size());
    for(auto& x : rt.nameIds){
        out.write(x.second);
        out.template write<SzSize_t>(x.first.size());
        out.write_str(x.first.data(), x.first.size());
    }

    out.template write<SzSize_t>(rt.nameConstants.size());
    for(string_t& x : rt.nameConstants){
        out.template write<SzSize_t>(x.size());
        out.write_str(x.data(), x.size());
    }

    // box names
    out.template write<SzSize_t>(rt.boxNames.size());
    for(string_t& x : rt.boxNames){
        out.template write<SzSize_t>(x.size());
        out.write_str(x.data(), x.size());
    }

    // strings
    out.template write<SzSize_t>(rt.stringConstants.size());
    for(String& str : rt.stringConstants){
        out.template write<SzSize_t>(str.size());
        out.write_str(str.data(), str.size());
    }

    // ints
    out.template write<SzSize_t>(rt.intConstants.size());
    for(integer_t i : rt.intConstants)
        out.write(i);

    // floats
    out.template write<SzSize_t>(rt.floatConstants.size());
    for(float_t f : rt.floatConstants)
        out.write(f);

    // code
    out << rt.code;

    // boxes
    out.template write<SzSize_t>(rt.boxes.size());
    for(size_t i = 0; i != rt.boxes.size(); ++i){
        bool native = rt.boxNames[i].back() == '!';
        out.template write<uint8_t>(native);
        out.write_str(rt.boxNames[i].data(), rt.boxNames[i].size());

        if(!native){
            for(auto& x : rt.boxes[i]->objects){
                out.template write<uint32_t>(x.first);
                switch(x.second->type){
                    case ObjectType::FUNCTION:
                        serialize(out, *x.second->f_ptr, rt);
                        break;

                    case ObjectType::CLASS:
                        serialize(out, *x.second->c_ptr, rt);
                        break;

                    default:
                        rt.sources.msg(error::ET_FATAL_ERROR, "cannot generate "
                            "compiled file because box contains unsupported "
                            "objects (err #5)");
                }
            }
        }
    }
    return out;
}

#endif
