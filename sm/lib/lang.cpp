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
 *      File lib/lang.cpp
 *
*/

#include <algorithm>

#include "sm/utils/unicode/utf8.h"
#include "sm/lib/stdlib.h"

#include "sm/runtime/casts.h"
#include "sm/runtime/Object.h"
#include "sm/runtime/utils.h"

namespace sm{
    namespace lib{
        Class* cString = nullptr;
        Class* cList = nullptr;
        Class* cTuple = nullptr;
        Class* cTable = nullptr;
        Class* cListIterator = nullptr;
        Class* cStringIterator = nullptr;

        namespace StringClass {}
        namespace ListClass {}
        namespace TupleClass {}
        namespace TableClass {
            using Table_t = std::unordered_map<RootObject, RootObject, ObjectHash, EqualTo>;
        }
        namespace ListIteratorClass {}
        namespace StringIteratorClass {}

        smLibDecl(lang){
            smInitBox

            smClass(String)
                /*
                 *
                 *       .d8888b.   888              d8b
                 *      d88P  Y88b  888              Y8P
                 *      Y88b.       888
                 *       "Y888b.    888888  888d888  888  88888b.    .d88b.
                 *          "Y88b.  888     888P"    888  888 "88b  d88P"88b
                 *            "888  888     888      888  888  888  888  888
                 *      Y88b  d88P  Y88b.   888      888  888  888  Y88b 888
                 *       "Y8888P"    "Y888  888      888  888  888   "Y88888
                 *                                                       888
                 *                                                  Y8b d88P
                 *                                                   "Y88P"
                */

                smMethod(new, smLambda {
                    return Object();
                })

                smMethod(delete, smLambda {
                    return Object();
                })

                smOpMethod(parse::TT_EQUAL, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return makeInteger(0);
                    return makeInteger(self->s_ptr->str == args[0]->s_ptr->str);
                })

                smOpMethod(parse::TT_NOT_EQUAL, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return makeInteger(0);
                    return makeInteger(self->s_ptr->str != args[0]->s_ptr->str);
                })

