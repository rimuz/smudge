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
#include "exec/Interpreter.h"
#include "compile/Statement.h"
#include "runtime/casts.h"

namespace sm{
    using namespace ObjectType;

    namespace lib {
        extern oid_t idHash;
    }

    Object::Object()
            : i(0), type(NONE) {}

    Object::Object(const Object& rhs)
            : i(rhs.i), type(rhs.type) {
        if(i_ptr){
            if(type == CLASS_INSTANCE){
                i_ptr->join();
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
                i_ptr->release();
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
                i_ptr->join();
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
                i_ptr->release();
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
                i_ptr->release();
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

    Object Object::copyPointer() const{
        return Object(*this);
    }

    Object Object::clone() const{
        //TODO: CLONE
        return Object();
    }

    Object::~Object(){
        if(type == CLASS_INSTANCE){
            i_ptr->release();
        } else if(type == STRING){
            if(!--s_ptr->rcount){
                delete s_ptr;
            }
        }
    }

    Instance::Instance(runtime::GarbageCollector& gc, bool temp)
            : _gc(&gc), _rcount(1), _temporary(temp) {
        if(temp){
            gc.addTemporaryInstance(this);
        } else {
            gc.addInstance(this);
        }
    }

    Instance::Instance(const Instance& rhs)
            : _gc(rhs._gc), _rcount(1), _temporary(rhs._temporary), objects(rhs.objects){
        if(_temporary){
            _gc->addTemporaryInstance(this);
        } else {
            _gc->addInstance(this);
        }
    }

    Instance::Instance(Instance&& rhs)
            : _gc(rhs._gc), _rcount(1), _temporary(rhs._temporary), objects(rhs.objects){
        if(_temporary){
            _gc->addTemporaryInstance(this);
        } else {
            _gc->addInstance(this);
        }
    }

    Instance& Instance::operator=(const Instance& rhs){
        objects = rhs.objects;
        _gc = rhs._gc;
        return *this;
    }

    Instance& Instance::operator=(Instance&& rhs){
        objects = rhs.objects;
        _gc = rhs._gc;
        return *this;
    }

    void Instance::validate(){
        std::lock_guard<std::mutex> lock(_gc->_rt->globalMutex);
        if(_temporary){
            _gc->removeTemporaryInstance(this);
            _gc->addInstance(this);
        }
    }

    void Instance::release(){
        if(!--_rcount){
            free();
        }
    }

    void Instance::join(){
        ++_rcount;
    }

    void Instance::free(){
        if(_temporary){
            _gc->removeTemporaryInstance(this);
        } else {
            _gc->removeInstance(this);
        }
        delete this;
    }

    bool Instance::isTemp() const noexcept{
        return _temporary;
    }

    size_t objectHash(exec::Interpreter& intp, const Object& obj){
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

    Object newObject(runtime::GarbageCollector& gc, bool temp) noexcept {
        Object obj;
        obj.type = ObjectType::CLASS_INSTANCE;
        obj.i_ptr = new Instance(gc, temp);
        return obj;
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

    namespace runtime{
        std::chrono::steady_clock::time_point* Runtime_t::execStart = nullptr;

        void GarbageCollector::addInstance(Instance* ptr){
            std::lock_guard<std::mutex> lock(vecMutex);
            if(++allocs > threshold){
                collect();
            }
            pointers.push_back(ptr);
        }

        void GarbageCollector::removeInstance(Instance* ptr){
            std::lock_guard<std::mutex> lock(vecMutex);
            allocs--;
            pointers.erase(std::remove(pointers.begin(), pointers.end(), ptr), pointers.end());
        }

        void GarbageCollector::addTemporaryInstance(Instance* ptr){
            std::lock_guard<std::mutex> lock(vecMutex);
            if(++allocs > threshold){
                collect();
            }
            temp.push_back(ptr);
        }

        void GarbageCollector::removeTemporaryInstance(Instance* ptr){
            std::lock_guard<std::mutex> lock(vecMutex);
            allocs--;
            temp.erase(std::remove(temp.begin(), temp.end(), ptr), temp.end());
        }

        using InVec_t = std::vector<Instance*>;
        inline void whiteToGray(InVec_t& white, InVec_t& grey, InVec_t& black, const Object& obj){
            if(obj.type == CLASS_INSTANCE && obj.i_ptr && std::find(grey.begin(), grey.end(), obj.i_ptr) == grey.end()
                     && std::find(black.begin(), black.end(), obj.i_ptr) == black.end()){
                std::lock_guard<std::mutex> dataLock(obj.i_ptr->data_mutex);
                white.erase(std::remove(white.begin(), white.end(), obj.i_ptr), white.end());
                grey.push_back(obj.i_ptr);
            }
        }

        /*
         * tri-colour garbage, 1st implementation.
         * TODO: optimize!
         *
         * NOTE: collect() doesn't lock vecMutex!
         * You have to lock it before any call!!
        */
        void GarbageCollector::collect(){
            std::lock_guard<std::mutex> globalLock(_rt->globalMutex);
            std::lock_guard<std::mutex> threadsLock(_rt->threadsDataMutex);

            std::vector<Instance*>  white(pointers), grey(temp), black;

            for(BoxVec_t::const_iterator cit = _rt->boxes.begin(); cit != _rt->boxes.end(); ++cit){
                for(ObjectDict_t::const_iterator cit2 = (*cit)->objects.begin(); cit2 != (*cit)->objects.end(); ++cit2){
                    whiteToGray(white, grey, black, cit2->second);
                }
            }

            for(exec::ThreadVec_t::iterator cit = _rt->threads.begin(); cit != _rt->threads.end(); ++cit){
                const exec::Interpreter& intp = **cit;

                for(ObjectVec_t::const_iterator cit2 = intp.exprStack.begin();
                        cit2 != intp.exprStack.end(); ++cit2){
                    whiteToGray(white, grey, black, *cit2);
                }

                for(exec::CallStack_t::const_iterator cit2 = intp.funcStack.begin();
                        cit2 != intp.funcStack.end(); ++cit2){
                    whiteToGray(white, grey, black, cit2->thisObject);

                    for(ObjectDictVec_t::const_iterator cit3 = cit2->codeBlocks.begin();
                            cit3 != cit2->codeBlocks.end(); ++cit3){
                        for(ObjectDict_t::const_iterator cit4 = (*cit3)->begin();
                                cit4 != (*cit3)->end(); ++cit4){
                            whiteToGray(white, grey, black, cit4->second);
                        }
                    }
                }
            }

            // now grey contains all the root pointers and white the other.
            while(!grey.empty()){
                Instance* ptr = grey.back();
                grey.pop_back();
                black.push_back(ptr);

                for(ObjectDict_t::const_iterator cit = ptr->objects.begin(); cit != ptr->objects.end(); ++cit){
                    const Object& obj = cit->second;
                    whiteToGray(white, grey, black, obj);
                }
            }

            // now black contains used data, and white garbage.
            for(Instance* ptr : white){
                ptr->free();
            }

            // now we've finished! good job!
            allocs = 0;
        }

        string_t Runtime_t::nameFromId(unsigned id) const{
            if(id < runtime::idsStart){
                switch(id){
                    case runtime::initId:
                        return "<init>";
                    case runtime::newId:
                        return "new";
                    case runtime::deleteId:
                        return "delete";
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
