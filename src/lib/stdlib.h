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

#include "runtime/Object.h"
#include "runtime/gc.h"
#include "runtime/id.h"
#include "runtime/casts.h"

#include "exec/Interpreter.h"
#include "typedefs.h"

#define smId(X) \
    runtime::genOrdinaryId(*intp.rt, X)

#define smOpId(X) \
    runtime::operatorId(X)

#define smLibDecl(LibName) \
    sm::Box_t* import_##LibName(sm::runtime::Runtime_t& rt, unsigned nBox)

#define smLibTuple(LibString, LibName) \
    { LibString, import_##LibName }

#define smNativeFunc(FuncName) \
    Object FuncName(sm::exec::Interpreter& intp, \
        sm::Function* thisFn, const sm::Object& self, \
        const sm::ObjectVec_t& args)

#define smInitBox \
    Box_t* thisBox = new Box_t(); \
    thisBox->boxName = nBox;

#define smReturnBox \
    return thisBox;

#define smClass(Name) \
    { using namespace Name##Class; \
    Class*& thisClass = c##Name;\
    unsigned thisClassName = runtime::genOrdinaryId(rt, #Name); \
    thisClass = new Class; \
    thisClass->boxName = thisBox->boxName; \
    thisClass->name = thisClassName; \
    thisClass->objects = {

#define smEnd \
    }; \
        Object clazz; \
        clazz.type = ObjectType::CLASS; \
        clazz.c_ptr = thisClass; \
        thisBox->objects[thisClassName] = std::move(clazz); \
    }


#define smLambda [] (sm::exec::Interpreter& intp, \
    sm::Function* thisFn, const sm::Object& self, \
    const sm::ObjectVec_t& args) -> sm::Object

#define smVar(Name, ...) \
    setVar(rt, thisBox, #Name, (__VA_ARGS__));

#define smFunc(Name, ...) \
    setNativeFunc(thisBox, runtime::genOrdinaryId(rt, #Name), (__VA_ARGS__));

#define smOperator(Op, ...) \
    setNativeFunc(thisBox, runtime::operatorId(Op), (__VA_ARGS__));

#define smIdFunc(Id, ...) \
    setNativeFunc(thisBox, Id, (__VA_ARGS__));

#define smMethod(MethodName, ...) \
    { runtime::genOrdinaryId(rt, #MethodName), \
        genFunc(thisBox, runtime::genOrdinaryId(rt, #MethodName), (__VA_ARGS__)) },

#define smOpMethod(Operator, ...) \
    { runtime::operatorId(Operator), \
    genFunc(thisBox, runtime::operatorId(Operator), (__VA_ARGS__)) },

#define smIdMethod(Id, ...) \
    { Id, genFunc(thisBox, Id, (__VA_ARGS__)) },

#define smSetData(Type) \
    setData<Type>(intp, self)

#define smGetData(Type) \
    getData<Type>(intp, self)

#define smHas(Id) \
    (self.i_ptr->objects.find(Id) != self.i_ptr->objects.end())

#define smRef(Id) \
    (self.i_ptr->objects[Id])

namespace sm {
    namespace lib {
        using InitFunc_t = Box_t* (*) (runtime::Runtime_t&, unsigned);
        using LibDict_t = Map_t<string_t, InitFunc_t>;

        smLibDecl(io);
        smLibDecl(lang);
        smLibDecl(system);

        const LibDict_t libs = {
            smLibTuple("std.io!", io),
            smLibTuple("std.lang!", lang),
            smLibTuple("std.system!", system),
        };

        template <typename Tp>
        inline Tp*& data(const Object& obj){
            return reinterpret_cast<Tp*&>(obj.i_ptr->objects[runtime::dataId].ptr);
        }

        template <typename Tp>
        inline Tp*& getData(exec::Interpreter& intp, const Object& obj){
            Tp*& ptrRef = data<Tp>(obj);
            if(!ptrRef){
                intp.rt->sources.printStackTrace(intp, error::ERROR,
                    std::string("native data not initialized before use in "
                    "object ") + runtime::errorString(intp, obj));
            }
            return ptrRef;
        }

        template <typename Tp>
        inline Tp*& setData(exec::Interpreter& intp, const Object& obj){
            Tp*& ptrRef = data<Tp>(obj);
            if(ptrRef){
                intp.rt->sources.printStackTrace(intp, error::ERROR,
                    std::string("initializing native data twice in "
                    "object ") + runtime::errorString(intp, obj));
            }
            return ptrRef;
        }

        inline void setNativeFunc(Box_t* box, oid_t id, NativeFuncPtr_t fn){
            box->objects[id] = makeNativeFunction(id, box->boxName, fn);
        }

        inline void setVar(runtime::Runtime_t& rt, Box_t* box,
                const string_t& name, Object obj){
            unsigned varName = runtime::genOrdinaryId(rt, name);
            box->objects[varName] = std::move(obj);
        }

        inline Object genFunc(Box_t* box, oid_t id, NativeFuncPtr_t fn){
            return makeNativeFunction(id, box->boxName, fn);
        }
    }
}

#endif
