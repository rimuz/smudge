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

#include "utils/unicode/utf8.h"
#include "lib/stdlib.h"

#include "runtime/casts.h"
#include "runtime/Object.h"
#include "runtime/utils.h"

namespace sm{
    namespace lib{
        Class* cString = nullptr;
        Class* cList = nullptr;

        oid_t idToString;

        namespace StringClass {
            _NativeFunc(idx);
            _NativeFunc(len);
            _NativeFunc(count);
            _NativeFunc(empty);
            _NativeFunc(compare);
            _NativeFunc(u_compare);
            _NativeFunc(equal);
            _NativeFunc(not_equal);
            _NativeFunc(less);
            _NativeFunc(greater);
            _NativeFunc(lessOrEqual);
            _NativeFunc(greaterOrEqual);
            _NativeFunc(plus);
            _NativeFunc(minus);
            _NativeFunc(mul);
            _NativeFunc(get);
            _NativeFunc(u_get);
            _NativeFunc(getc);
            _NativeFunc(u_getc);
            _NativeFunc(contains);
            _NativeFunc(bytes);
            _NativeFunc(chars);
            _NativeFunc(join);
            _NativeFunc(ends_with);
            _NativeFunc(starts_with);
            _NativeFunc(hash);
            _NativeFunc(find);
            _NativeFunc(find_last);
            _NativeFunc(substr);
            _NativeFunc(u_substr);
            _NativeFunc(replace_first);
            _NativeFunc(replace);
            _NativeFunc(split);
            _NativeFunc(trim);
            _NativeFunc(upper);
            _NativeFunc(lower);
            _NativeFunc(u_upper);
            _NativeFunc(u_lower);
            _NativeFunc(clone);
            _NativeFunc(to_string);
        }

        namespace ListClass {
            class List : public Instance {
            public:
                ObjectVec_t vec;

                template <class... Tp>
                List(runtime::GarbageCollector& gc, bool temp, Tp&&...args)
                    : Instance(gc, temp), vec(std::forward<Tp>(args)...){}

                _NativeMethod(bracing, 1);
                _NativeMethod(plus, 1);
                _NativeMethod(bitor_op, 1);
                _NativeMethod(bitand_op, 1);
                _NativeMethod(equal_op, 1);

                _NativeMethod(reserve, 1);
                _NativeMethod(resize, 1);
                _NativeMethod(pop, 0);
                _NativeMethod(push, 1);
                _NativeMethod(clone, 2);
                _NativeMethod(tuple, 2);
                _NativeMethod(append, 1);
                _NativeMethod(insert, 2);
                _NativeMethod(insert_list, 2);
                _NativeMethod(copy_list, 2);
                _NativeMethod(reverse, 2);
                _NativeMethod(sort, 2);
                _NativeMethod(unique, 2);
                _NativeMethod(to_string, 0);
            };

            _NativeFunc(bracing);
            _NativeFunc(plus);
            _NativeFunc(bitor_op);
            _NativeFunc(bitand_op);
            _NativeFunc(equal_op);

            _NativeFunc(reserve);
            _NativeFunc(resize);
            _NativeFunc(pop);
            _NativeFunc(push);
            _NativeFunc(clone);
            _NativeFunc(tuple);
            _NativeFunc(append);
            _NativeFunc(insert);
            _NativeFunc(insert_list);
            _NativeFunc(copy_list);
            _NativeFunc(reverse);
            _NativeFunc(sort);
            _NativeFunc(unique);
            _NativeFunc(to_string);
        }

