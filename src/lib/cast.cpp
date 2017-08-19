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
 *      File lib/cast.cpp
 *
*/

#include <cstdlib>
#include "lib/stdlib.h"
#include "runtime/casts.h"
#include "utils/unicode/utf8.h"

namespace sm{
    namespace lib{

        extern Class* cString;

        smLibDecl(cast){
            smInitBox

            smFunc(int, smLambda {
                if(args.empty())
                    return Object();
                const Object& obj = args[0];
                switch(obj.type){
                    case ObjectType::INTEGER:
                        return obj;
                    case ObjectType::FLOAT:
                        return makeInteger(obj.f);
                    case ObjectType::STRING: {
                        std::string str (obj.s_ptr->str.begin(), obj.s_ptr->str.end());
                        return makeInteger(std::atol(str.c_str()));
                    }
                }
                return Object();
            })

            smFunc(float, smLambda {
                if(args.empty())
                    return Object();
                const Object& obj = args[0];
                switch(obj.type){
                    case ObjectType::INTEGER:
                        return makeFloat(obj.i);
                    case ObjectType::FLOAT:
                        return obj;
                    case ObjectType::STRING: {
                        std::string str (obj.s_ptr->str.begin(), obj.s_ptr->str.end());
                        return makeFloat(std::atof(str.c_str()));
                    }
                }
                return Object();
            })

            smFunc(string, smLambda {
                if(args.empty())
                    return makeString("<null>");
                return runtime::implicitToString(intp, args[0]);
            })

            smFunc(uchar, smLambda {
                if(args.empty() || args[0].type != ObjectType::INTEGER)
                    return Object();
                unicode_t cp = args[0].i;
                unicode_t ch = uByCodepoint(cp);
                if(ch == UTF8_ERROR)
                    return Object();
                return makeString(ch);
            })

            smFunc(ucode, smLambda {
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return Object();
                String& str = args[0].s_ptr->str;
                if(str.empty())
                    return Object();
                unicode_t ch = str.uCharAt(0);
                unicode_t cp = uGetCodepoint(ch);
                if(cp == UTF8_ERROR)
                    return Object();
                return makeInteger(cp);
            })

            smFunc(value, smLambda {
                return args.empty() ? Object() : args[0];
            })

            smFunc(is_int, smLambda {
                return makeBool(!args.empty() && args[0].type == ObjectType::INTEGER);
            })

            smFunc(is_float, smLambda {
                return makeBool(!args.empty() && args[0].type == ObjectType::FLOAT);
            })

            smFunc(is_string, smLambda {
                return makeBool(!args.empty() && args[0].type == ObjectType::STRING);
            })

            smFunc(typeof, smLambda {
                if(args.empty())
                    return makeString("n");

                std::string str;
                const Object* obj = &args[0];

                Repeat:
                switch(obj->type){
                    case ObjectType::NONE:
                        str.push_back('n');
                        break;
                    case ObjectType::INTEGER:
                        str.push_back('i');
                        break;
                    case ObjectType::FLOAT:
                        str.push_back('f');
                        break;
                    case ObjectType::STRING:
                        str.push_back('s');
                        break;
                    case ObjectType::CLASS_INSTANCE:
                        str.push_back('o');
                        break;
                    case ObjectType::ENUM:
                        str.push_back('e');
                        break;
                    case ObjectType::CLASS:
                        str.push_back('c');
                        break;
                    case ObjectType::FUNCTION:
                        str.push_back('F');
                        break;
                    case ObjectType::METHOD:
                        str.push_back('m');
                        break;
                    case ObjectType::BOX:
                        str.push_back('b');
                        break;
                    case ObjectType::WEAK_REFERENCE:
                        obj = obj->o_ptr;
                        str.push_back('r');
                        goto Repeat;
                    case ObjectType::INSTANCE_CREATOR:
                        str.push_back('I');
                        break;
                    case ObjectType::NATIVE_DATA:
                        str.push_back('d');
                        break;
                    default:
                        str.push_back('u');
                        break;
                }
                return makeString(str.begin(), str.end());
            })

            smFunc(classof, smLambda {
                if(args.empty())
                    return Object();
                else if(args[0].type == ObjectType::CLASS)
                    return args[0];
                else if(args[0].type == ObjectType::STRING){
                    Object ret(ObjectType::CLASS);
                    ret.c_ptr = cString;
                    return ret;
                } else if(args[0].type == ObjectType::CLASS_INSTANCE){
                    Object ret(ObjectType::CLASS);
                    ret.c_ptr = args[0].i_ptr->base;
                    return ret;
                }
                return Object();
            })

            smFunc(baseof, smLambda {
                if(args.empty() || args[0].type != ObjectType::CLASS)
                    return Object();

                size_t idx = 0;
                if(args.size() > 1){
                    if(args[0].type != ObjectType::INTEGER)
                        return Object();
                    idx = args[0].i;
                }

                if(args[0].c_ptr->bases.size() <= idx)
                    return Object();
                Object ret(ObjectType::CLASS);
                ret.c_ptr = args[0].c_ptr->bases[idx];
                return ret;
            })

            smFunc(same, smLambda {
                Object a, b;
                if(!args.empty()){
                    a = args[0];
                    if(args.size() != 1)
                        b = args[1];
                    if(a.type == ObjectType::WEAK_REFERENCE)
                        a = *a.o_ptr;
                    if(b.type == ObjectType::WEAK_REFERENCE)
                        b = *b.o_ptr;

                }

                switch(a.type){
                    case ObjectType::NONE:
                        return makeBool(b.type == ObjectType::NONE);
                    case ObjectType::INTEGER:
                        return makeBool(b.type == ObjectType::INTEGER && a.i == b.i);
                    case ObjectType::FLOAT:
                        return makeBool(b.type == ObjectType::FLOAT && a.f == b.f);
                    case ObjectType::STRING:
                        return makeBool(b.type == ObjectType::STRING && a.s_ptr == b.s_ptr);
                    case ObjectType::CLASS_INSTANCE:
                        return makeBool(b.type == ObjectType::CLASS_INSTANCE && a.o_ptr == b.o_ptr);
                    case ObjectType::ENUM:
                        return makeBool(b.type == ObjectType::ENUM && a.e_ptr == b.e_ptr);
                    case ObjectType::CLASS:
                        return makeBool(b.type == ObjectType::CLASS && a.c_ptr == b.c_ptr);
                    case ObjectType::FUNCTION:
                        return makeBool(b.type == ObjectType::FUNCTION && a.f_ptr == b.f_ptr);
                    case ObjectType::METHOD:
                        return makeBool(b.type == ObjectType::METHOD && a.m_ptr == b.m_ptr);
                    case ObjectType::BOX:
                        return makeBool(b.type == ObjectType::BOX && a.c_ptr == b.c_ptr);
                    case ObjectType::INSTANCE_CREATOR:
                        return makeBool(b.type == ObjectType::INSTANCE_CREATOR && a.c_ptr == b.c_ptr);
                    case ObjectType::NATIVE_DATA:
                        return makeBool(b.type == ObjectType::NATIVE_DATA && a.ptr == b.ptr);
                }
                return Object();
            })

            smFunc(kin, smLambda {
                if(args.empty())
                    return makeTrue();

                if(args.size() == 1)
                    return args[0].type == ObjectType::NONE ? makeTrue() : makeFalse();

                Class* cl;

                Object a = args[0], b = args[1];

                if(a.type == ObjectType::WEAK_REFERENCE)
                    a = *a.o_ptr;
                if(b.type == ObjectType::WEAK_REFERENCE)
                    b = *b.o_ptr;

                switch(a.type){
                    case ObjectType::NONE:
                        return makeBool(b.type == ObjectType::NONE);
                    case ObjectType::INTEGER:
                        return makeBool(b.type == ObjectType::INTEGER);
                    case ObjectType::FLOAT:
                        return makeBool(b.type == ObjectType::FLOAT);
                    case ObjectType::STRING:
                        cl = cString;
                        break;
                    case ObjectType::CLASS_INSTANCE:
                        cl = a.i_ptr->base;
                        break;
                    case ObjectType::ENUM:
                        return makeBool(b.type == ObjectType::ENUM);
                    case ObjectType::CLASS:
                        cl = a.c_ptr;
                        break;
                    case ObjectType::FUNCTION:
                        return makeBool(b.type == ObjectType::FUNCTION);
                    case ObjectType::METHOD:
                        return makeBool(b.type == ObjectType::METHOD);
                    case ObjectType::BOX:
                        return makeBool(b.type == ObjectType::BOX);
                    case ObjectType::INSTANCE_CREATOR:
                        return makeBool(b.type == ObjectType::INSTANCE_CREATOR);
                    case ObjectType::NATIVE_DATA:
                        return makeBool(b.type == ObjectType::NATIVE_DATA);
                    default:
                        return Object();
                }

                Class* cl2;

                switch(b.type){
                    case ObjectType::CLASS_INSTANCE:
                        cl2 = b.i_ptr->base;
                        break;
                    case ObjectType::CLASS:
                        cl2 = b.c_ptr;
                        break;
                    case ObjectType::STRING:
                        cl2 = cString;
                        break;
                    default:
                        return makeFalse();
                }

                std::vector<Class*> to_check {cl2};
                while(!to_check.empty()){
                    Class* base = to_check.back();
                    to_check.pop_back();
                    if(base == cl)
                        return makeTrue();
                    to_check.insert(to_check.end(), base->bases.rbegin(), base->bases.rend());
                }

                return makeFalse();
            })

            smReturnBox
        }
    }
}
