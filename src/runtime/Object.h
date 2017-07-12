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

#include "typedefs.h"
#include "compile/Statement.h"
#include "utils/String.h"

namespace sm{
    struct Class;
    struct Enum;
    struct Function;
    struct Method;
    struct Box;
    struct RCString;

    class Object;
    class Instance;

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
    using ClassVec_t = BoxVec_t;
    using InstanceList_t = std::list<Instance>;

    namespace ObjectType {
        enum {
            // NONE is 'null'
            NONE, INTEGER, FLOAT, STRING, CLASS_INSTANCE, ENUM, CLASS, FUNCTION,
            METHOD, BOX, WEAK_REFERENCE, STRONG_REFERENCE, INSTANCE_CREATOR,
            NATIVE_DATA
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
        Object(enum_t tp) : type(tp) {}

        Object(const Object& rhs);
        Object(Object&& rhs);

        Object& operator=(const Object& rhs);
        Object& operator=(Object&& rhs);
        Object& operator=(std::nullptr_t); // manual reset

        /*
        * Only if Object is a reference!!
        */
        inline Object refGet() const noexcept;
        inline void refSet(Object obj) const noexcept;

        ~Object();
    };

    struct Class {
        ObjectDict_t objects;
        ClassVec_t bases;
        unsigned boxName = 0;  // if this is a box
        union {
            unsigned name = 0; // (Only classes)
            bool isInitialized; // (Only boxes)
        };
    };

    struct Enum {
        ObjectDict_t values;
        unsigned boxName = 0, name;
    };

    enum FunctionFlags {
        FF_NATIVE = 0x1,
        FF_VARARGS = 0x2,
    };

    struct Function {
        /*
         * in arguments, unsigned is the name ID of the argument,
         * the size_t, the last valid instruction address of the 'argcode' +1.
         * The 'argcode' is the code that has to be executed to get
         * the default value of an argument.
         * If the last argument is a VARARG, flags will have
         * value FF_VARARGS (see the above enum).
        */
        std::vector<std::tuple<unsigned, size_t>> arguments;
        union {
            NativeFuncPtr_t native_ptr;     // if native,
            size_t address = 0;             // if not.
        };
        unsigned boxName = 0, fnName = 0;
        char flags = 0;
    };

    struct Method {
        Object self;
        Object* func_ptr;
        std::atomic_uint rcount;

        Method() : rcount(1){}
    };

    struct RCString{
        string<> str;
        std::atomic_uint rcount;

        template <class... Tp>
        RCString(Tp&&... args) : str(std::forward<Tp>(args)...), rcount(1){}
    };

    struct Instance {
    public:
        ObjectDict_t objects;
        InstanceList_t::iterator it;
        exec::Interpreter& intp;
        Class* base;

        unsigned rcount;
        bool temporary;

        Instance(exec::Interpreter& _intp, Class* _base, bool temp)
            : intp(_intp), base(_base), rcount(1), temporary(temp) {}

        Instance(Instance&& rhs) = default;
        Instance& operator=(Instance&& rhs) = default;

        // Instance is not copyable
        Instance(const Instance& rhs) = delete;
        Instance& operator=(const Instance& rhs) = delete;

        void free(bool isGc = false) noexcept;

        Object get(string_t name) const noexcept;
        bool setFirst(string_t name, const Object& obj) noexcept;
        void set(string_t name, const Object& obj) noexcept;

        virtual ~Instance() = default;
    };

    size_t objectHash(exec::Interpreter& intp, const Object& obj) noexcept;

    Object makeFunction(Function*) noexcept;
    Object makeList(exec::Interpreter& intp, bool temp, ObjectVec_t vec = ObjectVec_t()) noexcept;
    Object makeTuple(exec::Interpreter& intp, bool temp, ObjectVec_t vec = ObjectVec_t()) noexcept;

    // creates an object
    Object makeInstance(exec::Interpreter& intp, Class* base, bool temp) noexcept;
    // creates an object AND calls ctor
    Object newInstance(exec::Interpreter& intp, Class* base, bool temp,
        const ObjectVec_t& args = {}) noexcept;

    Object makeTrue() noexcept;
    Object makeFalse() noexcept;
    Object makeBool(bool) noexcept;

    /* if obj is tuple or list */
    bool hasVector(const Object& obj, ObjectVec_t*& vecPtr) noexcept;

    Object Object::refGet() const noexcept{
        return *o_ptr;
    }

    void Object::refSet(Object obj) const noexcept{
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
        obj.f_ptr->flags = FF_NATIVE;
        obj.f_ptr->native_ptr = fn;
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

    inline Object makeClass(Class* ptr) noexcept{
        Object obj;
        obj.type = ObjectType::CLASS;
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
}

#endif