        _LibDecl(lang){
            Class* box = new Class;
            box->boxName = nBox;

            idToString = _Id("to_string");

            cString = setClass(rt, box, "String", {
                _OpTuple(StringClass, parse::TT_PLUS, plus),
                _OpTuple(StringClass, parse::TT_MINUS, minus),
                _OpTuple(StringClass, parse::TT_MULT, mul),
                _OpTuple(StringClass, parse::TT_EQUAL, equal),
                _OpTuple(StringClass, parse::TT_EQUAL, not_equal),
                _OpTuple(StringClass, parse::TT_LESS, less),
                _OpTuple(StringClass, parse::TT_GREATER, greater),
                _OpTuple(StringClass, parse::TT_LESS_OR_EQUAL, lessOrEqual),
                _OpTuple(StringClass, parse::TT_GREATER_OR_EQUAL, greaterOrEqual),

                _MethodTuple(StringClass, idx),
                _MethodTuple(StringClass, len),
                _MethodTuple(StringClass, count),
                _MethodTuple(StringClass, empty),
                _MethodTuple(StringClass, compare),
                _MethodTuple(StringClass, u_compare),
                _MethodTuple(StringClass, get),
                _MethodTuple(StringClass, u_get),
                _MethodTuple(StringClass, getc),
                _MethodTuple(StringClass, u_getc),
                _MethodTuple(StringClass, contains),
                _MethodTuple(StringClass, bytes),
                _MethodTuple(StringClass, chars),
                _MethodTuple(StringClass, join),
                _MethodTuple(StringClass, ends_with),
                _MethodTuple(StringClass, starts_with),
                _MethodTuple(StringClass, hash),
                _MethodTuple(StringClass, find),
                _MethodTuple(StringClass, find_last),
                _MethodTuple(StringClass, substr),
                _MethodTuple(StringClass, u_substr),
                _MethodTuple(StringClass, replace_first),
                _MethodTuple(StringClass, replace),
                _MethodTuple(StringClass, split),
                _MethodTuple(StringClass, trim),
                _MethodTuple(StringClass, upper),
                _MethodTuple(StringClass, lower),
                _MethodTuple(StringClass, u_upper),
                _MethodTuple(StringClass, u_lower),
                _MethodTuple(StringClass, clone),
                _MethodTuple(StringClass, to_string),
            });

            cList = setClass(rt, box, "List", {
                _OpTuple(ListClass, parse::TT_SQUARE_OPEN, bracing),
                _OpTuple(ListClass, parse::TT_PLUS, plus),
                _OpTuple(ListClass, parse::TT_OR, bitor_op),
                _OpTuple(ListClass, parse::TT_AND, bitand_op),
                _OpTuple(ListClass, parse::TT_EQUAL, equal_op),

                _MethodTuple(ListClass, reserve),
                _MethodTuple(ListClass, resize),
                _MethodTuple(ListClass, pop),
                _MethodTuple(ListClass, push),
                _MethodTuple(ListClass, clone),
                _MethodTuple(ListClass, tuple),
                _MethodTuple(ListClass, append),
                _MethodTuple(ListClass, insert),
                _MethodTuple(ListClass, insert_list),
                _MethodTuple(ListClass, copy_list),
                _MethodTuple(ListClass, reverse),
                _MethodTuple(ListClass, sort),
                _MethodTuple(ListClass, to_string),
                _MethodTuple(ListClass, unique),
            });

            return box;
        }

        namespace StringClass {
            _NativeFunc(idx){
                if(args.empty() || args[0].type != ObjectType::INTEGER)
                    return Object();
                integer_t i = args[0].i;
                String::iterator begin = self.s_ptr->str.begin(),
                    end = self.s_ptr->str.end();
                if(i > 0){
                    String::iterator ch_it = String::nthChar(begin, end, i);
                    return makeInteger(std::distance(begin, ch_it));
                } else if(i < 0){
                    integer_t n = self.s_ptr->str.uSize() + i;
                    if(n <= 0)
                        return Object();
                    String::iterator ch_it = String::nthChar(begin, end, n);
                    return makeInteger(std::distance(begin, ch_it));
                }
                return makeInteger(0);
            }

            _NativeFunc(len){
                return makeInteger(self.s_ptr->str.size());
            }

