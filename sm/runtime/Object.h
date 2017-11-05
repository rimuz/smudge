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
#include <cstddef>
#include <utility>
#include <ostream>
#include <iostream>
#include <map>
#include <array>

#include "sm/typedefs.h"
#include "sm/compile/Statement.h"
#include "sm/utils/String.h"

namespace sm{
    struct Class;
    struct Enum;
    struct Function;
    struct Method;
    struct Box;
    struct RCString;

    class RootObject;
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

    template <size_t Size>
    using RootObjectArray_t = std::array<RootObject, Size>;
    using RootObjectVec_t = std::vector<RootObject>;
    using RootObjectDict_t = Dict_t<RootObject>;
    using RootObjectDictVec_t = std::vector<RootObjectDict_t*>;

    using NativeFuncPtr_t = RootObject (*) (exec::Interpreter&, Function*, const RootObject&, const RootObjectVec_t&);
    using BoxVec_t = std::vector<Box*>;
    using ClassVec_t = std::vector<Class*>;
    using InstanceList_t = std::list<Instance>;
    using GCSearchFunc_t = void (*) (const Object&, ObjectVec_t&);

    constexpr std::nullptr_t nullobj = nullptr;

    namespace ObjectType {
        enum {
            // NONE is 'null'
            NONE, INTEGER, FLOAT, STRING, CLASS_INSTANCE, ENUM, CLASS, FUNCTION,
            METHOD, BOX, WEAK_REFERENCE, STRONG_REFERENCE, INSTANCE_CREATOR,
            NATIVE_DATA
        };

        constexpr const char* strTypes[] = {
            "NONE", "INTEGER", "FLOAT", "STRING", "CLASS_INSTANCE", "ENUM", "CLASS", "FUNCTION",
            "METHOD", "BOX", "WEAK_REFERENCE", "STRONG_REFERENCE", "INSTANCE_CREATOR",
            "NATIVE_DATA"
        };
    }

    class Object {
    public:
        union {
            // primitives
            integer_t i = 0L;
            float_t f;

            // structures
            Box* b_ptr;
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

            GCSearchFunc_t gcs_ptr;

            void* ptr;
        };

        enum_t type;

        Object() noexcept;
        Object(enum_t tp) noexcept : type(tp) {}
        Object(std::nullptr_t) noexcept : type(ObjectType::NONE){}

        Object(const Object& rhs) noexcept;
        Object(Object&& rhs) noexcept;

        Object& operator=(const Object& rhs) noexcept;
        Object& operator=(Object&& rhs) noexcept;
        Object& operator=(const RootObject& rhs) noexcept;
        Object& operator=(RootObject&& rhs) noexcept;

        Object& operator=(std::nullptr_t) noexcept; // manual reset

        /*
        * Only if Object is a reference!!
        */
        inline Object refGet() const noexcept;
        inline void refSet(Object obj) const noexcept;
        ~Object();
    };

    class RootObject {
    private:
        Object obj;
    public:
        RootObject() noexcept = default;
        RootObject(const Object&) noexcept;
        RootObject(Object&&) noexcept;

        RootObject(const RootObject&) noexcept;
        RootObject(RootObject&&) noexcept;

        RootObject& operator=(const Object&) noexcept;
        RootObject& operator=(Object&&) noexcept;

        RootObject& operator=(const RootObject&) noexcept;
        RootObject& operator=(RootObject&&) noexcept;

        inline Object& get() noexcept;
        inline const Object& get() const noexcept;

        inline const Object* operator->() const noexcept;
        inline Object* operator->() noexcept;

        inline operator const Object&() const noexcept;
        inline operator Object&() noexcept;

        ~RootObject() noexcept;
    };

    struct Box {
        RootObjectDict_t objects;
        unsigned name = 0;
        bool isInitialized = false;
    };

    struct Class {
        ObjectDict_t objects;
        ClassVec_t bases;
        unsigned boxName = 0, name = 0;
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
        std::vector<std::pair<unsigned, size_t>> arguments;
        union {
            NativeFuncPtr_t native_ptr;     // if native,
            size_t address = 0;             // if not.
        };
        unsigned boxName = 0, fnName = 0;
        byte_t flags = 0;
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

