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
#include "lib/stdlib.h"
#include "runtime/casts.h"

namespace sm{
    namespace lib{
        Class* cString = nullptr;

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
            _NativeFunc(mul);
            _NativeFunc(get);
            _NativeFunc(u_get);
            _NativeFunc(contains);
            _NativeFunc(bytes);
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
            _NativeFunc(reverse);
        }

        _LibDecl(lang){
            Class* box = new Class;
            box->boxName = nBox;

            cString = setClass(rt, box, "String", {
                _OpTuple(StringClass, parse::TT_PLUS, plus),
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
                _MethodTuple(StringClass, contains),
                _MethodTuple(StringClass, bytes),
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
                _MethodTuple(StringClass, reverse),
            });

            return box;
        }

        namespace StringClass {
            _NativeFunc(idx){
                if(args.empty() || args[0].type != ObjectType::INTEGER)
                    return makeInteger(0);
                integer_t i = args[0].i;
                String::iterator begin = self.s_ptr->str.begin(),
                    end = self.s_ptr->str.end();
                if(i > 0){
                    String::iterator ch_it = String::nthChar(begin, end, i);
                    return makeInteger(std::distance(begin, ch_it));
                } else if(i < 0){
                    integer_t n = self.s_ptr->str.uSize() + i;
                    if(n <= 0)
                        return makeInteger(0);
                    String::iterator ch_it = String::nthChar(begin, end, n);
                    return makeInteger(std::distance(begin, ch_it));
                }
                return makeInteger(0);
            }

            _NativeFunc(len){
                return makeInteger(self.s_ptr->str.size());
            }

            _NativeFunc(count){
                return makeInteger(self.s_ptr->str.uSize());
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
                Object str1;
                runtime::implicitToString(*intp.rt, args[0], str1);
                str0.s_ptr->str.insert(str0.s_ptr->str.end(), str1.s_ptr->str.begin(),
                    str1.s_ptr->str.end());
                return str0;
            }

            _NativeFunc(mul){
                integer_t i;
                if(args.empty() || args[0].type != ObjectType::INTEGER || (i = args[0].i) < 0)
                    return Object();

                Object str0 = makeString(self.s_ptr->str);
                size_t sz = str0.s_ptr->str.size();
                str0.s_ptr->str.resize(sz * i);
                if(i == 0)
                    return str0;

                String::iterator beg = str0.s_ptr->str.begin();
                String::iterator end = beg + sz;
                String::iterator curr = end;

                for(integer_t j = 1; j != i; ++j){
                    curr = std::copy(beg, end, curr);
                }
                return str0;
            }

            _NativeFunc(get){
                if(self.s_ptr->str.empty())
                    return Object();
                if(args.empty()){
                    return makeInteger(self.s_ptr->str[0]);
                } else if(args[0].type == ObjectType::INTEGER){
                    integer_t idx = args[1].i;
                    if(idx >= 0){
                        size_t i = static_cast<size_t>(idx);
                        if(i >= self.s_ptr->str.size())
                            return Object();
                        return makeInteger(self.s_ptr->str[i]);
                    }

                    idx += static_cast<integer_t>(self.s_ptr->str.size());
                    if(idx <= 0)
                        return Object();
                    return makeInteger(self.s_ptr->str[static_cast<size_t>(idx)]);
                }
                return Object();
            }

            _NativeFunc(u_get){
                unicode_t ch = 0;
                String::const_iterator beg, end = self.s_ptr->str.cend();

                if(self.s_ptr->str.empty())
                    return Object();
                if(args.empty()){
                    beg = self.s_ptr->str.begin();
                    String::uGet(beg, end, ch);
                    return makeInteger(ch);
                } else if(args[0].type == ObjectType::INTEGER){
                    integer_t idx = args[1].i;
                    if(idx >= 0){
                        size_t i = static_cast<size_t>(idx);
                        if(i >= self.s_ptr->str.size())
                            return Object();
                        beg = self.s_ptr->str.begin() + i;
                        String::uGet(beg, end, ch);
                        return makeInteger(ch);
                    }

                    idx += static_cast<integer_t>(self.s_ptr->str.size());
                    if(idx <= 0)
                        return Object();
                    beg = self.s_ptr->str.begin() + idx;
                    String::uGet(beg, end, ch);
                    return makeInteger(ch);
                }
                return Object();
            }

            _NativeFunc(contains){
                if(args.empty() || args[0].type != ObjectType::STRING)
                    return Object();
                return makeInteger(self.s_ptr->str.contains(args[0].s_ptr->str));
            }

            _NativeFunc(bytes){
                // TODO!!!!
                return Object();
            }

            _NativeFunc(join){
                // TODO!!!!
                return Object();
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
                // TODO!!
                return Object();
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

            _NativeFunc(reverse){
                // TODO!!!
                return makeString(self.s_ptr->str.uLower());
            }
        }
    }
}