            _NativeFunc(count){
                size_t argc = args.size();

                integer_t start = 0;
                integer_t size = self.s_ptr->str.size();
                integer_t end = size;

                if(argc != 0){
                    if(args[0].type == ObjectType::INTEGER){
                        start = args[0].i;
                        if(!runtime::findIndex(start, start, size))
                            return Object();
                    } else if(args[0].type != ObjectType::NONE)
                        return Object();
                }

                if(argc > 1){
                    if(args[1].type == ObjectType::INTEGER){
                        end = args[1].i;
                        if(end < 0){
                            end += size;
                            if(end < 0 || end < start)
                                return Object();
                        } else if(end > size)
                            end = size;
                    } else if(args[0].type != ObjectType::NONE)
                        return Object();
                }

                String::const_iterator beg = self.s_ptr->str.begin();
                return makeInteger(String::uSize(beg + start, beg + end));
            }

            _NativeFunc(empty){
                return makeInteger(self.s_ptr->str.empty());
            }

            _NativeFunc(compare){
                size_t argc = args.size();
                if(argc == 0 || args[0].type != ObjectType::STRING){
                    return makeInteger(0);
                } else if(argc >= 2 && runtime::implicitToBool(args[1])){
                    return makeInteger(self.s_ptr->str.compareIgnoreCase(args[0].s_ptr->str));
                }
                return makeInteger(self.s_ptr->str.compare(args[0].s_ptr->str));
            }

            _NativeFunc(u_compare){
                size_t argc = args.size();
                if(argc == 0 || args[0].type != ObjectType::STRING){
                    return makeInteger(0);
                } else if(argc >= 2 && runtime::implicitToBool(args[1])){
                    return makeInteger(self.s_ptr->str.uCompareIgnoreCase(args[0].s_ptr->str));
                }
                return makeInteger(self.s_ptr->str.compare(args[0].s_ptr->str));
            }

            _NativeFunc(equal){
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return makeInteger(0);
                return makeInteger(self.s_ptr->str == args[0].s_ptr->str);
            }

            _NativeFunc(not_equal){
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return makeInteger(0);
                return makeInteger(self.s_ptr->str != args[0].s_ptr->str);
            }

            _NativeFunc(less){
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return makeInteger(0);
                return makeInteger(self.s_ptr->str < args[0].s_ptr->str);
            }

            _NativeFunc(greater){
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return makeInteger(0);
                return makeInteger(self.s_ptr->str > args[0].s_ptr->str);
            }

            _NativeFunc(lessOrEqual){
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return makeInteger(0);
                return makeInteger(self.s_ptr->str <= args[0].s_ptr->str);
            }

            _NativeFunc(greaterOrEqual){
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return makeInteger(0);
                return makeInteger(self.s_ptr->str >= args[0].s_ptr->str);
            }

            _NativeFunc(plus){
                if(args.empty())
                    return Object();
                Object str0 = makeString(self.s_ptr->str);
                Object str1 = runtime::implicitToString(intp, args[0]);
                str0.s_ptr->str.insert(str0.s_ptr->str.end(), str1.s_ptr->str.begin(),
                    str1.s_ptr->str.end());
                return str0;
            }

            _NativeFunc(minus){
                integer_t i;
                if(args.empty() || args[0].type != ObjectType::INTEGER || (i = args[0].i) < 0)
                    return Object();
                if(i >= static_cast<integer_t>(self.s_ptr->str.size()))
                    return Object();
                return makeString(self.s_ptr->str.begin(), self.s_ptr->str.end() - i);
            }

            _NativeFunc(mul){
                if(args.empty() || args[0].type != ObjectType::INTEGER)
                    return Object();
                integer_t i = args[0].i;

                Object str0 = makeString(self.s_ptr->str);
                size_t sz = str0.s_ptr->str.size();
                if(i == 0) {
                    return str0;
                } else if(i < 0){
                    i *= -1;
                    str0.s_ptr->str.resize(sz * i);

                    String::reverse_iterator beg = self.s_ptr->str.rbegin();
                    String::reverse_iterator end = self.s_ptr->str.rend();
                    String::iterator curr = str0.s_ptr->str.begin();

                    for(integer_t j = 0; j != i; ++j){
                        curr = std::copy(beg, end, curr);
                    }
                } else {
                    str0.s_ptr->str.resize(sz * i);

                    String::iterator beg = str0.s_ptr->str.begin();
                    String::iterator end = str0.s_ptr->str.begin() + sz;
                    String::iterator curr = end;

                    for(integer_t j = 1; j != i; ++j){
                        curr = std::copy(beg, end, curr);
                    }
                }
                return str0;
            }

