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

#include "sm/exec/interpreter/defines.h"
#include "sm/compile/defs.h"
#include "sm/runtime/casts.h"
#include "sm/runtime/gc.h"
#include "sm/runtime/id.h"
#include "sm/typedefs.h"

namespace sm{
    namespace lib{
        extern Class* cString;
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
                    RootObject out;
                    if(runtime::find<ObjectType::CLASS_INSTANCE>(in, out, lib::idToString)){
                        Function* f_ptr;
                        RootObject self = in;
                        if(runtime::callable(out, self, f_ptr)){
                            RootObject str = intp.callFunction(f_ptr, {}, self, true);
                            if(str->type != ObjectType::STRING){
                                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                                    std::string("method 'to_string()' in ")
                                    + runtime::errorString(intp, self)
                                    + " didn't return a string");
                            }
                            return str;
                        } else {
                            intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                                std::string("'to_string()' must be a function in ")
                                + runtime::errorString(intp, in));
                        }
                    }

                    std::ostringstream oss;
                    oss << "<instance of "
                        << intp.rt->boxNames[in.i_ptr->base->boxName] << "::"
                        << intp.rt->nameFromId(in.i_ptr->base->name) << ">";
                    return makeString(oss.str().c_str());
                }

                case ObjectType::ENUM:{
                    std::ostringstream oss;
                    oss << "<enum "
                        << intp.rt->boxNames[in.e_ptr->boxName] << "::"
                        << intp.rt->nameFromId(in.e_ptr->name) << ">";
                    return makeString(oss.str().c_str());
                }

                case ObjectType::CLASS:{
                    std::ostringstream oss;
                    oss << "<class "
                        << intp.rt->boxNames[in.c_ptr->boxName] << "::"
                        << intp.rt->nameFromId(in.c_ptr->name) << ">";
                    return makeString(oss.str().c_str());
                }

                case ObjectType::FUNCTION:{
                    std::ostringstream oss;
                    oss << "<function "
                        << intp.rt->boxNames[in.f_ptr->boxName] << "::"
                        << intp.rt->nameFromId(in.f_ptr->fnName) << "()>";
                    return makeString(oss.str().c_str());
                }

                case ObjectType::BOX:{
                    std::ostringstream oss;
                    oss << "<box " << intp.rt->boxNames[in.b_ptr->name] << ">";
                    return makeString(oss.str().c_str());
                }

                case ObjectType::WEAK_REFERENCE:
                case ObjectType::STRONG_REFERENCE: {
                    RootObject obj = in.refGet();
                    if(obj->type == ObjectType::CLASS_INSTANCE){
                        std::ostringstream oss;
                        oss << "<ref to object "
                            << intp.rt->boxNames[obj->i_ptr->base->boxName] << "::"
                            << intp.rt->nameFromId(obj->i_ptr->base->name) << ">";
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
                    << intp.rt->boxNames[in.i_ptr->base->boxName] << "::"
                    << intp.rt->nameFromId(in.i_ptr->base->name) << ">";
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

        bool of_type(const Object& in, Class* type){
            return (in.type == ObjectType::CLASS_INSTANCE && in.i_ptr->base == type)
                || (in.type == ObjectType::STRING && type == lib::cString);
        }

        /* self: is an output parameter! */
        bool callable(const Object& in, Object& self, Function*& out){
            RootObject obj = in;
            _OcValue(obj);

            while (obj->type == ObjectType::METHOD){
                self = obj->m_ptr->self;
                obj = *obj->m_ptr->func_ptr;
            }

            switch(obj->type){
                case ObjectType::FUNCTION:
                    out = obj->f_ptr;
                    return true;

                case ObjectType::BOX:{
                    if(runtime::find<ObjectType::BOX>(obj, obj, runtime::roundId)
                            && obj->type == ObjectType::FUNCTION){
                        out = obj->f_ptr;
                        return true;
                    }
                    return false;
                }

                case ObjectType::CLASS: {
                    self = nullptr;
                    self.type = ObjectType::INSTANCE_CREATOR;
                    self.c_ptr = obj->c_ptr;
                    return true;
                }

                case ObjectType::CLASS_INSTANCE:{
                    RootObject func;
                    if(runtime::find<ObjectType::CLASS_INSTANCE>(obj, func, runtime::roundId)
                            && func->type == ObjectType::FUNCTION){
                        out = func->f_ptr;
                        self = std::move(obj);
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
