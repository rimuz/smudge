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
 *      File runtime/id.cpp
 *
*/

#include "runtime/id.h"

namespace sm{
    namespace lib {
        extern Class* cString;
    }

    namespace runtime{
        unsigned genOrdinaryId(Runtime_t& rt, const string_t& name){
            compile::NamesMap_t::const_iterator found = rt.nameIds.find(name);
            if(found == rt.nameIds.end()){
                rt.nameConstants.push_back(name);
                return runtime::idsStart + (rt.nameIds[name] = rt.nameConstants.size() -1);
            } else {
                return runtime::idsStart + found->second;
            }
        }

        bool find_any(const Object& in, Object& out, unsigned id){
            switch(in.type){
                case ObjectType::BOX:
                    return find<ObjectType::BOX>(in, out, id);
                case ObjectType::CLASS_INSTANCE:
                    return find<ObjectType::CLASS_INSTANCE>(in, out, id);
                case ObjectType::ENUM:
                    return find<ObjectType::ENUM>(in, out, id);
                case ObjectType::STRING:
                    return find<ObjectType::STRING>(in, out, id);
                default:
                    return false;
            }
        }

        template <>
        bool find<ObjectType::CLASS_INSTANCE>(const Object& in, Object& out, unsigned id){
            ObjectDict_t::const_iterator it = in.i_ptr->objects.find(id);
            if(it != in.i_ptr->objects.end()){
                out = it->second;
                return true;
            }

            std::vector<Class*> to_check = in.i_ptr->base->bases;
            while(!to_check.empty()){
                Class* base = to_check.back();
                to_check.pop_back();

                it = base->objects.find(id);
                if(it != base->objects.end()){
                    out = it->second;
                    return true;
                }
                to_check.insert(to_check.end(), base->bases.begin(), base->bases.end());
            }
            return false;
        }

        template <>
        bool find<ObjectType::ENUM>(const Object& in, Object& out, unsigned id){
            ObjectDict_t::const_iterator it = in.e_ptr->values.find(id);
            if(it != in.e_ptr->values.end()){
                out = it->second;
                return true;
            }

            return false;
        }

        template <>
        bool find<ObjectType::CLASS>(const Object& in, Object& out, unsigned id){
            ObjectDict_t::const_iterator it;

            std::vector<Class*> to_check {in.c_ptr};
            while(!to_check.empty()){
                Class* base = to_check.back();
                to_check.pop_back();

                it = base->objects.find(id);
                if(it != base->objects.end()){
                    out = it->second;
                    return true;
                }
                to_check.insert(to_check.end(), base->bases.rbegin(), base->bases.rend());
            }
            return false;
        }

        template <>
        bool find<ObjectType::BOX>(const Object& in, Object& out, unsigned id){
            ObjectDict_t::const_iterator it = in.c_ptr->objects.find(id);
            if(it == in.c_ptr->objects.end()){
                return false;
            }

            out = it->second;
            return true;
        }

        template <>
        bool find<ObjectType::STRING>(const Object& in, Object& out, unsigned id){
            ObjectDict_t::const_iterator it = lib::cString->objects.find(id);
            if(it != lib::cString->objects.end()){
                out = it->second;
                return true;
            }
            return false;
        }
    }
}
