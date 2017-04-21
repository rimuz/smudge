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
 *      File runtime/casts.cpp
 *
*/

#include <sstream>
#include <iomanip>

#include "runtime/casts.h"
#include "runtime/gc.h"
#include "runtime/id.h"
#include "exec/interpreter/defines.h"
#include "typedefs.h"

#define _BoxName(Box) (Box.back() == '!' ? Box.substr(0, Box.size()-1) : Box)

namespace sm{
    namespace runtime{
        bool implicitToInt(const Object& in, integer_t& out){
            switch(in.type){
                case ObjectType::INTEGER:
                    out = in.i;
                    return true;
                case ObjectType::FLOAT:
                    out = static_cast<integer_t>(in.f);
                    return true;
                default:
                    return false;
            }
        }

        bool implicitToInt(const Object& in, Object& out){
            out = nullptr;
            out.type = ObjectType::INTEGER;
            return implicitToInt(in, out.i);
        }

        bool implicitToFloat(const Object& in, float_t& out){
            switch(in.type){
                case ObjectType::INTEGER:
                    out = static_cast<float_t>(in.i);
                    return true;
                case ObjectType::FLOAT:
                    out = in.f;
                    return true;
                default:
                    return false;
            }
        }

        bool implicitToFloat(const Object& in, Object& out){
            out = nullptr;
            out.type = ObjectType::FLOAT;
            return implicitToFloat(in, out.f);
        }

        bool implicitToString(const Runtime_t& rt, const Object& in, Object& out){
            switch(in.type){
                case ObjectType::NONE:
                    out = makeString("<null>");
                    return true;
                case ObjectType::INTEGER:
                    out = makeString(std::to_string(in.i).c_str());
                    return true;
                case ObjectType::FLOAT: {
                    std::ostringstream oss;
                    oss << in.f;
                    out = makeString(oss.str().c_str());
                    return true;
                }
                case ObjectType::STRING:
                    out = in;
                    return true;

                case ObjectType::CLASS_INSTANCE:{
                    std::ostringstream oss;
                    oss << "<instance of "
                        << _BoxName(rt.boxNames[in.i_ptr->base->boxName]) << "::"
                        << rt.nameFromId(in.i_ptr->base->name)
                        << " at 0x" << std::setw(8)
                        << std::setfill('0') << std::hex
                        << reinterpret_cast<size_t>(in.i_ptr) << ">";
                    out = makeString(oss.str().c_str());
                    return true;
                }

                case ObjectType::ENUM:{
                    std::ostringstream oss;
                    oss << "<enum "
                        << _BoxName(rt.boxNames[in.e_ptr->boxName]) << "::"
                        << rt.nameFromId(in.e_ptr->name) << ">";
                    out = makeString(oss.str().c_str());
                    return true;
                }

                case ObjectType::CLASS:{
                    std::ostringstream oss;
                    oss << "<class "
                        << _BoxName(rt.boxNames[in.c_ptr->boxName]) << "::"
                        << rt.nameFromId(in.c_ptr->name) << ">";
                    out = makeString(oss.str().c_str());
                    return true;
                }

                case ObjectType::FUNCTION:{
                    std::ostringstream oss;
                    oss << "<function "
                        << _BoxName(rt.boxNames[in.f_ptr->boxName]) << "::"
                        << rt.nameFromId(in.f_ptr->fnName) << "()>";
                    out = makeString(oss.str().c_str());
                    return true;
                }

                case ObjectType::BOX:{
                    std::ostringstream oss;
                    oss << "<box " << _BoxName(rt.boxNames[in.c_ptr->boxName]) << ">";
                    out = makeString(oss.str().c_str());
                    return true;
                }

                case ObjectType::WEAK_REFERENCE:
                case ObjectType::STRONG_REFERENCE: {
                    Object obj = in.refGet();
                    if(obj.type == ObjectType::CLASS_INSTANCE){
                        std::ostringstream oss;
                        oss << "<ref to object "
                            << _BoxName(rt.boxNames[obj.i_ptr->base->boxName]) << "::"
                            << rt.nameFromId(obj.i_ptr->base->name)
                            << " 0x" << std::setw(_SM_PTR_SIZE*2)
                            << std::setfill('0') << std::hex
                            << reinterpret_cast<size_t>(obj.i_ptr) << ">";
                        out = makeString(oss.str().c_str());
                    } else {
                        out = makeString("<reference>");
                    }
                    return true;
                }

                default:
                    out = makeString("<unknown>");
                    return false;
            }
        }

        string_t errorString(const Runtime_t& rt, const Object& in){
            Object out;
            if(in.type == ObjectType::STRING){
                return "<string>";
            } if(in.type == ObjectType::INTEGER){
                return "<int>";
            } if(in.type == ObjectType::FLOAT){
                return "<float>";
            } else if(!implicitToString(rt, in, out)){
                return "<unknown>";
            }
            return string_t(out.s_ptr->str.begin(), out.s_ptr->str.end());
        }

        bool implicitToBool(const Object& in){
            if(in.type == ObjectType::NONE){
                return false;
            } else if(in.type == ObjectType::INTEGER || in.type == ObjectType::FLOAT){
                return in.i != 0;
            }
            return true;
        }

        bool callable(const Object& in, Function*& out){
            Object obj = in;
            _OcValue(obj);

            switch(obj.type){
                case ObjectType::FUNCTION:
                    out = obj.f_ptr;
                    return true;

                case ObjectType::BOX:{
                    if(runtime::find<ObjectType::BOX>(obj, obj, runtime::roundId)
                            && obj.type == ObjectType::FUNCTION){
                        out = obj.f_ptr;
                        return true;
                    }
                    return false;
                }

                case ObjectType::CLASS_INSTANCE:{
                    if(runtime::find<ObjectType::CLASS_INSTANCE>(obj, obj, runtime::roundId)
                            && obj.type == ObjectType::FUNCTION){
                        out = obj.f_ptr;
                        return true;
                    }
                    return false;
                }

                default:
                    return false;
            }
        }
    }
}