            _NativeFunc(get){
                String::const_iterator begin = self.s_ptr->str.cbegin();

                if(self.s_ptr->str.empty())
                    return Object();
                if(args.empty()){
                    return makeString(begin, begin+1);
                } else if(args[0].type == ObjectType::INTEGER){
                    integer_t idx = args[0].i;
                    if(idx >= 0){
                        size_t i = static_cast<size_t>(idx);
                        if(i >= self.s_ptr->str.size())
                            return Object();
                        return makeString(begin+i, begin+i+1);
                    }

                    idx += static_cast<integer_t>(self.s_ptr->str.size());
                    if(idx < 0)
                        return Object();
                    return makeString(begin+idx, begin+idx+1);
                }
                return Object();
            }

            _NativeFunc(u_get){
                if(self.s_ptr->str.empty())
                    return Object();
                integer_t strSize = self.s_ptr->str.uSize();
                if(args.empty()){
                    return makeString(self.s_ptr->str.uCharAt(0));
                } else if(args[0].type == ObjectType::INTEGER){
                    integer_t idx = args[0].i;
                    if(idx >= 0){
                        if(idx >= strSize)
                            return Object();
                        return makeString(self.s_ptr->str.uCharAt(idx));
                    }

                    idx += strSize;
                    if(idx < 0)
                        return Object();
                    return makeString(self.s_ptr->str.uCharAt(idx));
                }
                return Object();
            }

            _NativeFunc(getc){
                if(self.s_ptr->str.empty())
                    return Object();
                if(args.empty()){
                    return makeInteger(self.s_ptr->str[0]);
                } else if(args[0].type == ObjectType::INTEGER){
                    integer_t idx = args[0].i;
                    if(idx >= 0){
                        size_t i = static_cast<size_t>(idx);
                        if(i >= self.s_ptr->str.size())
                            return Object();
                        return makeInteger(self.s_ptr->str[i]);
                    }

                    idx += static_cast<integer_t>(self.s_ptr->str.size());
                    if(idx < 0)
                        return Object();
                    return makeInteger(self.s_ptr->str[static_cast<size_t>(idx)]);
                }
                return Object();
            }

            _NativeFunc(u_getc){
                if(self.s_ptr->str.empty())
                    return Object();
                integer_t strSize = self.s_ptr->str.uSize();
                if(args.empty()){
                    return makeInteger(uGetCodepoint(self.s_ptr->str.uCharAt(0)));
                } else if(args[0].type == ObjectType::INTEGER){
                    integer_t idx = args[0].i;
                    if(idx >= 0){
                        if(idx >= strSize)
                            return Object();
                        return makeInteger(uGetCodepoint(self.s_ptr->str.uCharAt(idx)));
                    }

                    idx += strSize;
                    if(idx < 0)
                        return Object();
                    return makeInteger(uGetCodepoint(self.s_ptr->str.uCharAt(idx)));
                }
                return Object();
            }

            _NativeFunc(contains){
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return Object();
                return makeInteger(self.s_ptr->str.contains(args[0].s_ptr->str));
            }

            _NativeFunc(bytes){
                Object list = makeList(intp.rt->gc, false, ObjectVec_t(self.s_ptr->str.size()));

                String::const_iterator iit = self.s_ptr->str.begin();
                ObjectVec_t& out = reinterpret_cast<ListClass::List*>(list.i_ptr)->vec;
                ObjectVec_t::iterator oit = out.begin(), end = out.end();

                while(oit != end){
                    *oit++ = makeInteger(static_cast<unsigned char>(*iit++));
                }
                return list;
            }

