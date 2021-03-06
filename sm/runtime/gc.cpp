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
#include <mutex>
#include <vector>
#include <algorithm>

#include "sm/runtime/id.h"
#include "sm/runtime/gc.h"
#include "sm/runtime/Object.h"
#include "sm/runtime/casts.h"
#include "sm/compile/Statement.h"
#include "sm/compile/defs.h"
#include "sm/exec/Interpreter.h"
#include "sm/typedefs.h"

#ifndef _SM_OS_WINDOWS
#include <dlfcn.h>
#endif

namespace sm{
    using namespace ObjectType;

    Object::Object() noexcept
            : i(0), type(NONE) {}

    Object::Object(const Object& rhs) noexcept
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

    Object::Object(Object&& rhs) noexcept
            : i(rhs.i), type(rhs.type) {
        rhs.type = NONE;
        rhs.i = 0;
    }

    Object& Object::operator=(const Object& rhs) noexcept{
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

    Object& Object::operator=(Object&& rhs) noexcept{
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

    Object& Object::operator=(std::nullptr_t) noexcept{
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

    Object& Object::operator=(const RootObject& rhs) noexcept {
        return this->operator=(rhs.get());
    }

    Object& Object::operator=(RootObject&& rhs) noexcept{
        return this->operator=(rhs.get());
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

    RootObject::RootObject(const Object& rhs) noexcept{
        if(rhs.type == ObjectType::CLASS_INSTANCE && rhs.i_ptr)
            ++rhs.i_ptr->roots;
        obj = rhs;
    }

    RootObject::RootObject(Object&& rhs) noexcept{
        if(rhs.type == ObjectType::CLASS_INSTANCE && rhs.i_ptr)
            ++rhs.i_ptr->roots;
        obj = std::move(rhs);
    }

    RootObject::RootObject(const RootObject& rhs) noexcept{
        obj = rhs.obj;
        if(obj.type == ObjectType::CLASS_INSTANCE && obj.i_ptr)
            ++obj.i_ptr->roots;
    }

    RootObject::RootObject(RootObject&& rhs) noexcept{
        obj = rhs.obj;
        rhs.obj = nullptr;
    }

    RootObject& RootObject::operator=(const Object& rhs) noexcept{
        if(obj.type == ObjectType::CLASS_INSTANCE && obj.i_ptr)
            --obj.i_ptr->roots;
        obj = rhs;
        if(obj.type == ObjectType::CLASS_INSTANCE && obj.i_ptr)
            ++obj.i_ptr->roots;
        return *this;
    }

    RootObject& RootObject::operator=(Object&& rhs) noexcept{
        if(obj.type == ObjectType::CLASS_INSTANCE && obj.i_ptr)
            --obj.i_ptr->roots;
        obj = std::move(rhs);
        if(obj.type == ObjectType::CLASS_INSTANCE && obj.i_ptr)
            ++obj.i_ptr->roots;
        return *this;
    }

    RootObject& RootObject::operator=(const RootObject& rhs) noexcept{
        if(obj.type == ObjectType::CLASS_INSTANCE && obj.i_ptr)
            --obj.i_ptr->roots;
        obj = rhs.obj;
        if(obj.type == ObjectType::CLASS_INSTANCE && obj.i_ptr)
            ++obj.i_ptr->roots;
        return *this;
    }

    RootObject& RootObject::operator=(RootObject&& rhs) noexcept {
        if(obj.type == ObjectType::CLASS_INSTANCE && obj.i_ptr)
            --obj.i_ptr->roots;
        obj = std::move(rhs.obj);
        rhs.obj = nullptr;
        return *this;
    }

    RootObject::~RootObject() noexcept{
        if(obj.type == ObjectType::CLASS_INSTANCE && obj.i_ptr)
            --obj.i_ptr->roots;
    }

    ObjectHash::ObjectHash(runtime::Runtime_t& ref) : rt(ref){}

    size_t ObjectHash::operator() (const RootObject& hashable) const noexcept{
        switch(hashable->type){
            case NONE:
                return 0;
            case INTEGER:
                return std::hash<integer_t>()(hashable->i);
            case FLOAT:
                return std::hash<float_t>()(hashable->f);
            case STRING:
                return hashable->s_ptr->str.hash();
            case CLASS_INSTANCE: {
                exec::Interpreter* intp = rt.getCurrentThread();
                if(!intp)
                    return 0;

                Object func, self = hashable;
                Function* f_ptr;

                if(!runtime::find<CLASS_INSTANCE>(hashable, func, lib::idHash)){
                    intp->rt->sources.printStackTrace(*intp, error::ET_ERROR,
                        std::string("function 'hash()' not found in ")
                        + runtime::errorString(*intp, hashable));
                } else if(!runtime::callable(func, self, f_ptr)){
                    intp->rt->sources.printStackTrace(*intp, error::ET_ERROR,
                        std::string("'hash' is not a function in ")
                        + runtime::errorString(*intp, hashable));
                }

                RootObject ret = intp->callFunction(f_ptr, {}, self, true);
                if(ret->type != INTEGER){
                    intp->rt->sources.printStackTrace(*intp, error::ET_ERROR,
                        std::string("'hash()' must return an int value in ")
                        + runtime::errorString(*intp, hashable));
                }

                return ret->i;
            }

            case BOX: {
                exec::Interpreter* intp = rt.getCurrentThread();
                if(!intp)
                    return 0;

                Object func, self;
                Function* f_ptr;

                if(!runtime::find<BOX>(hashable, func, lib::idHash)){
                    intp->rt->sources.printStackTrace(*intp, error::ET_ERROR,
                        std::string("function 'hash()' not found in ")
                        + runtime::errorString(*intp, hashable));
                } else if(!runtime::callable(func, self, f_ptr)){
                    intp->rt->sources.printStackTrace(*intp, error::ET_ERROR,
                        std::string("'hash' is not a function in ")
                        + runtime::errorString(*intp, hashable));
                }

                RootObject ret = intp->callFunction(f_ptr, {}, self, true);
                if(ret->type != INTEGER){
                    intp->rt->sources.printStackTrace(*intp, error::ET_ERROR,
                        std::string("'hash()' must return an int value in ")
                        + runtime::errorString(*intp, hashable));
                }

                return ret->i;
            }

            default:{
                exec::Interpreter* intp = rt.getCurrentThread();
                if(!intp)
                    return 0;

                intp->rt->sources.printStackTrace(*intp, error::ET_ERROR,
                    runtime::errorString(*intp, hashable)
                    + " is not an hashable object.");
                return 0;
             }
        }
    }

    EqualTo::EqualTo(runtime::Runtime_t& ref) : rt(ref) {}

    bool EqualTo::operator() (const RootObject& lhs, const RootObject& rhs) const noexcept{
        switch(lhs->type){
            case NONE:
                return rhs->type == NONE;
            case INTEGER:
                return rhs->type == INTEGER && lhs->i == rhs->i;
            case FLOAT:
                return rhs->type == FLOAT && lhs->f == rhs->f;
            case STRING:
                return rhs->type == STRING && lhs->s_ptr->str == rhs->s_ptr->str;
            case CLASS_INSTANCE: {
                exec::Interpreter* intp = rt.getCurrentThread();
                if(!intp)
                    return 0;

                Object func, self = lhs;
                Function* f_ptr;

                if(!runtime::find<CLASS_INSTANCE>(lhs, func, runtime::operatorId(parse::TT_EQUAL))){
                    intp->rt->sources.printStackTrace(*intp, error::ET_ERROR,
                        std::string("'operator==' not found in ")
                        + runtime::errorString(*intp, lhs));
                } else if(!runtime::callable(func, self, f_ptr)){
                    intp->rt->sources.printStackTrace(*intp, error::ET_ERROR,
                        std::string("'operator==' is not a function in ")
                        + runtime::errorString(*intp, lhs));
                }

                return runtime::implicitToBool(intp->callFunction(f_ptr, {rhs}, self, true));
            }

            case BOX: {
                exec::Interpreter* intp = rt.getCurrentThread();
                if(!intp)
                    return 0;

                Object func, self;
                Function* f_ptr;

                if(!runtime::find<BOX>(lhs, func, runtime::operatorId(parse::TT_EQUAL))){
                    intp->rt->sources.printStackTrace(*intp, error::ET_ERROR,
                        std::string("'operator==' not found in ")
                        + runtime::errorString(*intp, lhs));
                } else if(!runtime::callable(func, self, f_ptr)){
                    intp->rt->sources.printStackTrace(*intp, error::ET_ERROR,
                        std::string("'operator==' is not a function in ")
                        + runtime::errorString(*intp, lhs));
                }

                return runtime::implicitToBool(intp->callFunction(f_ptr, {rhs}, self, true));
            }

            default: {
                exec::Interpreter* intp = rt.getCurrentThread();
                if(!intp)
                    return 0;

                intp->rt->sources.printStackTrace(*intp, error::ET_ERROR,
                    runtime::errorString(*intp, lhs)
                    + " is not an comparable object.");
                return 0;
            }
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

    RootObject makeInstance(exec::Interpreter& intp, Class* base) noexcept{
        return intp.rt->gc.makeTempInstance(intp, base);
    }


    RootObject newInstance(exec::Interpreter& intp, Class* base, const RootObjectVec_t& args) noexcept {
        RootObject self, clazz(ObjectType::CLASS);
        Function* func_ptr;
        clazz->c_ptr = base;

        runtime::callable(clazz, self, func_ptr);
        return intp.callFunction(func_ptr, args, self, true);
    }

    void Instance::free(bool isGc) noexcept {
        bool cantWork = !isGc && rt.gc.gcWorking;
        if(deleting || cantWork)
            return;
        if(isGc){
            destroy(false);
            rt.gc.instances.erase(it);
        } else {
            destroy(callDelete);
            std::lock_guard<std::mutex> lock(rt.gc.instances_m);
            rt.gc.instances.erase(it);
        }
    }

    void Instance::destroy(bool invokeDeleteFn) noexcept{
        if(!base)
            return;
        exec::Interpreter* i_ptr = rt.getCurrentThread();

        if(!i_ptr) // Interpreter was deleted alredy (end of main)
            return;

        exec::Interpreter& intp = *i_ptr;
        ObjectDict_t::iterator it;
        Function* func_ptr;

        Object self = Object(ObjectType::CLASS_INSTANCE);
        self.i_ptr = this;
        deleting = true;

        if(invokeDeleteFn){
            it = base->objects.find(lib::idDelete);
            if(it != base->objects.end()){
                if(!runtime::callable(it->second, self, func_ptr))
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("'delete' is not a function in ")
                        + runtime::errorString(intp, self));
                // we don't care about the 'delete()' return value.
                intp.callFunction(func_ptr, {}, self, true);
            }
        } else {
            it = base->objects.find(runtime::gcCollectId);
            if(it != base->objects.end()){
                if(!runtime::callable(it->second, self, func_ptr))
                    intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                        std::string("'<gc_collect>' is not a function in ")
                        + runtime::errorString(intp, self));
                // we don't care about the '<gc_collect>()' return value.
                intp.callFunction(func_ptr, {}, self, true);
            }
        }

        objects.clear();
    }

    Instance::~Instance(){
        --rt.gc.allocs;
    }

    namespace runtime{
        std::chrono::steady_clock::time_point* Runtime_t::execStart = nullptr;
        std::vector<LibHandle_t> Runtime_t::sharedLibs;

        // Instance Pointer's Vector  -> IPVec_t
        using IPVec_t = std::vector<Instance*>;
        using CPVec_t = std::vector<Class*>;
        using MPVec_t = std::vector<Method*>;

        Object GarbageCollector::makeTempInstance(exec::Interpreter& _intp, Class* _base) noexcept {
            if(++allocs == threshold){
                allocs = 0;
                collect();
            }

            instances.emplace_back(*_intp.rt, _base, true);
            InstanceList_t::iterator it = std::prev(instances.end());
            it->it = it; // this line made my day :D
            it->temporary = true;

            Object obj;
            obj.type = ObjectType::CLASS_INSTANCE;
            obj.i_ptr = &*it;
            return obj;
        }

        struct GCData{
            exec::Interpreter intp;
            IPVec_t white, gray;
            CPVec_t classes;
            MPVec_t methods;

            GCData(Runtime_t& rt) noexcept : intp(rt) {}
        };

        void whiteToGray(GCData& data, Object& obj){
            if(obj.type == ObjectType::CLASS_INSTANCE){
                Instance* ptr = obj.i_ptr;
                IPVec_t::iterator curr = std::find(data.white.begin(), data.white.end(), ptr);

                /*
                 * If curr is in WHITE, it's neither in the GRAY nor in the BLACK
                */
                if(curr != data.white.end()) {
                    // moving curr (WHITE) in GRAY
                    std::swap(*curr, data.white.back());
                    data.white.pop_back();
                    data.gray.emplace_back(ptr);

                    for(auto& child : ptr->objects){
                        // recursively search inside all objects pointed by obj
                        whiteToGray(data, child.second);
                    }
                }

                ObjectDict_t::iterator it = obj.i_ptr->base->objects.find(runtime::gcSearchId);
                if(it != obj.i_ptr->base->objects.end()){
                    ObjectVec_t out;
                    it->second.gcs_ptr(obj, out);

                    for(auto& child : out){
                        /*
                         * recursively search inside all objects
                         * natively linked to obj, such as for
                         * List or Tuple.
                        */
                        whiteToGray(data, child);
                    }
                }

            } else if(obj.type == ObjectType::CLASS){
                if(std::find(data.classes.begin(), data.classes.end(), obj.c_ptr) == data.classes.end())
                    return;

                data.classes.emplace_back(obj.c_ptr);
                for(auto& child : obj.c_ptr->objects)
                    whiteToGray(data, child.second);
            } else if(obj.type == ObjectType::METHOD){
                if(std::find(data.methods.begin(), data.methods.end(), obj.m_ptr) == data.methods.end())
                    return;

                data.methods.emplace_back(obj.m_ptr);
                whiteToGray(data, obj.m_ptr->self);
            }
        }

        /*
         * Tri-color garbage collection.
         * 3rd implementation (2017.09.17)
         *
         * Note: There is no BLACK, because we
         * can easily avoid to use it.
         *
        */

        void GarbageCollector::collect(){
            std::lock_guard<std::mutex> lock1(instances_m), lock2(_rt->threads_m);
            GCData data(*_rt);
            gcWorking = true;

            /*
             * GRAY = roots
             * WHITE = other objects
            */
            for(Instance& inst : instances){
                if(inst.roots)
                    data.gray.emplace_back(&inst);
                else
                    data.white.emplace_back(&inst);
            }

            while(!data.gray.empty()){
                Instance* ptr = data.gray.back();
                data.gray.pop_back();

                for(auto& child : ptr->objects){
                    whiteToGray(data, child.second);
                }
            }

            // now WHITE contains garbage.
            for(Instance* ptr : data.white){
                if(!ptr->roots){
                    ptr->free(true);
                }
            }

            // now we've finished! good job!
            gcWorking = false;
        }

        std::thread::id Runtime_t::main_id;

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

        // could be nullptr!! (checked by the caller)
        exec::Interpreter* Runtime_t::getCurrentThread() noexcept{
            std::thread::id curr = std::this_thread::get_id();
            if(curr == Runtime_t::main_id)
                return main_intp;

            std::lock_guard<std::mutex> lock(threads_m);
            auto it = threads.find(curr);
            if(it == threads.end()) {
                sources.msg(error::ET_BUG, "current thread does not have an exec::Interpreter instance in 'Runtime_t::threads' (err #1)");
                return nullptr; // should never return
            }

            return &it->second.data->intp;
        }

        void Runtime_t::exit() noexcept{
            if(execStart){
                std::chrono::steady_clock::time_point execEnd = std::chrono::steady_clock::now();
                std::cout << ".. Total time: " <<
                (static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(execEnd - *execStart).count())
                / 1000.f) << " ms." << std::endl;
            }
            freeLibraries();
        }

        void Runtime_t::freeLibraries() noexcept{
            for(LibHandle_t lib : sharedLibs)
                #ifdef _SM_OS_WINDOWS
                    FreeLibrary(lib);
                #else
                    dlclose(lib);
                #endif
            sharedLibs.clear();
        }

        void Runtime_t::freeData() {
            for(auto* box : boxes)
                delete box;
            boxes.clear();
        }

        Runtime_t::~Runtime_t() {
            freeLibraries();
            gc.gcWorking = true;
            // keeping gcWorking true
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
