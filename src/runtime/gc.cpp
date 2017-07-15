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
 *      File runtime/gc.cpp
 *
*/

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

#include "runtime/id.h"
#include "runtime/gc.h"
#include "runtime/Object.h"
#include "runtime/casts.h"
#include "compile/Statement.h"
#include "compile/defs.h"
#include "exec/Interpreter.h"

namespace sm{
    using namespace ObjectType;

    Object::Object()
            : i(0), type(NONE) {}

    Object::Object(const Object& rhs)
            : i(rhs.i), type(rhs.type) {
        if(i_ptr){
            if(type == CLASS_INSTANCE){
                ++i_ptr->rcount;
            } else if(type == STRING){
                ++s_ptr->rcount;
            } else if(type == METHOD){
                ++m_ptr->rcount;
            }
        }
    }

    Object::Object(Object&& rhs)
            : i(rhs.i), type(rhs.type) {
        rhs.type = NONE;
        rhs.i = 0;
    }

    Object& Object::operator=(const Object& rhs){
        if(i_ptr){
            if(type == CLASS_INSTANCE){
                if(!--i_ptr->rcount){
                    i_ptr->free();
                }
            } else if(type == STRING){
                if(!--s_ptr->rcount){
                    delete s_ptr;
                }
            } else if(type == METHOD){
                if(!--m_ptr->rcount){
                    delete m_ptr;
                }
            }
        }

        i = rhs.i;
        type = rhs.type;

        if(i_ptr){
            if(type == CLASS_INSTANCE){
                ++i_ptr->rcount;
            } else if(type == STRING){
                ++s_ptr->rcount;
            } else if(type == METHOD){
                ++m_ptr->rcount;
            }
        }

        return *this;
    }

    Object& Object::operator=(Object&& rhs){
        if(i_ptr){
            if(type == CLASS_INSTANCE){
                if(!--i_ptr->rcount){
                    i_ptr->free();
                }
            } else if(type == STRING){
                if(!--s_ptr->rcount){
                    delete s_ptr;
                }
            } else if(type == METHOD){
                if(!--m_ptr->rcount){
                    delete m_ptr;
                }
            }
        }

        i = rhs.i;
        type = rhs.type;

        rhs.i = 0;
        rhs.type = NONE;

        return *this;
    }

    Object& Object::operator=(std::nullptr_t){
        if(i_ptr){
            if(type == CLASS_INSTANCE){
                if(!--i_ptr->rcount){
                    i_ptr->free();
                }
            } else if(type == STRING){
                if(!--s_ptr->rcount){
                    delete s_ptr;
                }
            } else if(type == METHOD){
                if(!--m_ptr->rcount){
                    delete m_ptr;
                }
            }
        }

        i = 0;
        type = NONE;

        return *this;
    }

    Object::~Object(){
        if(i_ptr){
            if(type == CLASS_INSTANCE){
                if(!--i_ptr->rcount){
                    i_ptr->free();
                }
            } else if(type == STRING){
                if(!--s_ptr->rcount){
                    delete s_ptr;
                }
            } else if(type == METHOD){
                if(!--m_ptr->rcount){
                    delete m_ptr;
                }
            }
        }
    }

    size_t objectHash(exec::Interpreter& intp, const Object& obj) noexcept{
        Object hashable = obj;
        if(hashable.type == WEAK_REFERENCE
                || hashable.type == STRONG_REFERENCE)
            hashable = *hashable.o_ptr;

        switch(hashable.type){
            case NONE:
                return 0;
            case INTEGER:
                return std::hash<integer_t>()(hashable.i);
            case FLOAT:
                return std::hash<float_t>()(hashable.f);
            case STRING:
                return hashable.s_ptr->str.hash();
            case CLASS_INSTANCE: {
                Object func;
                Function* f_ptr;
                exec::Interpreter& intp = hashable.i_ptr->intp;

                if(!runtime::find<CLASS_INSTANCE>(hashable, func, lib::idHash)){
                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("function 'hash()' not found in ")
                        + runtime::errorString(intp, hashable));
                } else if(!runtime::callable(func, hashable, f_ptr)){
                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("'hash' is not a function in ")
                        + runtime::errorString(intp, hashable));
                }

                Object ret = intp.callFunction(f_ptr, {}, hashable, true);
                if(ret.type != INTEGER){
                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("'hash()' must return an int value in ")
                        + runtime::errorString(intp, hashable));
                }

