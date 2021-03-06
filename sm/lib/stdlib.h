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
 *      File lib/stdlib.h
 *
*/

#ifndef _SM__LIB__STD_LIB_H
#define _SM__LIB__STD_LIB_H

#include <unordered_map>

#include "sm/runtime/Object.h"
#include "sm/runtime/gc.h"
#include "sm/runtime/id.h"
#include "sm/runtime/casts.h"

#include "sm/exec/Interpreter.h"
#include "sm/typedefs.h"

#define smId(X) \
    runtime::genOrdinaryId(*intp.rt, X)

#define smOpId(X) \
    runtime::operatorId(X)

#define smLibDecl(LibName) \
    sm::Box* import_##LibName(sm::runtime::Runtime_t& rt, unsigned nBox)

#ifdef _SM_OS_WINDOWS
#   define smLibrary \
        extern "C" sm::Box* __declspec(dllexport) import_library(sm::runtime::Runtime_t& rt, unsigned nBox)
#else
#   define smLibrary \
        extern "C" sm::Box* import_library(sm::runtime::Runtime_t& rt, unsigned nBox)
#endif

#define smLibTuple(LibString, LibName) \
    { LibString, import_##LibName }

#define smNativeFunc(FuncName) \
    sm::RootObject FuncName(sm::exec::Interpreter& intp, \
        sm::Function* thisFn, const sm::RootObject& self, \
        const sm::RootObjectVec_t& args)

#define smInitBox \
    sm::Box* thisBox = new sm::Box(); \
    thisBox->name = nBox;

#define smReturnBox \
    return thisBox;

#define smClass(Name) \
    { using namespace Name##Class; \
    sm::Class*& thisClass = c##Name;\
    unsigned thisClassName = sm::runtime::genOrdinaryId(rt, #Name); \
    thisClass = new sm::Class; \
    thisClass->boxName = thisBox->name; \
    thisClass->name = thisClassName; \
    thisClass->objects = {

#define smEnd \
    }; \
        sm::Object clazz; \
        clazz.type = sm::ObjectType::CLASS; \
        clazz.c_ptr = thisClass; \
        thisBox->objects[thisClassName] = std::move(clazz); \
    }

#define smLambda \
    [] (sm::exec::Interpreter& intp, \
    sm::Function* thisFn, const sm::RootObject& self, \
    const sm::RootObjectVec_t& args) -> sm::RootObject

#define smVar(Name, ...) \
    sm::lib::setVar(rt, thisBox, #Name, (__VA_ARGS__));

#define smFunc(Name, ...) \
    sm::lib::setNativeFunc(thisBox, sm::runtime::genOrdinaryId(rt, #Name), (__VA_ARGS__));

#define smOperator(Op, ...) \
    sm::lib::setNativeFunc(thisBox, sm::runtime::operatorId(Op), (__VA_ARGS__));

#define smIdFunc(Id, ...) \
    sm::lib::setNativeFunc(thisBox, Id, (__VA_ARGS__));

#define smGCSearch(...) \
    { sm::runtime::gcSearchId, sm::lib::genGCSearch(__VA_ARGS__) },

#define smSearchLambda \
    [] (const Object& self, ObjectVec_t& out) -> void

#define smOpMethod(Operator, ...) \
    { sm::runtime::operatorId(Operator), \
        sm::lib::genFunc(thisBox, sm::runtime::operatorId(Operator), (__VA_ARGS__)) },

#define smIdMethod(Id, ...) \
    { Id, sm::lib::genFunc(thisBox, Id, (__VA_ARGS__)) },

#define smMethod(Id, ...) \
    { sm::runtime::genOrdinaryId(rt, #Id), sm::lib::genFunc(thisBox, \
        sm::runtime::genOrdinaryId(rt, #Id), (__VA_ARGS__)) },

#define smSetData(Type) \
    sm::lib::setData<Type>(intp, self)

#define smGetData(Type) \
    sm::lib::getData<Type>(intp, self)

#define smDeleteData(Type) \
    Type* ptr = smGetData(Type); \
    sm::lib::data<Type>(self) = nullptr; \
    delete ptr;

#define smHas(Id) \
    (self->i_ptr->objects.find(Id) != self->i_ptr->objects.end())

#define smRef(Id) \
    (static_cast<Object&>(self->i_ptr->objects[Id]))

namespace sm {
    namespace lib {
        #ifdef _SM_OS_WINDOWS
        using DynInitFunc_t = Box* (*) (runtime::Runtime_t&, unsigned);
        #else
        using DynInitFunc_t = Box* (*) (runtime::Runtime_t&, unsigned);
        #endif

        using InitFunc_t = Box* (*) (runtime::Runtime_t&, unsigned);
        using LibDict_t = Map_t<string_t, InitFunc_t>;

        smLibDecl(lang);
        extern const LibDict_t libs;

        template <typename Tp>
        inline Tp*& data(const Object& obj){
            return reinterpret_cast<Tp*&>(obj.i_ptr->objects[runtime::dataId].ptr);
        }

        template <typename Tp>
        inline Tp*& getData(exec::Interpreter& intp, const Object& obj){
            Tp*& ptrRef = data<Tp>(obj);
            if(!ptrRef){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("native data not initialized before use in "
                    "object ") + runtime::errorString(intp, obj));
            }
            return ptrRef;
        }

        template <typename Tp>
        inline Tp*& setData(exec::Interpreter& intp, const Object& obj){
            Tp*& ptrRef = data<Tp>(obj);
            if(ptrRef){
                intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                    std::string("initializing native data twice in "
                    "object ") + runtime::errorString(intp, obj));
            }
            return ptrRef;
        }

        inline void setNativeFunc(Box* box, oid_t id, NativeFuncPtr_t fn){
            box->objects[id] = makeNativeFunction(id, box->name, fn);
        }

        inline void setVar(runtime::Runtime_t& rt, Box* box,
                const string_t& name, Object obj){
            unsigned varName = runtime::genOrdinaryId(rt, name);
            box->objects[varName] = std::move(obj);
        }

        inline RootObject genFunc(Box* box, oid_t id, NativeFuncPtr_t fn){
            return makeNativeFunction(id, box->name, fn);
        }

        inline RootObject genGCSearch(GCSearchFunc_t ptr){
            Object obj(ObjectType::NATIVE_DATA);
            obj.gcs_ptr = ptr;
            return obj;
        }
    }
}

#endif