                smOpMethod(parse::TT_LESS, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return makeInteger(0);
                    return makeInteger(self->s_ptr->str < args[0]->s_ptr->str);
                })

                smOpMethod(parse::TT_GREATER, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return makeInteger(0);
                    return makeInteger(self->s_ptr->str > args[0]->s_ptr->str);
                })

                smOpMethod(parse::TT_LESS_OR_EQUAL, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return makeInteger(0);
                    return makeInteger(self->s_ptr->str <= args[0]->s_ptr->str);
                })

                smOpMethod(parse::TT_GREATER_OR_EQUAL, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return makeInteger(0);
                    return makeInteger(self->s_ptr->str >= args[0]->s_ptr->str);
                })

                smOpMethod(parse::TT_PLUS, smLambda {
                    if(args.empty())
                        return Object();
                    Object str0 = makeString(self->s_ptr->str);
                    Object str1 = runtime::implicitToString(intp, args[0]);
                    str0.s_ptr->str.insert(str0.s_ptr->str.end(), str1.s_ptr->str.begin(),
                        str1.s_ptr->str.end());
                    return str0;
                })

                smOpMethod(parse::TT_MINUS, smLambda {
                    integer_t i;
                    if(args.empty() || args[0]->type != ObjectType::INTEGER || (i = args[0]->i) < 0)
                        return Object();
                    if(i >= static_cast<integer_t>(self->s_ptr->str.size()))
                        return Object();
                    return makeString(self->s_ptr->str.begin(), self->s_ptr->str.end() - i);
                })

                smOpMethod(parse::TT_MULT, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::INTEGER)
                        return Object();

                    integer_t i = args[0]->i;
                    Object str0 = makeString();
                    size_t sz = self->s_ptr->str.size();

                    if(i == 0) {
                        return str0;
                    } else if(i < 0){
                        i *= -1;
                        str0.s_ptr->str.resize(sz * i);

                        String::reverse_iterator beg = self->s_ptr->str.rbegin();
                        String::reverse_iterator end = self->s_ptr->str.rend();
                        String::iterator curr = str0.s_ptr->str.begin();

                        for(integer_t j = 0; j != i; ++j){
                            curr = std::copy(beg, end, curr);
                        }
                    } else {
                        str0.s_ptr->str.resize(sz * i);

                        String::iterator beg = self->s_ptr->str.begin();
                        String::iterator end = self->s_ptr->str.end();
                        String::iterator curr = str0.s_ptr->str.begin();

                        for(integer_t j = 0; j != i; ++j){
                            curr = std::copy(beg, end, curr);
                        }
                    }
                    return str0;
                })

                smMethod(idx, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::INTEGER)
                        return Object();

                    integer_t i = args[0]->i;
                    String::iterator begin = self->s_ptr->str.begin(),
                        end = self->s_ptr->str.end();

                    if(i > 0){
                        String::iterator ch_it = String::nthChar(begin, end, i);
                        return makeInteger(std::distance(begin, ch_it));
                    } else if(i < 0){
                        integer_t n = self->s_ptr->str.uSize() + i;
                        if(n <= 0)
                            return Object();
                        String::iterator ch_it = String::nthChar(begin, end, n);
                        return makeInteger(std::distance(begin, ch_it));
                    }
                    return makeInteger(0);
                })

                smMethod(len, smLambda {
                    return makeInteger(self->s_ptr->str.size());
                })

                smMethod(count, smLambda {
                    size_t argc = args.size();

                    integer_t start = 0;
                    integer_t size = self->s_ptr->str.size();
                    integer_t end = size;

                    if(argc != 0){
                        if(args[0]->type == ObjectType::INTEGER){
                            start = args[0]->i;
                            if(!runtime::findIndex(start, start, size))
                                return Object();
                        } else if(args[0]->type != ObjectType::NONE)
                            return Object();
                    }

                    if(argc > 1){
                        if(args[1]->type == ObjectType::INTEGER){
                            end = args[1]->i;
                            if(end < 0){
                                end += size;
                                if(end < 0 || end < start)
                                    return Object();
                            } else if(end > size)
                                end = size;
                            else if(end < start)
                                return Object();
                        } else if(args[1]->type != ObjectType::NONE)
                            return Object();
                    }

                    String::const_iterator beg = self->s_ptr->str.begin();
                    return makeInteger(String::uSize(beg + start, beg + end));
                })

                smMethod(empty, smLambda {
                    return makeInteger(self->s_ptr->str.empty());
                })

                smMethod(compare, smLambda {
                    size_t argc = args.size();
                    if(argc == 0 || args[0]->type != ObjectType::STRING){
                        return makeInteger(0);
                    } else if(argc >= 2 && runtime::implicitToBool(args[1])){
                        return makeInteger(self->s_ptr->str.compareIgnoreCase(args[0]->s_ptr->str));
                    }
                    return makeInteger(self->s_ptr->str.compare(args[0]->s_ptr->str));
                })

                smMethod(u_compare, smLambda {
                    size_t argc = args.size();
                    if(argc == 0 || args[0]->type != ObjectType::STRING){
                        return makeInteger(1);
                    } else if(argc >= 2 && runtime::implicitToBool(args[1])){
                        return makeInteger(self->s_ptr->str.uCompareIgnoreCase(args[0]->s_ptr->str));
                    }
                    return makeInteger(self->s_ptr->str.compare(args[0]->s_ptr->str));
                })

                smMethod(get, smLambda {
                    String::const_iterator begin = self->s_ptr->str.cbegin();

                    if(self->s_ptr->str.empty())
                        return Object();
                    if(args.empty()){
                        return makeString(begin, begin+1);
                    } else if(args[0]->type == ObjectType::INTEGER){
                        integer_t idx = args[0]->i;
                        if(idx >= 0){
                            size_t i = static_cast<size_t>(idx);
                            if(i >= self->s_ptr->str.size())
                                return Object();
                            return makeString(begin+i, begin+i+1);
                        }

                        idx += static_cast<integer_t>(self->s_ptr->str.size());
                        if(idx < 0)
                            return Object();
                        return makeString(begin+idx, begin+idx+1);
                    }
                    return Object();
                })

                smMethod(u_get, smLambda {
                    if(self->s_ptr->str.empty())
                        return Object();
                    integer_t strSize = self->s_ptr->str.uSize();
                    if(args.empty()){
                        return makeString(self->s_ptr->str.uCharAt(0));
                    } else if(args[0]->type == ObjectType::INTEGER){
                        integer_t idx = args[0]->i;
                        if(idx >= 0){
                            if(idx >= strSize)
                                return Object();
                            return makeString(self->s_ptr->str.uCharAt(idx));
                        }

                        idx += strSize;
                        if(idx < 0)
                            return Object();
                        return makeString(self->s_ptr->str.uCharAt(idx));
                    }
                    return Object();
                })

                smMethod(getc, smLambda {
                    if(self->s_ptr->str.empty())
                        return Object();
                    if(args.empty()){
                        return makeInteger(self->s_ptr->str[0]);
                    } else if(args[0]->type == ObjectType::INTEGER){
                        integer_t idx = args[0]->i;
                        if(idx >= 0){
                            size_t i = static_cast<size_t>(idx);
                            if(i >= self->s_ptr->str.size())
                                return Object();
                            return makeInteger(self->s_ptr->str[i]);
                        }

                        idx += static_cast<integer_t>(self->s_ptr->str.size());
                        if(idx < 0)
                            return Object();
                        return makeInteger(self->s_ptr->str[static_cast<size_t>(idx)]);
                    }
                    return Object();
                })

                smMethod(u_getc, smLambda {
                    if(self->s_ptr->str.empty())
                        return Object();
                    integer_t strSize = self->s_ptr->str.uSize();
                    if(args.empty()){
                        return makeInteger(uGetCodepoint(self->s_ptr->str.uCharAt(0)));
                    } else if(args[0]->type == ObjectType::INTEGER){
                        integer_t idx = args[0]->i;
                        if(idx >= 0){
                            if(idx >= strSize)
                                return Object();
                            return makeInteger(uGetCodepoint(self->s_ptr->str.uCharAt(idx)));
                        }

                        idx += strSize;
                        if(idx < 0)
                            return Object();
                        return makeInteger(uGetCodepoint(self->s_ptr->str.uCharAt(idx)));
                    }
                    return Object();
                })

                smMethod(contains, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return Object();
                    return makeInteger(self->s_ptr->str.contains(args[0]->s_ptr->str));
                })

                smMethod(bytes, smLambda {
                    RootObject list = makeList(intp, RootObjectVec_t(self->s_ptr->str.size()));

                    String::const_iterator iit = self->s_ptr->str.begin();
                    ObjectVec_t& out = *getData<ObjectVec_t>(intp, list);
                    ObjectVec_t::iterator oit = out.begin(), end = out.end();

                    while(oit != end){
                        *oit++ = makeInteger(static_cast<unsigned char>(*iit++));
                    }
                    return list;
                })

                smMethod(chars, smLambda {
                    RootObject list = makeList(intp);

                    String::const_iterator curr = self->s_ptr->str.begin();
                    String::const_iterator end = self->s_ptr->str.end();
                    ObjectVec_t& out = *getData<ObjectVec_t>(intp, list);;
                    unicode_t ch;

                    while(String::uNext(curr, end, ch)){
                        out.push_back(makeInteger(uGetCodepoint(ch)));
                    }
                    return list;
                })

                smMethod(join, smLambda {
                    if(args.empty())
                        return makeString();
                    ObjectVec_t* in;
                    if(!hasVector(intp, args[0], in))
                        return Object();
                    Object out = makeString();
                    ObjectVec_t::const_iterator curr = in->cbegin(), end = in->cend();
                    if(curr != end){
                        while(1){
                            Object str = runtime::implicitToString(intp, *curr);
                            out.s_ptr->str.append(str.s_ptr->str);
                            if(++curr == end)
                                break;
                            else
                                out.s_ptr->str.append(self->s_ptr->str);
                        }
                    }
                    return out;
                })

                smMethod(ends_with, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return Object();
                    return makeInteger(self->s_ptr->str.endsWith(args[0]->s_ptr->str));
                })

                smMethod(starts_with, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return Object();
                    return makeInteger(self->s_ptr->str.startsWith(args[0]->s_ptr->str));
                })

                smMethod(hash, smLambda {
                    return makeInteger(self->s_ptr->str.hash());
                })

                smMethod(find, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return Object();
                    return makeInteger(self->s_ptr->str.find(args[0]->s_ptr->str)
                        - self->s_ptr->str.begin());
                })

                smMethod(find_last, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return Object();
                    return makeInteger(self->s_ptr->str.findLast(args[0]->s_ptr->str)
                        - self->s_ptr->str.begin());
                })

                smMethod(substr, smLambda {
                    size_t argc = args.size();

                    if(argc == 0 || args[0]->type != ObjectType::INTEGER){
                        return Object();
                    }

                    integer_t size = self->s_ptr->str.size();
                    integer_t start = args[0]->i;
                    String::iterator begin = self->s_ptr->str.begin();

                    if(start < 0){
                        start += size;
                        if(start < 0)
                            return Object();
                    } else if(start >= size)
                        return Object();

                    if(argc >= 2){
                        integer_t end = args[1]->i;
                        if(end < 0){
                            end += size;
                            if(end < 0 || end < start)
                                return Object();
                        } else if(end > size)
                            return Object();
                        else if(end < start)
                            return Object();
                        return makeString(begin + start, begin + end);
                    }

                    return makeString(begin + start, begin + size);
                })

                smMethod(u_substr, smLambda {
                    size_t argc = args.size();

                    if(argc == 0 || args[0]->type != ObjectType::INTEGER){
                        return Object();
                    }

                    integer_t size = self->s_ptr->str.uSize();
                    integer_t start = args[0]->i;

                    if(!runtime::findIndex(start, start, size))
                        return Object();

                    if(argc >= 2){
                        integer_t end = args[1]->i;
                        if(end < 0){
                            end += size;
                            if(end < 0 || end < start)
                                return Object();
                        } else if(end > size)
                            return Object();
                        else if(end < start)
                            return Object();
                        return makeString(self->s_ptr->str.uSubstr(start, end));
                    }

                    return makeString(self->s_ptr->str.uSubstr(start));
                })

                smMethod(replace_first, smLambda {
                    size_t argc = args.size();
                    if(argc == 0 || args[0]->type != ObjectType::STRING){
                        return self;
                    } else if(argc >= 2){
                        if(args[1]->type != ObjectType::STRING)
                            return Object();
                        return makeString(self->s_ptr->str.replaceFirst(
                                args[0]->s_ptr->str, args[1]->s_ptr->str
                        ));
                    }
                    return makeString(self->s_ptr->str.replaceFirst(args[0]->s_ptr->str));
                })

                smMethod(replace, smLambda {
                    size_t argc = args.size();
                    if(argc == 0 || args[0]->type != ObjectType::STRING){
                        return self;
                    } else if(argc >= 2){
                        if(args[1]->type != ObjectType::STRING)
                            return Object();
                        return makeString(self->s_ptr->str.replace(
                                args[0]->s_ptr->str, args[1]->s_ptr->str
                        ));
                    }
                    return makeString(self->s_ptr->str.replace(args[0]->s_ptr->str));
                })

                smMethod(split, smLambda {
                    Object sep;
                    bool skipEmpty = true;
                    size_t argc = args.size();

                    if(argc == 0){
                        sep = makeString(" \t\n");
                    } else {
                        if(argc > 1){
                            skipEmpty = runtime::implicitToBool(args[1]);
                        }

                        if(args[0]->type != ObjectType::STRING)
                            return Object();
                        sep = args[0];
                    }

                    std::vector<String> strings = self->s_ptr->str.split(sep.s_ptr->str, skipEmpty);
                    RootObject list = makeList(intp, RootObjectVec_t(strings.size()));

                    ObjectVec_t& out = *getData<ObjectVec_t>(intp, list);
                    ObjectVec_t::iterator beg = out.begin(), end = out.end();
                    std::vector<String>::const_iterator src = strings.cbegin();

                    for(; beg != end; ++beg, ++src){
                        *beg = makeString(std::move(*src));
                    }
                    return list;
                })

                smMethod(trim, smLambda {
                    return makeString(self->s_ptr->str.trim());
                })

                smMethod(upper, smLambda {
                    return makeString(self->s_ptr->str.upper());
                })

                smMethod(lower, smLambda {
                    return makeString(self->s_ptr->str.lower());
                })

                smMethod(u_upper, smLambda {
                    return makeString(self->s_ptr->str.uUpper());
                })

                smMethod(u_lower, smLambda {
                    return makeString(self->s_ptr->str.uLower());
                })

                smMethod(clone, smLambda {
                    return makeString(self->s_ptr->str);
                })

                smMethod(to_string, smLambda {
                    return self;
                })

                smMethod(iterate, smLambda {
                    return newInstance(intp, cStringIterator, {self});
                })
            smEnd

            smClass(List)
                /*
                 *
                 *          888       d8b            888
                 *          888       Y8P            888
                 *          888                      888
                 *          888       888  .d8888b   888888
                 *          888       888  88K       888
                 *          888       888  "Y8888b.  888
                 *          888       888       X88  Y88b.
                 *          88888888  888   88888P'   "Y888
                 *
                */


                smMethod(new, smLambda {
                    smSetData(ObjectVec_t) = new ObjectVec_t();
                    return Object();
                })

                smMethod(delete, smLambda {
                    smDeleteData(ObjectVec_t);
                    return Object();
                })

                smIdMethod(runtime::gcCollectId, smLambda {
                    smDeleteData(ObjectVec_t);
                    return Object();
                })

                smGCSearch(smSearchLambda {
                    ObjectVec_t* ptr = data<ObjectVec_t>(self);
                    if(ptr){
                        out.insert(out.end(), ptr->begin(), ptr->end());
                    }
                })

                smIdMethod(runtime::squareId, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    if(args.empty() || vec.empty() || args[0]->type != ObjectType::INTEGER)
                        return Object();
                    integer_t i = args[0]->i;
                    integer_t sz = vec.size();

                    if(!runtime::findIndex(i, i, sz))
                        return Object();
                    return makeRef<true>(vec[i]);
                })

                smMethod(get, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    if(args.empty() || vec.empty() || args[0]->type != ObjectType::INTEGER)
                        return Object();
                    integer_t i = args[0]->i;
                    integer_t sz = vec.size();

                    if(!runtime::findIndex(i, i, sz))
                        return Object();

                    return vec[i];
                })

                smOpMethod(parse::TT_PLUS, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t* vec2;
                    if(args.empty() || !hasVector(intp, args[0], vec2))
                        return Object();

                    RootObject newVec = makeList(intp, RootObjectVec_t(vec.begin(), vec.end()));
                    ObjectVec_t& out = *getData<ObjectVec_t>(intp, newVec);
                    out.insert(out.end(), vec2->cbegin(), vec2->cend());
                    return newVec;
                })

                smOpMethod(parse::TT_MINUS, smLambda {
                    integer_t i;
                    if(args.empty() || args[0]->type != ObjectType::INTEGER
                            || (i = args[0]->i) < 0)
                        return Object();
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    size_t idx = i;
                    if(idx >= vec.size())
                        return makeList(intp);
                    return makeList(intp, RootObjectVec_t(vec.begin(), vec.end() - idx));
                })

                smOpMethod(parse::TT_MULT, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::INTEGER)
                        return Object();
                    integer_t i = args[0]->i;

                    RootObject list = makeList(intp);
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t& newVec = *getData<ObjectVec_t>(intp, list);
                    size_t sz = vec.size();

                    if(i == 0) {
                        return list;
                    } else if(i < 0){
                        i *= -1;
                        newVec.resize(sz * i);

                        ObjectVec_t::reverse_iterator beg = vec.rbegin();
                        ObjectVec_t::reverse_iterator end = vec.rend();
                        ObjectVec_t::iterator curr = newVec.begin();

                        for(integer_t j = 0; j != i; ++j){
                            curr = std::copy(beg, end, curr);
                        }
                    } else {
                        newVec.resize(sz * i);

                        ObjectVec_t::iterator beg = vec.begin();
                        ObjectVec_t::iterator end = vec.end();
                        ObjectVec_t::iterator curr = newVec.begin();

                        for(integer_t j = 0; j != i; ++j){
                            curr = std::copy(beg, end, curr);
                        }
                    }
                    return list;
                })

                smOpMethod(parse::TT_OR, smLambda {
                    ObjectVec_t* vec2;
                    if(args.empty() || !hasVector(intp, args[0], vec2))
                        return Object();

                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    RootObject newVec = makeList(intp, RootObjectVec_t(vec.begin(), vec.end()));
                    ObjectVec_t& out = *getData<ObjectVec_t>(intp, newVec);

                    for(const Object& obj : *vec2){
                        if(std::find_if(out.begin(), out.end(), runtime::Equal(intp, obj)) == out.end()){
                            out.push_back(obj);
                        }
                    }
                    return newVec;
                })

                smOpMethod(parse::TT_AND, smLambda {
                    ObjectVec_t* vec2;
                    if(args.empty() || !hasVector(intp, args[0], vec2))
                        return Object();

                    RootObject newVec = makeList(intp);
                    ObjectVec_t& out = *getData<ObjectVec_t>(intp, newVec);
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);

                    for(const Object& obj : *vec2){
                        if(std::find_if(vec.begin(), vec.end(), runtime::Equal(intp, obj)) != vec.end()){
                            out.push_back(obj);
                        }
                    }
                    return newVec;
                })

                smOpMethod(parse::TT_EQUAL, smLambda {
                    if(args.empty() || !runtime::of_type(args[0], cList))
                        return makeFalse();

                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t& vec2 = *getData<ObjectVec_t>(intp, args[0]);

                    if(vec.size() != vec2.size())
                        return makeFalse();

                    return std::equal(vec.begin(), vec.end(), vec2.begin(), runtime::BinaryEqual(intp))
                        ? makeTrue() : makeFalse();
                })

                smOpMethod(parse::TT_NOT_EQUAL, smLambda {
                    if(args.empty() || !runtime::of_type(args[0], cList))
                        return makeTrue();

                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t& vec2 = *getData<ObjectVec_t>(intp, args[0]);

                    if(vec.size() != vec2.size())
                        return makeTrue();

                    return std::equal(vec.begin(), vec.end(), vec2.begin(), runtime::BinaryEqual(intp))
                        ? makeFalse() : makeTrue();
                })

                smMethod(reserve, smLambda {
                    integer_t i;
                    if(args.empty() || args[0]->type != ObjectType::INTEGER
                            || (i = args[0]->i) < 0)
                        return Object();
                    smGetData(ObjectVec_t)->reserve(i);
                    return Object();
                })

                smMethod(resize, smLambda {
                    integer_t i;
                    if(args.empty() || args[0]->type != ObjectType::INTEGER
                            || (i = args[0]->i) < 0)
                        return Object();
                    smGetData(ObjectVec_t)->resize(i);
                    return Object();
                })

                smMethod(pop, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    if(!vec.empty()){
                        RootObject back = std::move(vec.back());
                        vec.pop_back();
                        return back;
                    }
                    return Object();
                })

                smMethod(push, smLambda {
                    smGetData(ObjectVec_t)->push_back(args.empty() ? RootObject() : args[0]);
                    return Object();
                })

                smMethod(pop_front, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    if(!vec.empty()){
                        RootObject back = std::move(vec.front());
                        vec.erase(vec.begin());
                        return back;
                    }
                    return Object();
                })

                smMethod(push_front, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    vec.insert(vec.begin(), args.empty() ? RootObject() : args[0]);
                    return Object();
                })

                smMethod(clone, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);

                    integer_t start = 0;
                    integer_t size = vec.size();
                    integer_t end = size;

                    if(args.empty() || args[0]->type == ObjectType::NONE);
                        // do nothing!
                    else if(args[0]->type == ObjectType::INTEGER){
                        start = args[0]->i;
                        if(!runtime::findIndex(start, start, size))
                            return Object();
                    } else return Object();

                    if(args.size() < 2 || args[1]->type == ObjectType::NONE);
                        // do nothing!
                    else if(args[1]->type == ObjectType::INTEGER){
                        end = args[1]->i;
                        if(end < 0){
                            end += size;
                            if(end < 0 || end < start)
                                return Object();
                        } else if(end > size)
                            return Object();
                        else if(end < start)
                            return Object();
                    } else return Object();

                    return makeList(intp, RootObjectVec_t(vec.cbegin() + start, vec.cbegin() + end));
                })

                smMethod(tuple, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);

                    integer_t start = 0;
                    integer_t size = vec.size();
                    integer_t end = size;

                    if(args.empty() || args[0]->type == ObjectType::NONE);
                        // do nothing!
                    else if(args[0]->type == ObjectType::INTEGER){
                        start = args[0]->i;
                        if(!runtime::findIndex(start, start, size))
                            return Object();
                    } else return Object();

                    if(args.size() < 2 || args[1]->type == ObjectType::NONE);
                        // do nothing
                    else if(args[1]->type == ObjectType::INTEGER){
                        end = args[1]->i;
                        if(end < 0){
                            end += size;
                            if(end < 0 || end < start)
                                return Object();
                        } else if(end > size)
                            return Object();
                        else if(end < start)
                            return Object();
                    } else return Object();

                    return makeTuple(intp, RootObjectVec_t(vec.cbegin() + start, vec.cbegin() + end));
                })

                smMethod(append, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t* toAppend;
                    if(args.empty() || !hasVector(intp, args[0], toAppend))
                        return Object();
                    vec.insert(vec.end(), toAppend->begin(), toAppend->end());
                    return Object();
                })

                smMethod(insert, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    if(args[0]->type != ObjectType::INTEGER)
                        return Object();
                    integer_t idx = args[0]->i;
                    if(!runtime::findIndex(idx, idx, vec.size()+1)) // vec.size() is a valid parameter.
                        return Object();
                    vec.insert(vec.begin() + idx, args[1]);
                    return Object();
                })

                smMethod(erase, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);

                    integer_t size = vec.size();
                    integer_t start = 0;
                    integer_t end;

                    if(args.empty() || args[0]->type == ObjectType::NONE);
                        // do nothing!
                    else if(args[0]->type == ObjectType::INTEGER){
                        start = args[0]->i;
                        if(!runtime::findIndex(start, start, size))
                            return Object();
                    } else return Object();

                    if(args.size() < 2 || args[1]->type == ObjectType::NONE) {
                        vec.erase(vec.begin() + start);
                        return Object();
                    } else if(args[1]->type == ObjectType::INTEGER){
                        end = args[1]->i;
                        if(end < 0){
                            end += size;
                            if(end < 0 || end < start)
                                return Object();
                        } else if(end > size){
                            vec.erase(vec.begin() + start, vec.end());
                            return Object();
                        } else if(end < start)
                            return Object();
                    } else return Object();

                    vec.erase(vec.begin() + start, vec.begin() + end);
                    return Object();
                })

                smMethod(insert_list, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t* toInsert;

                    if(args.size() < 2 || args[0]->type != ObjectType::INTEGER
                            || !hasVector(intp, args[1], toInsert))
                        return Object();

                    integer_t idx = args[0]->i;
                    if(!runtime::findIndex(idx, idx, vec.size()+1)) // vec.size() is a valid parameter.
                        return Object();
                    vec.insert(vec.begin() + idx, toInsert->begin(), toInsert->end());
                    return Object();
                })

                smMethod(copy_list, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t* toCopy;

                    if(args.size() < 2 || args[0]->type != ObjectType::INTEGER
                            || !hasVector(intp, args[1], toCopy))
                        return  makeFalse();

                    integer_t idx = args[0]->i;
                    if(!runtime::findIndex(idx, idx, vec.size()+1)) // vec.size() is a valid parameter.
                        return makeFalse();

                    size_t sz = toCopy->size() + idx;
                    if(vec.size() < sz)
                        vec.resize(sz);
                    std::copy(toCopy->begin(), toCopy->end(), vec.begin() + idx);
                    return makeTrue();
                })

                smMethod(find, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t::iterator begin = vec.begin(), end = vec.end();
                    ObjectVec_t::iterator found = std::find_if(vec.begin(), vec.end(),
                            runtime::Equal(intp, args.empty() ? RootObject() : args[0]));

                    if(found == end)
                        return Object();
                    return makeInteger(found - begin);
                })

                smMethod(count, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    return makeInteger(std::count_if(vec.begin(), vec.end(),
                        runtime::Equal(intp, args.empty() ? RootObject() : args[0])));
                })

                smMethod(slice, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    integer_t start = 0;
                    integer_t size = vec.size();
                    integer_t end = size;

                    if(args.empty() || args[0]->type == ObjectType::NONE);
                        // do nothing!
                    else if(args[0]->type == ObjectType::INTEGER){
                        start = args[0]->i;
                        if(!runtime::findIndex(start, start, size))
                            return Object();
                    } else return Object();

                    if(args.size() < 2 || args[1]->type == ObjectType::NONE);
                        // do nothing!
                    else if(args[1]->type == ObjectType::INTEGER){
                        end = args[1]->i;
                        if(end < 0){
                            end += size;
                            if(end < 0 || end < start)
                                return Object();
                        } else if(end > size)
                            return Object();
                        else if(end < start)
                            return Object();
                    } else return Object();

                    return makeList(intp, RootObjectVec_t(vec.cbegin() + start, vec.cbegin() + end));
                })

                smMethod(reverse, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t reversed(vec.rbegin(), vec.rend());
                    vec.swap(reversed);
                    return Object();
                })

                smMethod(sort, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    if(args.empty() || !runtime::implicitToBool(args[0]))
                        std::sort(vec.begin(), vec.end(), runtime::BinaryLess(intp));
                    else
                        std::sort(vec.rbegin(), vec.rend(), runtime::BinaryLess(intp));
                    return Object();
                })

                smMethod(unique, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    vec.erase(std::unique(vec.begin(), vec.end(),
                        runtime::BinaryEqual(intp)), vec.end());
                    return Object();
                })

                smMethod(to_string, smLambda {
                    Object str = makeString("[");
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t::const_iterator it = vec.cbegin();

                    if(it != vec.cend()){
                        while(1) {
                            Object str2 = runtime::implicitToString(intp, *it++);
                            str.s_ptr->str.append(str2.s_ptr->str);
                            if(it == vec.cend())
                                break;
                            str.s_ptr->str.append(", ");
                        }
                    }
                    str.s_ptr->str.push_back(']');
                    return str;
                })

                smMethod(empty, smLambda {
                    return makeInteger(smGetData(ObjectVec_t)->empty());
                })

                smMethod(size, smLambda {
                    return makeInteger(smGetData(ObjectVec_t)->size());
                })

                smMethod(iterate, smLambda {
                    return newInstance(intp, cListIterator, {self});
                })

            smEnd

            smClass(Tuple)
                /*
                 *
                 *          88888888888                   888
                 *              888                       888
                 *              888                       888
                 *              888   888  888  88888b.   888   .d88b.
                 *              888   888  888  888 "88b  888  d8P  Y8b
                 *              888   888  888  888  888  888  88888888
                 *              888   Y88b 888  888 d88P  888  Y8b.
                 *              888    "Y88888  88888P"   888   "Y8888
                 *                              888
                 *                              888
                 *                              888
                 *
                */

                smMethod(new, smLambda {
                    smSetData(ObjectVec_t) = new ObjectVec_t();
                    return Object();
                })

                smMethod(delete, smLambda {
                    smDeleteData(ObjectVec_t);
                    return Object();
                })

                smIdMethod(runtime::gcCollectId, smLambda {
                    smDeleteData(ObjectVec_t);
                    return Object();
                })

                smGCSearch(smSearchLambda {
                    ObjectVec_t* ptr = data<ObjectVec_t>(self);
                    if(ptr){
                        out.insert(out.end(), ptr->begin(), ptr->end());
                    }
                })

                smOpMethod(parse::TT_PLUS, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t* vec2;

                    if(args.empty() || !hasVector(intp, args[0], vec2))
                        return Object();

                    RootObject newVec = makeTuple(intp, RootObjectVec_t(vec.begin(), vec.end()));
                    ObjectVec_t& out = *getData<ObjectVec_t>(intp, newVec);
                    out.insert(out.end(), vec2->cbegin(), vec2->cend());

                    return newVec;
                })

                smOpMethod(parse::TT_MINUS, smLambda {
                    integer_t i;
                    if(args.empty() || args[0]->type != ObjectType::INTEGER
                            || (i = args[0]->i) < 0)
                        return Object();

                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    size_t idx = i;
                    if(idx >= vec.size())
                        return makeTuple(intp);
                    return makeTuple(intp, RootObjectVec_t(vec.begin(), vec.end() - idx));
                })

                smOpMethod(parse::TT_MULT, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::INTEGER)
                        return Object();
                    integer_t i = args[0]->i;

                    RootObject tuple = makeTuple(intp);
                    ObjectVec_t& newVec = *getData<ObjectVec_t>(intp, tuple);
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    size_t sz = vec.size();

                    if(i == 0) {
                        return tuple;
                    } else if(i < 0){
                        i *= -1;
                        newVec.resize(sz * i);

                        ObjectVec_t::reverse_iterator beg = vec.rbegin();
                        ObjectVec_t::reverse_iterator end = vec.rend();
                        ObjectVec_t::iterator curr = newVec.begin();

                        for(integer_t j = 0; j != i; ++j){
                            curr = std::copy(beg, end, curr);
                        }
                    } else {
                        newVec.resize(sz * i);

                        ObjectVec_t::iterator beg = vec.begin();
                        ObjectVec_t::iterator end = vec.end();
                        ObjectVec_t::iterator curr = newVec.begin();

                        for(integer_t j = 0; j != i; ++j){
                            curr = std::copy(beg, end, curr);
                        }
                    }
                    return tuple;
                })

                smOpMethod(parse::TT_OR, smLambda {
                    ObjectVec_t* vec2;
                    if(args.empty() || !hasVector(intp, args[0], vec2))
                        return Object();

                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    RootObject newVec = makeTuple(intp, RootObjectVec_t(vec.begin(), vec.end()));
                    ObjectVec_t& out = *getData<ObjectVec_t>(intp, newVec);

                    for(const Object& obj : *vec2){
                        if(std::find_if(out.begin(), out.end(), runtime::Equal(intp, obj)) == out.end()){
                            out.push_back(obj);
                        }
                    }
                    return newVec;
                })

                smOpMethod(parse::TT_AND, smLambda {
                    ObjectVec_t* vec2;
                    if(args.empty() || !hasVector(intp, args[0], vec2))
                        return Object();

                    RootObject newVec = makeTuple(intp, {});
                    ObjectVec_t& out = *getData<ObjectVec_t>(intp, newVec);
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);

                    for(const Object& obj : *vec2){
                        if(std::find_if(vec.begin(), vec.end(), runtime::Equal(intp, obj)) != vec.end()){
                            out.push_back(obj);
                        }
                    }
                    return newVec;
                })

                smOpMethod(parse::TT_EQUAL, smLambda {
                    if(args.empty() || !runtime::of_type(args[0], cTuple))
                        return makeFalse();

                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t& vec2 = *getData<ObjectVec_t>(intp, args[0]);

                    if(vec.size() != vec2.size())
                        return makeFalse();
                    return std::equal(vec.begin(), vec.end(), vec2.begin(), runtime::BinaryEqual(intp))
                        ? makeTrue() : makeFalse();
                })

                smOpMethod(parse::TT_NOT_EQUAL, smLambda {
                    if(args.empty() || !runtime::of_type(args[0], cTuple))
                        return makeTrue();

                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t& vec2 = *getData<ObjectVec_t>(intp, args[0]);

                    if(vec.size() != vec2.size())
                        return makeTrue();
                    return std::equal(vec.begin(), vec.end(), vec2.begin(), runtime::BinaryEqual(intp))
                        ? makeFalse() : makeTrue();
                })

                smMethod(slice, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);

                    integer_t start = 0;
                    integer_t size = vec.size();
                    integer_t end = size;

                    if(args.empty() || args[0]->type == ObjectType::NONE);
                        // do nothing
                    else if(args[0]->type == ObjectType::INTEGER){
                        start = args[0]->i;
                        if(!runtime::findIndex(start, start, size))
                            return Object();
                    } else return Object();

                    if(args.size() < 2 || args[1]->type == ObjectType::NONE);
                        // do nothing
                    else if(args[1]->type == ObjectType::INTEGER){
                        end = args[1]->i;
                        if(end < 0){
                            end += size;
                            if(end < 0 || end < start)
                                return Object();
                        } else if(end > size)
                            return Object();
                        else if(end < start)
                            return Object();
                    } else return Object();

                    return makeTuple(intp, RootObjectVec_t(vec.cbegin() + start, vec.cbegin() + end));
                })

                smMethod(get, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    if(vec.empty() || args.empty() || args[0]->type != ObjectType::INTEGER)
                        return Object();

                    integer_t i = args[0]->i;
                    integer_t sz = vec.size();

                    if(!runtime::findIndex(i, i, sz))
                        return Object();
                    return vec[i];
                })

                smMethod(list, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);

                    integer_t start = 0;
                    integer_t size = vec.size();
                    integer_t end = size;

                    if(args.empty() || args[0]->type == ObjectType::NONE);
                        // do nothing
                    else if(args[0]->type == ObjectType::INTEGER){
                        start = args[0]->i;
                        if(!runtime::findIndex(start, start, size))
                            return Object();
                    } else return Object();

                    if(args.size() < 2 || args[1]->type == ObjectType::NONE);
                        // do nothing
                    else if(args[1]->type == ObjectType::INTEGER){
                        end = args[1]->i;
                        if(end < 0){
                            end += size;
                            if(end < 0 || end < start)
                                return Object();
                        } else if(end > size)
                            return Object();
                        else if(end < start)
                            return Object();
                    } else return Object();

                    return makeList(intp, RootObjectVec_t(vec.cbegin() + start, vec.cbegin() + end));
                })

                smMethod(hash, smLambda {
                    // TODO implement a real hash algorithm instead of a sum!
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    size_t hash = 0;
                    ObjectHash hasher(*intp.rt);

                    for(const Object& obj : vec){
                        hash += hasher(obj);
                    }
                    return makeInteger(hash);
                })

                smMethod(size, smLambda {
                    return makeInteger(smGetData(ObjectVec_t)->size());
                })

                smMethod(empty, smLambda {
                    return makeInteger(smGetData(ObjectVec_t)->empty());
                })

                smMethod(clone, smLambda {
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);

                    integer_t start = 0;
                    integer_t size = vec.size();
                    integer_t end = size;

                    if(args.empty() || args[0]->type == ObjectType::NONE);
                        // do nothing!
                    else if(args[0]->type == ObjectType::INTEGER){
                        start = args[0]->i;
                        if(!runtime::findIndex(start, start, size))
                            return Object();
                    } else return Object();

                    if(args.size() < 2 || args[1]->type == ObjectType::NONE);
                        // do nothing!
                    else if(args[1]->type == ObjectType::INTEGER){
                        end = args[1]->i;
                        if(end < 0){
                            end += size;
                            if(end < 0 || end < start)
                                return Object();
                        } else if(end > size)
                            return Object();
                        else if(end < start)
                            return Object();
                    } else return Object();

                    return makeTuple(intp, RootObjectVec_t(vec.cbegin() + start, vec.cbegin() + end));
                })

                smMethod(to_string, smLambda{
                    Object str = makeString("(");
                    ObjectVec_t& vec = *smGetData(ObjectVec_t);
                    ObjectVec_t::const_iterator it = vec.cbegin();
                    if(it != vec.cend()){
                        while(1) {
                            Object str2 = runtime::implicitToString(intp, *it++);
                            str.s_ptr->str.append(str2.s_ptr->str);
                            if(it == vec.cend())
                                break;
                            str.s_ptr->str.append(", ");
                        }
                    }
                    str.s_ptr->str.push_back(')');
                    return str;
                })

                smMethod(iterate, smLambda {
                    return newInstance(intp, cListIterator, {self});
                })

            smEnd

            smClass(Table)
                /*
                 *
                 *        88888888888       888      888
                 *            888           888      888
                 *            888           888      888
                 *            888   8888b.  88888b.  888  .d88b.
                 *            888      "88b 888 "88b 888 d8P  Y8b
                 *            888  .d888888 888  888 888 88888888
                 *            888  888  888 888 d88P 888 Y8b.
                 *            888  "Y888888 88888P"  888  "Y8888
                 *
                 *
                */
                smMethod(new, smLambda {
                    smSetData(Table_t) = new Table_t(12, ObjectHash(*intp.rt), EqualTo(*intp.rt));
                    return Object();
                })

                smMethod(delete, smLambda {
                    smDeleteData(Table_t);
                    return Object();
                })

                smIdMethod(runtime::gcCollectId, smLambda {
                    smDeleteData(Table_t);
                    return Object();
                })

                smIdMethod(runtime::squareId, smLambda {
                    RootObject key = args.empty() ? RootObject() : args[0];
                    return makeRef<true>(smGetData(Table_t)->operator[](key));
                })

                smMethod(get, smLambda {
                    RootObject key = args.empty() ? RootObject() : args[0];
                    return smGetData(Table_t)->operator[](key);
                })

                smMethod(set, smLambda {
                    size_t sz = args.size();
                    RootObject key, value;

                    if(sz != 0){
                        key = args[0];
                        if(sz > 1)
                            value = args[1];
                    }

                    smGetData(Table_t)->operator[](key) = value;
                    return Object();
                })

                smMethod(erase, smLambda {
                    RootObject key = args.empty() ? RootObject() : args[0];
                    smGetData(Table_t)->erase(key);
                    return Object();
                })

                smMethod(has, smLambda {
                    Table_t* ptr = smGetData(Table_t);
                    RootObject key = args.empty() ? RootObject() : args[0];
                    return makeBool(ptr->find(key) != ptr->end());
                })

                smMethod(reserve, smLambda {
                    Table_t* ptr = smGetData(Table_t);
                    if(args.empty() || args[0]->type != ObjectType::INTEGER
                            || args[0]->i <= static_cast<integer_t>(ptr->size()))
                        return Object();
                    ptr->reserve(args[0]->i);
                    return Object();
                })

                smMethod(empty, smLambda {
                    return makeBool(smGetData(Table_t)->empty());
                })

                smMethod(size, smLambda {
                    return makeInteger(smGetData(Table_t)->size());
                })

                smOpMethod(parse::TT_EQUAL, smLambda {
                    if(args.empty() || !runtime::of_type(args[0], cTable))
                        return Object();

                    Table_t* tableA = smGetData(Table_t),
                        *tableB = data<Table_t>(args[0]);

                    runtime::BinaryEqual comp (intp);
                    return makeBool(tableA->size() == tableB->size() &&
                        std::equal(tableA->begin(), tableA->end(), tableB->begin(),
                            [comp](const Table_t::value_type& lhs, const Table_t::value_type& rhs) -> bool {
                                return comp(lhs.first, rhs.first) && comp(lhs.second, rhs.second);
                            }
                        ));
                })

                smOpMethod(parse::TT_NOT_EQUAL, smLambda {
                    if(args.empty() || !runtime::of_type(args[0], cTable))
                        return Object();

                    Table_t* tableA = smGetData(Table_t),
                        *tableB = data<Table_t>(args[0]);

                    runtime::BinaryEqual comp (intp);
                    return makeBool(tableA->size() != tableB->size() ||
                        !std::equal(tableA->begin(), tableA->end(), tableB->begin(),
                            [comp](const Table_t::value_type& lhs, const Table_t::value_type& rhs) -> bool {
                                return comp(lhs.first, rhs.first) && comp(lhs.second, rhs.second);
                            }
                        ));
                })

                smMethod(keys, smLambda {
                    Table_t* ptr = smGetData(Table_t);
                    RootObjectVec_t vec;
                    vec.reserve(ptr->size());

                    for(auto it = ptr->begin(); it != ptr->end(); ++it)
                        vec.push_back(it->first);
                    return makeList(intp, std::move(vec));
                })

                smMethod(values, smLambda {
                    Table_t* ptr = smGetData(Table_t);
                    RootObjectVec_t vec;
                    vec.reserve(ptr->size());

                    for(auto it = ptr->begin(); it != ptr->end(); ++it)
                        vec.emplace_back(it->second);
                    return makeList(intp, std::move(vec));
                })

                smMethod(list, smLambda {
                    Table_t* ptr = smGetData(Table_t);
                    RootObjectVec_t vec;
                    vec.reserve(ptr->size());

                    for(auto it = ptr->begin(); it != ptr->end(); ++it)
                        vec.emplace_back(makeList(intp, RootObjectVec_t{it->first, it->second}));
                    return makeList(intp, std::move(vec));
                })
            smEnd

            smClass(ListIterator)
                /*
                 *
                 *        888       d8b            888     8888888  888                                888
                 *        888       Y8P            888       888    888                                888
                 *        888                      888       888    888                                888
                 *        888       888  .d8888b   888888    888    888888  .d88b.   888d888  8888b.   888888  .d88b.   888d888
                 *        888       888  88K       888       888    888    d8P  Y8b  888P"       "88b  888    d88""88b  888P"
                 *        888       888  "Y8888b.  888       888    888    88888888  888     .d888888  888    888  888  888
                 *        888       888       X88  Y88b.     888    Y88b.  Y8b.      888     888  888  Y88b.  Y88..88P  888
                 *        88888888  888   88888P'   "Y888  8888888   "Y888  "Y8888   888     "Y888888   "Y888  "Y88P"   888
                 *
                */

                smMethod(new, smLambda {
                    ObjectVec_t* dummy;
                    if(args.empty() || !hasVector(intp, args[0], dummy))
                        return Object();

                    smRef(smId("list")) = args[0];
                    smRef(smId("idx")) = makeInteger(0);
                    return Object();
                })

                smMethod(delete, smLambda {
                    return Object();
                })

                smMethod(next, smLambda {
                    Object& list = smRef(smId("list"));
                    Object& idx = smRef(smId("idx"));

                    if(idx.type != ObjectType::INTEGER){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                            "'idx' is not an integer inside ListIterator");
                    }

                    ObjectVec_t* ref;
                    if(!hasVector(intp, list, ref)){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                            "'list' is not an integer inside ListIterator");
                    }

                    integer_t& i = idx.i;
                    bool check = i < static_cast<integer_t>(ref->size());
                    RootObject obj;

                    if(check){
                        obj = (*ref)[i++];
                    }

                    return makeTuple(intp, RootObjectVec_t{obj, makeBool(check)});
                })
            smEnd

            smClass(StringIterator)
                /*
                 *
                 *        .d8888b.   888              d8b                      8888888  888                                888
                 *       d88P  Y88b  888              Y8P                        888    888                                888
                 *       Y88b.       888                                         888    888                                888
                 *        "Y888b.    888888  888d888  888  88888b.    .d88b.     888    888888  .d88b.   888d888  8888b.   888888  .d88b.   888d888
                 *           "Y88b.  888     888P"    888  888 "88b  d88P"88b    888    888    d8P  Y8b  888P"       "88b  888    d88""88b  888P"
                 *             "888  888     888      888  888  888  888  888    888    888    88888888  888     .d888888  888    888  888  888
                 *       Y88b  d88P  Y88b.   888      888  888  888  Y88b 888    888    Y88b.  Y8b.      888     888  888  Y88b.  Y88..88P  888
                 *        "Y8888P"    "Y888  888      888  888  888   "Y88888  8888888   "Y888  "Y8888   888     "Y888888   "Y888  "Y88P"   888
                 *                                                        888
                 *                                                   Y8b d88P
                 *                                                    "Y88P"
                */

                smMethod(new, smLambda {
                    if(args.empty() || args[0]->type != ObjectType::STRING)
                        return Object();
                    smRef(smId("str")) = args[0];
                    smRef(smId("idx")) = makeInteger(0);
                    return Object();
                })

                smMethod(delete, smLambda {
                    return Object();
                })

                smMethod(next, smLambda {
                    Object& str = smRef(smId("str"));
                    Object& idx = smRef(smId("idx"));

                    if(str.type != ObjectType::STRING){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                            "'str' is not a string inside StringIterator");
                    }

                    if(idx.type != ObjectType::INTEGER){
                        intp.rt->sources.printStackTrace(intp, error::ET_ERROR,
                            "'idx' is not an integer inside StringIterator");
                    }

                    String& ref = str.s_ptr->str;
                    integer_t& i = idx.i;

                    String::const_iterator it = ref.begin() + i;
                    unicode_t ch;
                    bool check = i < static_cast<integer_t>(ref.size());

                    if(check && String::uNext(it, ref.end(), ch)){
                        i = it - ref.begin();
                        return makeTuple(intp, {
                            makeString(ch), makeTrue()
                        });
                    }

                    return makeTuple(intp, {Object(), makeFalse()});
                })
            smEnd

            smReturnBox
        }
    }

    RootObject makeList(exec::Interpreter& intp, RootObjectVec_t vec) noexcept{
        RootObject list = newInstance(intp, lib::cList);
        lib::getData<ObjectVec_t>(intp, list)->assign(vec.begin(), vec.end());
        return list;
    }

    RootObject makeTuple(exec::Interpreter& intp, RootObjectVec_t vec) noexcept{
        RootObject tuple = newInstance(intp, lib::cTuple);
        lib::getData<ObjectVec_t>(intp, tuple)->assign(vec.begin(),vec.end());
        return tuple;
    }

    bool hasVector(exec::Interpreter& intp, const Object& obj, ObjectVec_t*& vecPtr) noexcept{
        if(runtime::of_type(obj, lib::cList) || runtime::of_type(obj, lib::cTuple)){
            vecPtr = lib::getData<ObjectVec_t>(intp, obj);
            return true;
        }
        return false;
    }
}