            _NativeFunc(chars){
                Object list = makeList(intp.rt->gc, false);

                String::const_iterator curr = self.s_ptr->str.begin();
                String::const_iterator end = self.s_ptr->str.end();
                ObjectVec_t& out = reinterpret_cast<ListClass::List*>(list.i_ptr)->vec;
                unicode_t ch;

                while(String::uNext(curr, end, ch)){
                    out.push_back(makeInteger(uGetCodepoint(ch)));
                }
                return list;
            }

            _NativeFunc(join){
                if(args.empty())
                    return makeString();
                if(!runtime::of_type(args[0], cList))
                    return Object();
                Object out = makeString();
                ObjectVec_t& in = reinterpret_cast<ListClass::List*>(args[0].i_ptr)->vec;
                ObjectVec_t::const_iterator curr = in.cbegin(), end = in.cend();
                if(curr != end){
                    while(1){
                        Object str = runtime::implicitToString(intp, *curr);
                        out.s_ptr->str.append(str.s_ptr->str);
                        if(++curr == end)
                            break;
                        else
                            out.s_ptr->str.append(self.s_ptr->str);
                    }
                }
                return out;
            }

            _NativeFunc(ends_with){
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return Object();
                return makeInteger(self.s_ptr->str.endsWith(args[0].s_ptr->str));
            }

            _NativeFunc(starts_with){
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return Object();
                return makeInteger(self.s_ptr->str.startsWith(args[0].s_ptr->str));
            }

            _NativeFunc(hash){
                return makeInteger(self.s_ptr->str.hash());
            }

            _NativeFunc(find){
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return Object();
                return makeInteger(self.s_ptr->str.find(args[0].s_ptr->str)
                    - self.s_ptr->str.begin());
            }

            _NativeFunc(find_last){
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return Object();
                return makeInteger(self.s_ptr->str.findLast(args[0].s_ptr->str)
                    - self.s_ptr->str.begin());
            }

            _NativeFunc(substr){
                size_t argc = args.size();

                if(argc == 0 || args[0].type != ObjectType::INTEGER){
                    return Object();
                }

                integer_t size = self.s_ptr->str.size();
                integer_t start = args[0].i;
                String::iterator begin = self.s_ptr->str.begin();

                if(start < 0){
                    start += size;
                    if(start < 0)
                        return Object();
                } else if(start >= size)
                    return Object();

                if(argc >= 2){
                    integer_t end = args[1].i;
                    if(end < 0){
                        end += size;
                        if(end < 0 || end < start)
                            return Object();
                    } else if(end > size)
                        return Object();
                    String::iterator begin = self.s_ptr->str.begin();
                    return makeString(begin + start, begin + end);
                }

                return makeString(begin + start, begin + size);
            }

            _NativeFunc(u_substr){
                size_t argc = args.size();

                if(argc == 0 || args[0].type != ObjectType::INTEGER){
                    return Object();
                }

                integer_t size = self.s_ptr->str.uSize();
                integer_t start = args[0].i;

                if(!runtime::findIndex(start, start, size))
                    return Object();

                if(argc >= 2){
                    integer_t end = args[1].i;
                    if(end < 0){
                        end += size;
                        if(end < 0 || end < start)
                            return Object();
                    } else if(end > size)
                        return Object();
                    return makeString(self.s_ptr->str.uSubstr(start, end));
                }

                return makeString(self.s_ptr->str.uSubstr(start));
            }

            _NativeFunc(replace_first){
                size_t argc = args.size();
                if(argc == 0 || args[0].type != ObjectType::STRING){
                    return self;
                } else if(argc >= 2){
                    if(args[1].type != ObjectType::STRING)
                        return Object();
                    return makeString(self.s_ptr->str.replaceFirst(
                            args[0].s_ptr->str, args[1].s_ptr->str
                    ));
                }
                return makeString(self.s_ptr->str.replaceFirst(args[0].s_ptr->str));
            }