    class Instance {
        friend runtime::Runtime_t;
        friend runtime::GarbageCollector;
    private:
        bool deleting = false; // if dtor has been called
        bool callDelete = true; // whether destroy() has to be called
    public:
        ObjectDict_t objects;
        InstanceList_t::iterator it;
        runtime::Runtime_t& rt;
        Class* base;

        std::atomic_uint rcount, roots;
        bool temporary;

        Instance(runtime::Runtime_t& _rt, Class* _base, bool temp)
            : rt(_rt), base(_base), rcount(1), roots(0), temporary(temp) {}

        Instance(Instance&& rhs) = default;
        Instance& operator=(Instance&& rhs) = default;

        // Instance is not copyable
        Instance(const Instance& rhs) = delete;
        Instance& operator=(const Instance& rhs) = delete;

        void free(bool isGc = false) noexcept;
        void destroy(bool invokeDeleteFn) noexcept;

        Object get(string_t name) const noexcept;
        bool setFirst(string_t name, const Object& obj) noexcept;
        void set(string_t name, const Object& obj) noexcept;

        ~Instance();
    };

    size_t objectHash(exec::Interpreter& intp, const Object& obj) noexcept;

    Object makeFunction(Function*) noexcept;
    RootObject makeList(exec::Interpreter& intp, RootObjectVec_t vec = RootObjectVec_t()) noexcept;
    RootObject makeTuple(exec::Interpreter& intp, RootObjectVec_t vec = RootObjectVec_t()) noexcept;

    inline Object makeFloat(float_t value) noexcept;
    template <typename... Tp>
    inline Object makeString(Tp&&... args) noexcept;
    inline Object makeInteger(integer_t value) noexcept;
    inline Object makeBox(Box* ptr) noexcept;
    inline Object makeClass(Class* ptr) noexcept;

    inline Object makeMethod(Object self, Object* func_ptr) noexcept;
    inline Object makeNativeFunction (unsigned fnName, unsigned boxName, NativeFuncPtr_t fn) noexcept;

    // creates an object
    RootObject makeInstance(exec::Interpreter& intp, Class* base) noexcept;
    // creates an object AND calls ctor
    RootObject newInstance(exec::Interpreter& intp, Class* base, const RootObjectVec_t& args = {}) noexcept;

    Object makeTrue() noexcept;
    Object makeFalse() noexcept;
    Object makeBool(bool) noexcept;

    /* if obj is tuple or list */
    bool hasVector(exec::Interpreter& intp, const Object& obj, ObjectVec_t*& vecPtr) noexcept;

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

    inline Object makeFloat(float_t value) noexcept{
        Object obj;
        obj.type = ObjectType::FLOAT;
        obj.f = value;
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

    inline Object makeBox(Box* ptr) noexcept{
        Object obj;
        obj.type = ObjectType::BOX;
        obj.b_ptr = ptr;
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

    template <bool Strong = false>
    inline Object makeRef(Object& ref) noexcept = delete;
    template <bool Strong = false>
    inline Object makeRef(RootObject& ref) noexcept = delete;

    template <>
    inline Object makeRef<false>(Object& ref) noexcept{
        Object obj(ObjectType::WEAK_REFERENCE);
        obj.o_ptr = &ref;
        return obj;
    }

    template <>
    inline Object makeRef<true>(Object& ref) noexcept{
        Object obj(ObjectType::STRONG_REFERENCE);
        obj.o_ptr = &ref;
        return obj;
    }

    template <>
    inline Object makeRef<false>(RootObject& ref) noexcept{
        Object obj(ObjectType::WEAK_REFERENCE);
        obj.o_ptr = &ref.get();
        return obj;
    }

    template <>
    inline Object makeRef<true>(RootObject& ref) noexcept{
        Object obj(ObjectType::STRONG_REFERENCE);
        obj.o_ptr = &ref.get();
        return obj;
    }

    inline Object& RootObject::get() noexcept{
        return obj;
    }

    inline const Object& RootObject::get() const noexcept{
        return obj;
    }

    inline const Object* RootObject::operator->() const noexcept{
        return &obj;
    }

    inline Object* RootObject::operator->() noexcept{
        return &obj;
    }

    inline RootObject::operator const Object&() const noexcept{
        return obj;
    }

    inline RootObject::operator Object&() noexcept{
        return obj;
    }
}

#endif
