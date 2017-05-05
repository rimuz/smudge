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
 *      File runtime/Object.h
 *
*/

#ifndef _SM__RUNTIME__OBJECT_H
#define _SM__RUNTIME__OBJECT_H

#include <vector>
#include <atomic>
#include <mutex>
#include <cstddef>
#include <utility>
#include <ostream>
#include <iostream>
#include <map>

#include "require_cpp11.h"
#include "typedefs.h"
#include "compile/Statement.h"
#include "utils/String.h"

namespace sm{
    struct Class;
    struct Enum;
    struct Function;
    struct Method;
    struct Instance;
    struct InstanceInfo;
    struct Box;
    struct RCString;
    class Object;

    /*
     * Only to distinguish when Object is used for strings
    */
    using StringObject = Object;

    namespace runtime{
        class GarbageCollector;
        class Runtime_t;
    }

    namespace exec{
        class Interpreter;
    }

    template <typename K, typename V>
    using Map_t = std::map<K, V>;

    template <typename Tp>
    using Dict_t = Map_t <unsigned, Tp>;

    template <size_t Size>
        using ObjectArray_t = std::array<Object, Size>;
    using ObjectVec_t = std::vector<Object>;
    using ObjectDict_t = Dict_t<Object>;
    using ObjectDictVec_t = std::vector<ObjectDict_t*>;

    using NativeFuncPtr_t = Object (*) (exec::Interpreter&, Function*, const Object&, const ObjectVec_t&);
    using Box_t = Class;
    using BoxVec_t = std::vector<Box_t*>;

    namespace ObjectType {
        enum {
            // NONE is 'null'
            NONE, INTEGER, FLOAT, STRING, CLASS_INSTANCE, ENUM, CLASS, FUNCTION,
            METHOD, BOX, WEAK_REFERENCE, STRONG_REFERENCE, INSTANCE_CREATOR,
        };
    }

    class Object {
    public:
        union {
            // primitives
            integer_t i = 0L;
            float_t f;

            // structures
            Class* c_ptr;
            Enum* e_ptr;
            Function* f_ptr;
            RCString* s_ptr;

            // objects
            Instance* i_ptr;

            // Used by references
            Object* o_ptr;

            // used to store method data
            Method* m_ptr;

            void* ptr;
        };

        enum_t type;

        Object();

        Object(const Object& rhs);
        Object(Object&& rhs);

        Object& operator=(const Object& rhs);
        Object& operator=(Object&& rhs);
        Object& operator=(std::nullptr_t); // manual reset

        /*
         * copyPointer() doesn't copy the object in
         * memory, but creates a new Object instance
         * that points the same object.
         *
         * clone(), instead, copies the entire object
         * to an other indipendent Object instance.
         *
         * makeWeak() creates a weak pointer to the
         * object (weak means that it won't modify
         * the reference count).
         *
        */
        Object copyPointer() const;
        Object clone() const;

        bool operator<(const Object& rhs) const;
        bool operator<=(const Object& rhs) const;
        bool operator>=(const Object& rhs) const;
        bool operator==(const Object& rhs) const;
        bool operator!=(const Object& rhs) const;

        /*
        * Only if Object is a reference!!
        */
        inline Object refGet() const;
        inline void refSet(Object obj) const;

        ~Object();
    };

    struct Class {
        ObjectDict_t objects;
        Class* super = nullptr; // for inheritance!
        unsigned boxName = 0, name = 0; // if this is a box, name = 0
    };

    struct Enum {
        ObjectDict_t values;
        unsigned boxName = 0, name;
    };

    struct Function {
        ByteCodeVec_t argExpr; // default argument expressions
        NameVec_t argNames; // MSB of each is 1 if arg has 'ref' prefix, 0 otherwise
        size_t address = 0;
        unsigned boxName = 0, fnName = 0;
        bool native = false;
    };

    struct Method {
        Object self;
        Object* func_ptr;
        std::atomic_uint rcount;
    };

    struct RCString{
        string<> str;
        std::atomic_uint rcount;

        template <class... Tp>
        RCString(Tp&&... args) : str(std::forward<Tp>(args)...), rcount(1){}
    };

    /*
     * works only in the Heap because of the use of 'delete'!!
    */
    struct Instance {
        friend runtime::GarbageCollector;
        friend Object;
    private:
        runtime::GarbageCollector* _gc;
        std::atomic_uint _rcount;
        /*
         * not atomic because before every use
         * you lock the global mutex in Runtime_t.
         *
         * See also: ns/runtime/gc.h
        */
        bool _temporary;

        void validate();
    public:
        std::mutex data_mutex;
        ObjectDict_t objects;
        InstanceInfo* ii_ptr;
        Class* base;

        Instance(runtime::GarbageCollector& gc, bool temp = true);
        Instance(const Instance& rhs);
        Instance(Instance&& rhs);

        Instance& operator=(const Instance& rhs);
        Instance& operator=(Instance&& rhs);

        void release();
        void join();
        void free();

        bool isTemp() const noexcept;

        Object get(string_t name) const noexcept;
        bool setFirst(string_t name, const Object& obj) noexcept;
        void set(string_t name, const Object& obj) noexcept;

        virtual ~Instance() = default;
    };

    Object newObject(runtime::GarbageCollector& gc, bool temp = true) noexcept;
    Object makeFunction(Function*) noexcept;
    Object makeList(runtime::GarbageCollector& gc, bool temp, ObjectVec_t vec) noexcept;

    Object makeTrue() noexcept;
    Object makeFalse() noexcept;

    Object Object::refGet() const{
        return *o_ptr;
    }

    void Object::refSet(Object obj) const{
        *o_ptr = std::move(obj);
    }

    inline Object makeInteger(integer_t value) noexcept{
        Object obj;
        obj.type = ObjectType::INTEGER;
        obj.i = value;
        return obj;
    }

    template <class... Tp>
    inline Object makeString(Tp&&... args) noexcept{
        Object obj;
        obj.type = ObjectType::STRING;
        obj.s_ptr = new RCString(std::forward<Tp>(args)...);
        return obj;
    }

    inline Object makeNativeFunction (unsigned fnName, unsigned boxName, NativeFuncPtr_t fn) noexcept{
        Object obj;
        obj.type = ObjectType::FUNCTION;
        obj.f_ptr = new Function;
        obj.f_ptr->native = true;
        obj.f_ptr->address = reinterpret_cast<size_t>(fn);
        obj.f_ptr->boxName = boxName;
        obj.f_ptr->fnName = fnName;
        return obj;
    }

    inline Object makeBox(Box_t* ptr) noexcept{
        Object obj;
        obj.type = ObjectType::BOX;
        obj.c_ptr = ptr;
        return obj;
    }

    inline Object makeMethod(Object self, Object* func_ptr) noexcept{
        Object obj;
        obj.type = ObjectType::METHOD;
        obj.m_ptr = new Method;
        obj.m_ptr->self = std::move(self);
        obj.m_ptr->func_ptr = func_ptr;
        return obj;
    }

    template <typename ClassType = Instance, typename... ArgTypes>
    inline Object makeFastInstance(runtime::GarbageCollector& gc, Class* base, bool temp, ArgTypes&&... args) noexcept{
        Object obj;
        obj.type = ObjectType::CLASS_INSTANCE;
        obj.i_ptr = new ClassType(gc, temp, std::forward<ArgTypes>(args)...);
        obj.i_ptr->base = base;
        return obj;
    }
}

#endif