                return ret.i;
            }

            case BOX: {
                Object func;
                Object self;
                Function* f_ptr;
                if(!runtime::find<BOX>(hashable, func, lib::idHash)){
                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("function 'hash()' not found in ")
                        + runtime::errorString(intp, hashable));
                } else if(!runtime::callable(func, self, f_ptr)){
                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("'hash' is not a function in ")
                        + runtime::errorString(intp, hashable));
                }

                Object ret = intp.callFunction(f_ptr, {}, self, true);
                if(ret.type != INTEGER){
                    intp.rt->sources.printStackTrace(intp, error::ERROR,
                        std::string("'hash()' must return an int value in ")
                        + runtime::errorString(intp, hashable));
                }

                return ret.i;
            }

            default:
                intp.rt->sources.printStackTrace(intp, error::ERROR,
                    runtime::errorString(intp, hashable)
                    + " is not an hashable object.");
                return 0;
        }
    }

    Object makeFunction(Function* fn) noexcept {
        Object obj;
        obj.type = ObjectType::FUNCTION;
        obj.f_ptr = fn;
        return obj;
    }

    Object makeTrue() noexcept{
        Object obj;
        obj.type = ObjectType::INTEGER;
        obj.i = 1;
        return obj;
    }

    Object makeFalse() noexcept{
        Object obj;
        obj.type = ObjectType::INTEGER;
        obj.i = 0;
        return obj;
    }

    Object makeBool(bool b) noexcept{
        Object obj;
        obj.type = ObjectType::INTEGER;
        obj.i = static_cast<integer_t>(b);
        return obj;
    }

    Object makeInstance(exec::Interpreter& intp, Class* base, bool temp) noexcept{
        return intp.rt->gc.instance(intp, base, temp);
    }


    Object newInstance(exec::Interpreter& intp, Class* base, bool temp,
            const ObjectVec_t& args) noexcept {
        Object self, clazz(ObjectType::CLASS);
        Function* func_ptr;
        clazz.c_ptr = base;

        runtime::callable(clazz, self, func_ptr);
        return intp.callFunction(func_ptr, args, self, true);
    }

    void Instance::free(bool isGc) noexcept {
        if((isGc || !intp.rt->gc.gcWorking) && !deleting)
            (temporary ? intp.rt->gc.tempInstances : intp.rt->gc.instances).erase(it);
    }

    Instance::~Instance(){
        --intp.rt->gc.allocs;
        if(!base) return;
        ObjectDict_t::iterator it = base->objects.find(lib::idDelete);
        if(it != base->objects.end()){
            Object self = Object(ObjectType::CLASS_INSTANCE);
            Function* func_ptr;

            self.i_ptr = this;
            deleting = true;

            if(!runtime::callable(it->second, self, func_ptr))
                intp.rt->sources.printStackTrace(intp, error::ERROR,
                    std::string("'delete' is not a function in ")
                    + runtime::errorString(intp, self));
            // we don't care about the 'delete()' return value.
            intp.callFunction(func_ptr, {}, self, true);
        }
    }

    namespace runtime{
        std::chrono::steady_clock::time_point* Runtime_t::execStart = nullptr;
        // Instance Pointer's Vector  -> IPVec_t
        using IPVec_t = std::vector<Instance*>;

        Object GarbageCollector::instance(exec::Interpreter& _intp,
                Class* _base, bool temp) noexcept {
            if(++allocs == threshold){
                allocs = 0;
                collect();
            }

            InstanceList_t& list = temp ? tempInstances : instances;
            list.emplace_back(_intp, _base, temp);

            InstanceList_t::iterator it = std::prev(list.end());
            it->it = it; // this line made my day :D

            Object obj;
            obj.type = ObjectType::CLASS_INSTANCE;
            obj.i_ptr = &*it;
            return obj;
        }

        void whiteToGray(IPVec_t& white, IPVec_t& gray, Object& obj){
            if(obj.type == CLASS_INSTANCE){
                Instance* ptr = obj.i_ptr;
                IPVec_t::iterator curr = std::find(white.begin(), white.end(), ptr);

                /*
                 * If curr is in WHITE, it's neither in the GRAY nor in the BLACK
                */
                if(curr != white.end()) {
                    *curr = nullptr; // no erases in the vector
                    gray.emplace_back(ptr);

                    for(auto& child : ptr->objects){
                        // recursively search inside all objects pointed by obj
                        whiteToGray(white, gray, child.second);
                    }
                }
            }
        }

        inline void loadVec(InstanceList_t& list, IPVec_t& vec){
            vec.reserve(list.size());
            for(Instance& inst : list){
                vec.emplace_back(&inst);
            }
        }

        /*
         * Tri-color garbage collection.
         * 2nd implementation (2017.07.08)
         *
         * Note: There is no BLACK, because we
         * can be easily avoid to use it.
         *
         * Note 2: It won't erase items from WHITE,
         * It will set them to nullptr
         *
        */

        // TODO Class
        // TODO Methods
        void GarbageCollector::collect(){
            IPVec_t white, gray;
            gcWorking = true;

            loadVec(instances, white);
            loadVec(tempInstances, gray);

            // searching in the ROOTs
            for(auto* box : _rt->boxes){
                for(auto& child : box->objects){
                    whiteToGray(white, gray, child.second);
                }
            }

            for(auto* intp : _rt->threads){
                for(Object& obj : intp->exprStack){
                    whiteToGray(white, gray, obj);
                }

                for(exec::CallInfo_t& callInfo : intp->funcStack){
                    for(ObjectDict_t* dict : callInfo.codeBlocks){
                        if(dict){
                            for(auto& pair : *dict){
                                whiteToGray(white, gray, pair.second);
                            }
                        }
                    }
                }
            }

            // now GRAY contains all the root pointers and white the other.
            while(!gray.empty()){
                Instance* ptr = gray.back();
                gray.pop_back();

                for(auto& child : ptr->objects){
                    whiteToGray(white, gray, child.second);
                }
            }

            // now WHITE contains garbage.
            for(Instance* ptr : white){
                if(ptr){ // ignore 'erased' elements
                    ptr->free(true);
                }
            }

            // now we've finished! good job!
            gcWorking = false;
        }

        string_t Runtime_t::nameFromId(unsigned id) const{
            if(id < runtime::idsStart){
                switch(id){
                    case runtime::initId:
                        return "<init>";
                    case runtime::roundId:
                        return "operator()";
                    case runtime::squareId:
                        return "operator[]";
                    default:
                        return std::string("operator") + parse::normalOperatorsPlain[id];
                }
            } else {
                return nameConstants[id - runtime::idsStart];
            }
        }

        void Runtime_t::exit(int returnValue) noexcept{
            if(execStart){
                std::chrono::steady_clock::time_point execEnd = std::chrono::steady_clock::now();
                std::cout << ".. Total time: " <<
                (static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(execEnd - *execStart).count())
                / 1000.f) << " ms." << std::endl;
            }
            std::exit(returnValue);
        }

        namespace test{
            void print(const Runtime_t& rt){
                size_t i = 0;
                std::cout << "Runtime_t::nameConstants:" << std::endl;
                for(const string_t& name : rt.nameConstants){
                    std::cout << "  " << std::hex << std::setfill('0') << std::setw(2)
                        << i << ": " << std::dec << name << std::endl;
                    i++;
                }
                std::cout << "Runtime_t::intConstants:" << std::endl;
                i = 0;
                for(const integer_t& integer : rt.intConstants){
                    std::cout << "  " << std::hex << std::setfill('0') << std::setw(2)
                        << i << ": " << std::dec << integer << std::endl;
                    i++;
                }
                std::cout << "Runtime_t::floatConstants:" << std::endl;
                i = 0;
                for(const float_t& f : rt.floatConstants){
                    std::cout << "  " << std::hex << std::setfill('0') << std::setw(2)
                        << i << ": " << std::dec << f << std::endl;
                    i++;
                }
                std::cout << "Runtime_t::stringConstants:" << std::endl;
                i = 0;
                for(const String& str : rt.stringConstants){
                    std::cout << "  " << std::hex << std::setfill('0') << std::setw(2)
                        << i << ": " << std::dec << str << std::endl;
                    i++;
                }
                std::cout << "Runtime_t::code:" << std::endl << std::dec;
                compile::test::print(rt.code);
            }
        }
    }
}
