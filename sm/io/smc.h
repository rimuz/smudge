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


#include "sm/runtime/Object.h"
#include "sm/runtime/gc.h"

#include "sm/compile/v1/Compiler.h"
#include "sm/lib/stdlib.h"
#include "sm/typedefs.h"
#include "sm/io/RW.h"

template <typename Tp>
sm::Writer<Tp>& operator<<(sm::Writer<Tp>&, sm::ByteCode_t&) noexcept;
template <typename Tp>
sm::Writer<Tp>& operator<<(sm::Writer<Tp>&, sm::runtime::Runtime_t&) noexcept;

template <typename Tp>
sm::Reader<Tp>& operator>>(sm::Reader<Tp>&, sm::ByteCode_t&) noexcept;
template <typename Tp>
sm::Reader<Tp>& operator>>(sm::Reader<Tp>&, sm::runtime::Runtime_t&) noexcept;

namespace sm {
    namespace io {
        constexpr char magic [] = "\xC0\xDE\xC0\x00L";
        constexpr size_t magic_size = arraySize(magic);
        constexpr uint32_t format_version = 1;

        // Sz -> For serialize purposes
        using SzBool_t = uint8_t;
        using SzSize_t = uint32_t;

        template <typename Tp>
        void serialize(Writer<Tp>& out, Function&, runtime::Runtime_t&) noexcept;
        template <typename Tp>
        void serialize(Writer<Tp>& out, Class&, runtime::Runtime_t&) noexcept;

        template <typename Tp>
        void deserialize(Reader<Tp>& in, Function&, runtime::Runtime_t&) noexcept;
        template <typename Tp>
        void deserialize(Reader<Tp>& in, Class&, runtime::Runtime_t&) noexcept;

