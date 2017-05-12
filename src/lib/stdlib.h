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
#include "exec/Interpreter.h"
#include "require_cpp11.h"
#include "typedefs.h"

#define _Id(X) runtime::genOrdinaryId(rt, X)
#define _OpId(X) runtime::operatorId(X)
#define _LibDecl(LibName) sm::Box_t* import_##LibName(sm::runtime::Runtime_t& rt, unsigned nBox)
#define _LibTuple(LibString, LibName) { LibString, import_##LibName }
#define _NativeFunc(FuncName) Object FuncName(sm::exec::Interpreter& intp, sm::Function* thisFn, const sm::Object& self, const sm::ObjectVec_t& args)
#define _NativeMethod(MethodName, nArgs) inline sm::Object MethodName(sm::exec::Interpreter& intp, \
    sm::Function* thisFn, const sm::Object& self, const sm::ObjectArray_t<nArgs>& args)
#define _BindMethod(ClassName, FuncName, nArgs) \
    _NativeFunc(FuncName){ \
        if(!runtime::of_type(self, c##ClassName)) \
            intp.rt->sources.printStackTrace(*intp.rt, error::FATAL_ERROR, \
                "called native method '" #FuncName "' from an incompatible " \
                "object to native class '" #ClassName "'"); \
        sm::ObjectArray_t<nArgs> newArgs; \
        size_t argsSize = args.size(); \
        if(nArgs){ \
            if(argsSize > nArgs){ \
                std::copy(args.begin(), args.begin() + nArgs, newArgs.begin()); \
            } else { \
                std::copy(args.begin(), args.end(), newArgs.begin()); \
            } \
        } \
        return reinterpret_cast<ClassName*>(self.i_ptr)->FuncName(intp, thisFn, self, newArgs); \
    }
#define _MethodTuple(Namespace, MethodName) { _Id(#MethodName), \
    genFunc(rt, box, #MethodName, Namespace::MethodName) }
#define _OpTuple(Namespace, Operator, MethodName) { _OpId(Operator),\
    genOpFunc(rt, box, Operator, Namespace::MethodName) }


namespace sm {
    namespace lib {
        using InitFunc_t = Box_t* (*) (runtime::Runtime_t&, unsigned);
        using LibDict_t = Map_t<string_t, InitFunc_t>;

        _LibDecl(io);
        _LibDecl(lang);

        const LibDict_t libs = {
            _LibTuple("std/io.sm", io),
            _LibTuple("std/lang.sm", lang),
        };

        inline unsigned id(runtime::Runtime_t& rt, const string_t& str){
            return runtime::genOrdinaryId(rt, str);
        }

        inline void setNativeFn(runtime::Runtime_t& rt, Box_t* box,
                const string_t& name, NativeFuncPtr_t fn){
            unsigned fnName = runtime::genOrdinaryId(rt, name);
            Object func = makeNativeFunction(fnName, box->boxName, fn);
            box->objects[fnName] = std::move(func);
        }

        inline void setNativeOp(runtime::Runtime_t& rt, Box_t* box,
                enum_t op, NativeFuncPtr_t fn){
            unsigned fnName = runtime::operatorId(op);
            Object func = makeNativeFunction(fnName, box->boxName, fn);
            box->objects[fnName] = std::move(func);
        }

        inline void setVar(runtime::Runtime_t& rt, Box_t* box,
                const string_t& name, Object obj){
            unsigned varName = runtime::genOrdinaryId(rt, name);
            box->objects[varName] = std::move(obj);
        }

        inline Class* setClass(runtime::Runtime_t& rt, Box_t* box,
                const string_t& name, ObjectDict_t&& dict){
            unsigned className = runtime::genOrdinaryId(rt, name);
            Object clazz;
            clazz.type = ObjectType::CLASS;
            clazz.c_ptr = new Class;
            clazz.c_ptr->boxName = box->boxName;
            clazz.c_ptr->name = className;
            clazz.c_ptr->objects = std::move(dict);
            return (box->objects[className] = std::move(clazz)).c_ptr;
        }

        inline Object genFunc(runtime::Runtime_t& rt, Box_t* box,
                const string_t& name, NativeFuncPtr_t fn){
            unsigned fnName = runtime::genOrdinaryId(rt, name);
            Object func = makeNativeFunction(fnName, box->boxName, fn);
            return func;
        }

        inline Object genOpFunc(runtime::Runtime_t& rt, Box_t* box,
                enum_t op, NativeFuncPtr_t fn){
            unsigned fnName = runtime::operatorId(op);
            Object func = makeNativeFunction(fnName, box->boxName, fn);
            return func;
        }
    }
}

#endif
