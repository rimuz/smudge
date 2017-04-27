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
    namespace lib{
        extern oid_t idToString;
    }

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

        Object implicitToString(exec::Interpreter& intp, const Object& in){
            switch(in.type){
                case ObjectType::NONE:
                    return makeString("<null>");
                case ObjectType::INTEGER:
                    return makeString(std::to_string(in.i).c_str());

                case ObjectType::FLOAT: {
                    std::ostringstream oss;
                    oss << in.f;
                    return makeString(oss.str().c_str());
                }

                case ObjectType::STRING:
                    return in;

                case ObjectType::CLASS_INSTANCE:{
                    Object out;
                    if(runtime::find<ObjectType::CLASS_INSTANCE>(in, out, lib::idToString)){
                        Function* f_ptr;
                        if(runtime::callable(out, f_ptr)){
                            Object str = intp.callFunction(f_ptr, ObjectVec_t(), out, true);
                            if(str.type != ObjectType::STRING){
                                intp.rt->sources.printStackTrace(intp, error::ERROR,
                                    std::string("method 'to_string()' in ")
                                    + runtime::errorString(intp, out)
                                    + " didn't return a string");
                            }
                            return str;
                        } else {
                            intp.rt->sources.printStackTrace(intp, error::ERROR,
                                std::string("'to_string()' must be a function in ")
                                + runtime::errorString(intp, out));
                        }
                    }

                    std::ostringstream oss;
                    oss << "<instance of "
                        << _BoxName(intp.rt->boxNames[in.i_ptr->base->boxName]) << "::"
                        << intp.rt->nameFromId(in.i_ptr->base->name)
                        << " at 0x" << std::setw(8)
                        << std::setfill('0') << std::hex
                        << reinterpret_cast<size_t>(in.i_ptr) << ">";
                    return makeString(oss.str().c_str());
                }

                case ObjectType::ENUM:{
                    std::ostringstream oss;
                    oss << "<enum "
                        << _BoxName(intp.rt->boxNames[in.e_ptr->boxName]) << "::"
                        << intp.rt->nameFromId(in.e_ptr->name) << ">";
                    return makeString(oss.str().c_str());
                }

                case ObjectType::CLASS:{
                    std::ostringstream oss;
                    oss << "<class "
                        << _BoxName(intp.rt->boxNames[in.c_ptr->boxName]) << "::"
                        << intp.rt->nameFromId(in.c_ptr->name) << ">";
                    return makeString(oss.str().c_str());
                }

                case ObjectType::FUNCTION:{
                    std::ostringstream oss;
                    oss << "<function "
                        << _BoxName(intp.rt->boxNames[in.f_ptr->boxName]) << "::"
                        << intp.rt->nameFromId(in.f_ptr->fnName) << "()>";
                    return makeString(oss.str().c_str());
                }

                case ObjectType::BOX:{
                    std::ostringstream oss;
                    oss << "<box " << _BoxName(intp.rt->boxNames[in.c_ptr->boxName]) << ">";
                    return makeString(oss.str().c_str());
                }

                case ObjectType::WEAK_REFERENCE:
                case ObjectType::STRONG_REFERENCE: {
                    Object obj = in.refGet();
                    if(obj.type == ObjectType::CLASS_INSTANCE){
                        std::ostringstream oss;
                        oss << "<ref to object "
                            << _BoxName(intp.rt->boxNames[obj.i_ptr->base->boxName]) << "::"
                            << intp.rt->nameFromId(obj.i_ptr->base->name)
                            << " 0x" << std::setw(_SM_PTR_SIZE*2)
                            << std::setfill('0') << std::hex
                            << reinterpret_cast<size_t>(obj.i_ptr) << ">";
                        return makeString(oss.str().c_str());
                    }
                    return makeString("<reference>");
                }

                default:
                    return makeString("<unknown>");
            }
        }

        string_t errorString(exec::Interpreter& intp, const Object& in){
            if(in.type == ObjectType::STRING){
                return "<string>";
            } if(in.type == ObjectType::INTEGER){
                return "<int>";
            } if(in.type == ObjectType::FLOAT){
                return "<float>";
            } if(in.type == ObjectType::CLASS_INSTANCE){
                std::ostringstream oss;
                oss << "<instance of "
                    << _BoxName(intp.rt->boxNames[in.i_ptr->base->boxName]) << "::"
                    << intp.rt->nameFromId(in.i_ptr->base->name)
                    << " at 0x" << std::setw(8)
                    << std::setfill('0') << std::hex
                    << reinterpret_cast<size_t>(in.i_ptr) << ">";
                return oss.str();
            }
            Object out = implicitToString(intp, in);
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

                /*
                case ObjectType::CLASS_INSTANCE:{
                    if(runtime::find<ObjectType::CLASS_INSTANCE>(obj, obj, runtime::roundId)
                            && obj.type == ObjectType::FUNCTION){
                        out = obj.f_ptr;
                        return true;
                    }
                    return false;
                }
                */

                default:
                    return false;
            }
        }
    }
}