            _NativeFunc(replace){
                size_t argc = args.size();
                if(argc == 0 || args[0].type != ObjectType::STRING){
                    return self;
                } else if(argc >= 2){
                    if(args[1].type != ObjectType::STRING)
                        return Object();
                    return makeString(self.s_ptr->str.replace(
                            args[0].s_ptr->str, args[1].s_ptr->str
                    ));
                }
                return makeString(self.s_ptr->str.replace(args[0].s_ptr->str));
            }

            _NativeFunc(split){
                Object sep;
                bool skipEmpty = true;
                size_t argc = args.size();

                if(argc == 0){
                    sep = makeString(" \t\n");
                } else {
                    if(argc > 1){
                        skipEmpty = runtime::implicitToBool(args[1]);
                    }

                    if(args[0].type != ObjectType::STRING)
                        return Object();
                    sep = args[0];
                }

                std::vector<String> strings = self.s_ptr->str.split(sep.s_ptr->str, skipEmpty);
                Object list = makeList(intp.rt->gc, false, ObjectVec_t(strings.size()));

                ObjectVec_t& out = reinterpret_cast<ListClass::List*>(list.i_ptr)->vec;
                ObjectVec_t::iterator beg = out.begin(), end = out.end();
                std::vector<String>::const_iterator src = strings.cbegin();

                for(; beg != end; ++beg, ++src){
                    *beg = makeString(std::move(*src));
                }
                return list;
            }

            _NativeFunc(trim){
                return makeString(self.s_ptr->str.trim());
            }

            _NativeFunc(upper){
                return makeString(self.s_ptr->str.upper());
            }

            _NativeFunc(lower){
                return makeString(self.s_ptr->str.lower());
            }

            _NativeFunc(u_upper){
                return makeString(self.s_ptr->str.uUpper());
            }

            _NativeFunc(u_lower){
                return makeString(self.s_ptr->str.uLower());
            }

            _NativeFunc(clone){
                return makeString(self.s_ptr->str);
            }

            _NativeFunc(to_string){
                return self;
            }
        }

        namespace ListClass {
            _BindMethod(List, bracing, 1);
            _BindMethod(List, plus, 1);
            _BindMethod(List, bitor_op, 1);
            _BindMethod(List, bitand_op, 1);
            _BindMethod(List, equal_op, 1);

            _BindMethod(List, reserve, 1);
            _BindMethod(List, resize, 1);
            _BindMethod(List, pop, 0);
            _BindMethod(List, push, 1);
            _BindMethod(List, clone, 2);
            _BindMethod(List, tuple, 2);
            _BindMethod(List, append, 1);
            _BindMethod(List, insert, 2);
            _BindMethod(List, insert_list, 2);
            _BindMethod(List, copy_list, 2);
            _BindMethod(List, reverse, 2);
            _BindMethod(List, sort, 2);
            _BindMethod(List, unique, 2);
            _BindMethod(List, to_string, 0);

            _NativeMethod(List::bracing, 1){
                if(vec.empty() || args[0].type != ObjectType::INTEGER)
                    return Object();
                integer_t i = args[0].i;
                integer_t sz = vec.size();

                if(!runtime::findIndex(i, i, sz))
                    return Object();

                Object ref;
                ref.type = ObjectType::STRONG_REFERENCE;
                ref.o_ptr = &vec[i];
                return ref;
            }

            _NativeMethod(List::plus, 1){
                if(!runtime::of_type(args[0], cList))
                    return Object();
                Object newVec = makeList(intp.rt->gc, false, vec);
                ObjectVec_t& vec2 = reinterpret_cast<List*>(args[0].i_ptr)->vec;
                ObjectVec_t& out = reinterpret_cast<List*>(newVec.i_ptr)->vec;
                out.insert(out.end(), vec2.cbegin(), vec2.cend());
                return newVec;
            }