        template <typename Tp>
        void serialize(Writer<Tp>& out, Function& f, runtime::Runtime_t& rt) noexcept{
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
        void deserialize(Reader<Tp>& in, Function& f, runtime::Runtime_t& rt) noexcept{
            in.template read_cast<uint32_t>(f.boxName);
            in.template read_cast<uint32_t>(f.fnName);
            in.template read_cast<SzSize_t>(f.address);
            in.template read_cast<uint8_t>(f.flags);
            SzSize_t size = in.template read<SzSize_t>();

            f.arguments.reserve(size);
            while(size--){
                uint32_t first = in.template read<uint32_t>();
                SzSize_t second = in.template read<SzSize_t>();
                f.arguments.emplace_back(first, second);
            }
        }

        template <typename Tp>
        void serialize(Writer<Tp>& out, Class& c, runtime::Runtime_t& rt) noexcept{
            out.template write<SzSize_t>(c.objects.size());

            for(auto& x : c.objects){
                out.template write<uint32_t>(x.first);
                switch(x.second.type){
                    case ObjectType::FUNCTION:
                        out.template write<uint8_t>(0xFF);
                        serialize(out, *x.second.f_ptr, rt);
                        break;

                    default:
                        rt.sources.msg(error::ET_FATAL_ERROR, "cannot generate "
                            "compiled file because class contains  an unsupported "
                            "object (err #6)");
                }
            }
        }

        template <typename Tp>
        void deserialize(Reader<Tp>& in, Class& c, runtime::Runtime_t& rt) noexcept{
            SzSize_t size = in.template read<SzSize_t>();

            while(size--){
                uint32_t id = in.template read<uint32_t>();
                if(in.template read<uint8_t>() != 0xFF)
                    rt.sources.msg(error::ET_FATAL_ERROR,
                        "unsupported object type inside class (err #7)");

                Object func(ObjectType::FUNCTION);
                func.f_ptr = new Function();
                deserialize(in, *func.f_ptr, rt);
                c.objects.emplace(id, std::move(func));
            }
        }
    }
}

template <typename Tp>
sm::Writer<Tp>& operator<<(sm::Writer<Tp>& out, sm::ByteCode_t& code) noexcept{
    using namespace sm;
    using namespace sm::io;

    out.template write<SzSize_t>(code.size());
    out.write_str(&code[0], code.size());
    return out;
}

template <typename Tp>
sm::Reader<Tp>& operator>>(sm::Reader<Tp>& in, sm::ByteCode_t& code) noexcept{
    using namespace sm;
    using namespace sm::io;

    SzSize_t size = in.template read<SzSize_t>();
    code.resize(size);
    in.read_str(&code[0], size);
    return in;
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

    // version
    out.template write<uint32_t>(format_version);

    // options
    out.write(options);

    // symbol tables
    out.template write<SzSize_t>(rt.nameIds.size());
    for(auto& x : rt.nameIds){
        out.template write<uint16_t>(x.second);
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
        out.template write<uint32_t>(i);

        if(!native){
            out.template write<SzSize_t>(rt.boxes[i]->objects.size());
            for(auto& x : rt.boxes[i]->objects){
                out.template write<uint32_t>(x.first);
                switch(x.second->type){
                    case ObjectType::FUNCTION:
                        out.template write<uint8_t>(0xFF);
                        serialize(out, *x.second->f_ptr, rt);
                        break;

                    case ObjectType::CLASS:
                        out.template write<uint8_t>(0xFE);
                        serialize(out, *x.second->c_ptr, rt);
                        break;

                    default:
                        rt.sources.msg(error::ET_FATAL_ERROR, "cannot generate "
                            "compiled file because box contains unsupported "
                            "objects (err #8)");
                }
            }
        }
    }
    return out;
}


template <typename Tp>
sm::Reader<Tp>& operator>>(sm::Reader<Tp>& in, sm::runtime::Runtime_t& rt) noexcept{
    using namespace sm;
    using namespace sm::io;

    // magic code
    {
        char preamble [magic_size];
        in.read_str(reinterpret_cast<byte_t*>(preamble), magic_size);
        if(!std::equal(preamble, preamble + magic_size, magic))
            rt.sources.msg(error::ET_FATAL_ERROR,
                "unsupported file format (err #9)");
    }

    // version
    {
        uint32_t v = in.template read<uint32_t>();
        if(v != format_version)
            rt.sources.msg(error::ET_FATAL_ERROR,
                std::string("unsupported bytecode version [required: ")
                + std::to_string(v) + ", supported: "
                + std::to_string(format_version) + "] (err #10)");
    }

    // options
    {
        uint16_t options = in.template read<uint16_t>();
        rt.noStd        = options & 0x1;
        rt.callInit     = options & 0x2;
        rt.callMain     = options & 0x4;
        rt.callNew      = options & 0x8;
    }

    // symbol tables
    {
        SzSize_t size = in.template read<SzSize_t>();
        while(size--){
            uint16_t id = in.template read<uint16_t>();
            SzSize_t strSize = in.template read<SzSize_t>();

            string_t str;
            str.resize(strSize);
            in.read_str(reinterpret_cast<byte_t*>(&str[0]), strSize);
            rt.nameIds.emplace(std::move(str), id);
        }
    }

    {
        SzSize_t size = in.template read<SzSize_t>();
        rt.nameConstants.reserve(size);
        while(size--){
            SzSize_t strSize = in.template read<SzSize_t>();
            string_t str;

            str.resize(strSize);
            in.read_str(reinterpret_cast<byte_t*>(&str[0]), strSize);
            rt.nameConstants.emplace_back(std::move(str));
        }
    }

    // box names
    {
        SzSize_t size = in.template read<SzSize_t>();
        rt.boxNames.reserve(size);
        while(size--){
            SzSize_t strSize = in.template read<SzSize_t>();
            string_t str;
            str.resize(strSize);
            in.read_str(reinterpret_cast<byte_t*>(&str[0]), strSize);
            rt.boxNames.emplace_back(std::move(str));
        }
    }

    // strings
    {
        SzSize_t size = in.template read<SzSize_t>();
        rt.stringConstants.reserve(size);
        while(size--){
            SzSize_t strSize = in.template read<SzSize_t>();
            String str;
            str.resize(strSize);
            in.read_str(reinterpret_cast<byte_t*>(&str[0]), strSize);
            rt.stringConstants.emplace_back(std::move(str));
        }
    }

    // ints
    {
        SzSize_t size = in.template read<SzSize_t>();
        rt.intConstants.reserve(size);
        while(size--)
            rt.intConstants.emplace_back(in.template read<integer_t>());
    }

    // floats
    {
        SzSize_t size = in.template read<SzSize_t>();
        rt.floatConstants.reserve(size);
        while(size--)
            rt.floatConstants.emplace_back(in.template read<float_t>());
    }

    // code
    in >> rt.code;

    // boxes
    {
        SzSize_t size = in.template read<SzSize_t>();
        rt.boxes.reserve(size);
        while(size--){
            uint8_t isNative = in.template read<uint8_t>();
            uint32_t id = in.template read<uint32_t>();

            if(isNative){
                string_t name = rt.boxNames[id];
                std::replace(name.begin(), name.end(), '.', fileSeparator);
                name.pop_back();
                bool found = false;

                for(const string_t& dir : rt.paths){
                    std::string path = dir + name + _SM_DL_EXT;
                    Box* box;
                    if(compile::v1::Compiler::load_native(path.c_str(), rt, id, box)){
                        if(!box)
                            rt.sources.msg(error::ET_ERROR,
                                std::string("dynamic library '") + path
                                + "' is not a Smudge native box.");
                        rt.boxes.push_back(box);
                        found = true;
                        break;
                    }
                }

                if(!found){
                    string_t& boxName = rt.boxNames[id];
                    lib::LibDict_t::const_iterator cit = lib::libs.find(boxName);
                    if(cit == lib::libs.end() || rt.noStd){
                        rt.sources.msg(error::ET_ERROR,
                            std::string("can't import '") + boxName
                            + "'. Make sure the library exists.");
                    } else {
                        Box* box = cit->second(rt, id);
                        rt.boxes.push_back(box);
                    }
                }
            } else {
                Box* b = new Box;
                SzSize_t size = in.template read<SzSize_t>();

                b->name = id;
                rt.boxes.emplace_back(b);

                while(size--){
                    uint32_t id = in.template read<uint32_t>();
                    Object obj;

                    switch(in.template read<uint8_t>()){
                        case 0xFF:
                            obj.type = ObjectType::FUNCTION;
                            obj.f_ptr = new Function();
                            deserialize(in, *obj.f_ptr, rt);
                            break;

                        case 0xFE:
                            obj.type = ObjectType::CLASS;
                            obj.c_ptr = new Class();
                            deserialize(in, *obj.c_ptr, rt);
                            break;

                        default:
                            rt.sources.msg(error::ET_FATAL_ERROR,
                                "unknown object type inside class (err #11)");
                    }

                    b->objects.emplace(id, std::move(obj));
                }
            }
        }
    }
    return in;
}

#endif