            _NativeMethod(List::bitor_op, 1){
                if(!runtime::of_type(args[0], cList))
                    return Object();
                Object newVec = makeList(intp.rt->gc, false, vec);
                ObjectVec_t& vec2 = reinterpret_cast<List*>(args[0].i_ptr)->vec;
                ObjectVec_t& out = reinterpret_cast<List*>(newVec.i_ptr)->vec;
                for(const Object& obj : vec2){
                    if(std::find_if(out.begin(), out.end(), runtime::Equal(intp, obj)) == out.end()){
                        out.push_back(obj);
                    }
                }
                return newVec;
            }

            _NativeMethod(List::bitand_op, 1){
                if(!runtime::of_type(args[0], cList))
                    return Object();
                Object newVec = makeList(intp.rt->gc, false, {});
                ObjectVec_t& vec2 = reinterpret_cast<List*>(args[0].i_ptr)->vec;
                ObjectVec_t& out = reinterpret_cast<List*>(newVec.i_ptr)->vec;
                for(const Object& obj : vec2){
                    if(std::find_if(vec.begin(), vec.end(), runtime::Equal(intp, obj)) != vec.end()){
                        out.push_back(obj);
                    }
                }
                return newVec;
            }

            _NativeMethod(List::equal_op, 1){
                if(!runtime::of_type(args[0], cList))
                    return makeFalse();
                ObjectVec_t& vec2 = reinterpret_cast<List*>(args[0].i_ptr)->vec;
                if(vec.size() != vec2.size())
                    return makeFalse();
                return std::equal(vec.begin(), vec.end(), vec2.begin(), runtime::BinaryEqual(intp))
                    ? makeTrue() : makeFalse();
            }

            _NativeMethod(List::reserve, 1){
                if(args[0].type != ObjectType::INTEGER)
                    return Object();
                vec.reserve(args[0].i);
                return Object();
            }

            _NativeMethod(List::resize, 1){
                if(args[0].type != ObjectType::INTEGER)
                    return Object();
                vec.resize(args[0].i);
                return Object();
            }

            _NativeMethod(List::pop, 0){
                if(!vec.empty()){
                    Object back = std::move(vec.back());
                    vec.pop_back();
                    return back;
                }
                return Object();
            }

            _NativeMethod(List::push, 1){
                vec.push_back(args[0]);
                return Object();
            }

            _NativeMethod(List::clone, 2){
                integer_t start = 0;
                integer_t size = vec.size();
                integer_t end = size;

                if(args[0].type == ObjectType::INTEGER){
                    start = args[0].i;
                    if(!runtime::findIndex(start, start, size))
                        return Object();
                } else if(args[0].type != ObjectType::NONE)
                    return Object();

                if(args[1].type == ObjectType::INTEGER){
                    end = args[1].i;
                    if(end < 0){
                        end += size;
                        if(end < 0 || end < start)
                            return Object();
                    } else if(end > size)
                        return Object();
                } else if(args[0].type != ObjectType::NONE)
                    return Object();

                return makeList(intp.rt->gc, false,
                    ObjectVec_t(vec.cbegin() + start, vec.cbegin() + end));
            }

            _NativeMethod(List::tuple, 2){
                return Object();
            }

            _NativeMethod(List::append, 1){
                return Object();
            }

            _NativeMethod(List::insert, 2){
                return Object();
            }

            _NativeMethod(List::insert_list, 2){
                return Object();
            }

            _NativeMethod(List::copy_list, 2){
                return Object();
            }

            _NativeMethod(List::reverse, 2){
                return Object();
            }

            _NativeMethod(List::sort, 2){
                return Object();
            }

            _NativeMethod(List::unique, 2){
                return Object();
            }

            _NativeMethod(List::to_string, 0){
                Object str = makeString("[");
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
            }
        }
    }

    Object makeList(runtime::GarbageCollector& gc, bool temp, ObjectVec_t vec) noexcept{
        return makeFastInstance<lib::ListClass::List>(gc, lib::cList, temp, std::move(vec));
    }
}
