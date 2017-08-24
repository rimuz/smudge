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
 *      File utils/String.h
 *
*/

#ifndef _SM__UTILS__STRING_H
#define _SM__UTILS__STRING_H

#include "utils/unicode/utf8.h"
#include "typedefs.h"

#include <vector>
#include <ostream>
#include <utility>
#include <cstring>
#include <iterator>
#include <string>

namespace sm{
    // Tp -> char type (default = char)
    template <class Tp = StringCharType_t, class A = std::allocator<Tp>>
    class string;

    using String = string<>;
}

namespace std {
    template <class Tp, class A>
    struct hash <sm::string<Tp, A>> {
    public:
        size_t operator()(const sm::string<Tp, A>&) const;
    };
}

template <class Tp, class A>
std::ostream& operator<<(const std::ostream&, const sm::string<Tp, A>&);

namespace sm {
    template <class Tp, class A>
    class string : public std::vector<Tp, A>{
    public:
        using unichar = unicode_t;
        using base = std::vector<Tp, A>;

        using value_type = typename base::value_type;
        using size_type = typename base::size_type;
        using difference_type = typename base::difference_type;
        using reference = typename base::reference;
        using pointer = typename base::pointer;
        using const_reference = typename base::const_reference;
        using iterator = typename base::iterator;
        using const_iterator = typename base::const_iterator;
        using reverse_iterator = typename base::reverse_iterator;
        using const_reverse_iterator = typename base::const_reverse_iterator;

        string();
        string(const value_type*);
        string(unicode_t);
        template <class InputIterator>
        string(InputIterator, InputIterator);
        string(const string<Tp, A>&) = default;
        string(string<Tp, A>&&) = default;

        string<Tp, A>& operator=(const value_type*);
        string<Tp, A>& operator=(const string<Tp, A>&) = default;
        string<Tp, A>& operator=(string<Tp, A>&&) = default;

        string<Tp, A> operator+(const value_type*) const;
        string<Tp, A> operator+(const string<Tp, A>&) const;

        string<Tp, A>& operator+=(const value_type*);
        string<Tp, A>& operator+=(const string<Tp, A>&);

        bool operator==(const string<Tp, A>&) const;
        bool operator==(const value_type*) const;
        bool operator!=(const string<Tp, A>&) const;
        bool operator!=(const value_type*) const;

        bool operator<(const string<Tp, A>&) const;
        bool operator>(const string<Tp, A>&) const;
        bool operator<=(const string<Tp, A>&) const;
        bool operator>=(const string<Tp, A>&) const;

        // reference operator [] (const_iterator) const;

        int compare(const string<Tp, A>&) const;
        int compare(const value_type*) const;
        int compareIgnoreCase(const string<Tp, A>&) const;
        int compareIgnoreCase(const value_type*) const;

        int uCompareIgnoreCase(const string<Tp, A>&) const;
        int uCompareIgnoreCase(const value_type*) const;

        bool equals(const string<Tp, A>&) const;
        bool equals(const value_type*) const;
        bool equalsIgnoreCase(const string<Tp, A>&) const;
        bool equalsIgnoreCase(const value_type*) const;
        bool uEqualsIgnoreCase(const string<Tp, A>&) const;
        bool uEqualsIgnoreCase(const value_type*) const;

        string<Tp, A> trim() const;
        size_type uSize() const;
        size_t hash() const;

        value_type* ptr();
        const value_type* ptr() const;

        void append(const value_type*);
        void append(const string<Tp, A>&);
        void append(unicode_t);

        string<Tp, A> uSubstr(size_type, size_type);
        string<Tp, A> uSubstr(size_type);
        string<Tp, A> substr(size_type, size_type) const;
        string<Tp, A> substr(size_type) const;

        string<Tp, A> replace(const string<Tp, A>&, const string<Tp, A>& = string<Tp, A>()) const;
        string<Tp, A> replaceFirst(const string<Tp, A>&, const string<Tp, A>& = string<Tp, A>()) const;
        string<Tp, A> replaceChar(value_type, value_type) const;

        std::vector<string<Tp, A>> split(const string<Tp, A>&, bool skipEmpty = true) const;
        std::vector<string<Tp, A>> split(const value_type*, bool skipEmpty = true) const;

        bool contains(const string<Tp, A>&) const;

        bool startsWith(const string<Tp, A>&) const;
        bool startsWith(const value_type*) const;

        bool endsWith(const string<Tp, A>&) const;
        bool endsWith(const value_type*) const;

        value_type charAt(size_type) const; // size_type = number of the byte
        unicode_t uCharAt(size_type) const; // size_type = number of the character

        void setChar(size_type, value_type);
        void uSetChar(size_type, unicode_t);
        void uSetChar(iterator, unicode_t);

        string<Tp, A> lower() const;
        string<Tp, A> upper() const;

        string<Tp, A> uLower() const;
        string<Tp, A> uUpper() const;

        iterator find(const string<Tp, A>&);
        const_iterator find(const string<Tp, A>&) const;
        iterator findLast(const string<Tp, A>&);
        const_iterator findLast(const string<Tp, A>&) const;

        bool uCheck() const;

        static Tp upper(Tp);
        static Tp lower(Tp);

        static int compare(Tp, Tp);
        static int compareIgnoreCase(Tp, Tp);
        static int uCompareIgnoreCase(unicode_t, unicode_t);
        static constexpr bool isSpace(Tp);

        static bool uNext(const_iterator& it, const_iterator end, unicode_t&);
        static bool uNext(iterator& it, const_iterator end, unicode_t&);
        static bool uNextCPtr(const value_type*&, unicode_t&);
        static bool uGet(const_iterator it, const_iterator end, unicode_t&);
        static void uAppend(std::string&, unicode_t);

        static iterator find(iterator first, iterator last, const string<Tp, A>&);
        static const_iterator find(const_iterator first, const_iterator last, const string<Tp, A>&);

        static size_t uSize(const_iterator, const_iterator);
        static iterator nthChar(iterator, iterator, size_t);

        ~string() = default;
    };
}

namespace std{
    template <class Tp, class A>
    size_t hash<sm::string<Tp, A>>::operator ()(const sm::string<Tp, A>& str) const{
        return str.hash();
    }
}

template <class Tp, class A>
std::ostream& operator<<(std::ostream& os, const sm::string<Tp, A>& str){
    os.write(str.ptr(), str.size());
    return os;
}

namespace sm{
    template <class Tp, class A>
    string<Tp, A>::string(){}

    template <class Tp, class A>
    string<Tp, A>::string(const value_type* str){
        if(str){
            size_t strSize = std::strlen(str);
            this->assign(str, str + strSize);
        }
    }

    template <class Tp, class A>
    template <class InputIterator>
    string<Tp, A>::string(InputIterator beg, InputIterator end) : base(beg, end) {}

    template <class Tp, class A>
    string<Tp, A>::string(unicode_t ch) {
        append(ch);
    }

    template <class Tp, class A>
    string<Tp, A>& string<Tp, A>::operator=(const value_type* str){
        size_t strSize = std::strlen(str);
        this->assign(str, str + strSize);
        return *this;
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::operator+(const value_type* rhs) const {
        string<Tp, A> str;
        size_t rhsSize = std::strlen(rhs);
        size_type size = this->size();
        str.reserve(size + rhsSize);
        str.insert(str.end(), this->begin(), this->end());
        str.insert(str.end(), rhs, rhs + rhsSize);
        return str;
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::operator+(const string<Tp, A>& rhs) const{
        string<Tp, A> str;
        size_type rhsSize = rhs.size();
        size_type size = this->size();
        str.reserve(size + rhsSize);
        str.insert(str.end(), this->begin(), this->end());
        str.insert(str.end(), rhs, rhs + rhsSize);
        return str;
    }

    template <class Tp, class A>
    string<Tp, A>& string<Tp, A>::operator+=(const value_type* rhs){
        this->insert(this->end(), rhs, rhs + std::strlen(rhs));
        return *this;
    }

    template <class Tp, class A>
    string<Tp, A>& string<Tp, A>::operator+=(const string<Tp, A>& rhs){
        this->insert(this->end(), rhs.begin(), rhs.end());
        return *this;
    }

    template <class Tp, class A>
    bool string<Tp, A>::operator==(const string<Tp, A>& str) const{
        return !compare(str);
    }

    template <class Tp, class A>
    bool string<Tp, A>::operator==(const value_type* str) const{
        return !compare(str);
    }

    template <class Tp, class A>
    bool string<Tp, A>::operator!=(const string<Tp, A>& str) const{
        return compare(str);
    }

    template <class Tp, class A>
    bool string<Tp, A>::operator!=(const value_type* str) const{
        return compare(str);
    }

    template <class Tp, class A>
    bool string<Tp, A>::operator<(const string<Tp, A>& str) const{
        return compare(str) < 0;
    }

    template <class Tp, class A>
    bool string<Tp, A>::operator>(const string<Tp, A>& str) const{
        return compare(str) > 0;
    }

    template <class Tp, class A>
    bool string<Tp, A>::operator<=(const string<Tp, A>& str) const{
        return compare(str) <= 0;
    }

    template <class Tp, class A>
    bool string<Tp, A>::operator>=(const string<Tp, A>& str) const{
        return compare(str) >= 0;
    }

    template <class Tp, class A>
    int string<Tp, A>::compare(const string<Tp, A>& str) const{
        int cmp;
        bool greater;
        size_type strSize = str.size();
        size_type size = this->size();
        size_type minSize = (greater = size > strSize) ? strSize : size;

        for(size_type i = 0;; ++i){
            if(i == minSize){
                return greater ? 1 : ((strSize == size) ? 0 : -1);
            } else if((cmp = compare(this->operator[](i), str[i]))){
                return cmp;
            }
        }
        return 0;
    }

    template <class Tp, class A>
    int string<Tp, A>::compare(const value_type* str) const {
        int cmp;
        size_type size = this->size();
        Tp ch;

        for(size_type i = 0;; ++i){
            if(i == size){
                return str[i] ? -1 : 0;
            } else if(!(ch = str[i])){
                return 1;
            } else if((cmp = compare(this->operator[](i), ch))){
                return cmp;
            }
        }
        return 0;
    }

    template <class Tp, class A>
    int string<Tp, A>::compareIgnoreCase(const string<Tp, A>& str) const {
        int cmp;
        bool greater;
        size_type strSize = str.size();
        size_type size = this->size();
        size_type minSize = (greater = size > strSize) ? strSize : size;

        for(size_type i = 0;; ++i){
            if(i == minSize){
                return greater ? 1 : ((strSize == size) ? 0 : -1);
            } else if((cmp = compareIgnoreCase(this->operator[](i), str[i]))){
                return cmp;
            }
        }
        return 0;
    }

    template <class Tp, class A>
    int string<Tp, A>::compareIgnoreCase(const value_type* str) const{
        int cmp;
        size_type size = this->size();
        Tp ch;

        for(size_type i = 0;; ++i){
            if(i == size){
                return str[i] ? -1 : 0;
            } else if(!(ch = str[i])){
                return 1;
            } else if((cmp = compareIgnoreCase(this->operator[](i), ch))){
                return cmp;
            }
        }
        return 0;
    }

    template <class Tp, class A>
    int string<Tp, A>::uCompareIgnoreCase(const value_type* str) const{
        int cmp;
        unicode_t ch1;
        unicode_t ch2;
        const_iterator beg = this->begin();
        const_iterator end = this->end();

        while(1){
            if(beg == end){
                return *str ? -1 : 0;
            } else if(!(*str)){
                return 1;
            } else {
                string<Tp, A>::uNext(beg, end, ch1);
                string<Tp, A>::uNextCPtr(str, ch2);
                if((cmp = uCompareIgnoreCase(ch1, ch2))){
                    return cmp;
                }
            }
        }
        return 0;
    }

    template <class Tp, class A>
    int string<Tp, A>::uCompareIgnoreCase(const string<Tp, A>& str) const {
        int cmp;
        unicode_t ch1;
        unicode_t ch2;
        const_iterator beg = this->begin();
        const_iterator end = this->end();
        const_iterator strBeg = str.begin();
        const_iterator strEnd = str.end();

        while(1){
            if(beg == end){
                return strBeg == strEnd ? 0 : -1;
            } else if(strBeg == strEnd){
                return 1;
            } else {
                string<Tp, A>::uNext(beg, end, ch1);
                string<Tp, A>::uNext(strBeg, strEnd, ch2);
                if((cmp = uCompareIgnoreCase(ch1, ch2))){
                    return cmp;
                }
            }
        }
        return 0;
    }

    template <class Tp, class A>
    bool string<Tp, A>::equals(const string<Tp, A>& str) const{
        return !compare(str);
    }

    template <class Tp, class A>
    bool string<Tp, A>::equals(const value_type* str) const{
        return !compare(str);
    }

    template <class Tp, class A>
    bool string<Tp, A>::equalsIgnoreCase(const string<Tp, A>& str) const{
        return !compareIgnoreCase(str);
    }

    template <class Tp, class A>
    bool string<Tp, A>::equalsIgnoreCase(const value_type* str) const{
        return !compareIgnoreCase(str);
    }

    template <class Tp, class A>
    bool string<Tp, A>::uEqualsIgnoreCase(const string<Tp, A>& str) const{
        return !uCompareIgnoreCase(str);
    }

    template <class Tp, class A>
    bool string<Tp, A>::uEqualsIgnoreCase(const value_type* str) const{
        return !uCompareIgnoreCase(str);
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::trim() const {
        if(this->empty())
            return string<Tp, A>();
        const_iterator start = this->begin(), end = this->end()-1;
        while (start != this->end() && isSpace(*start))
            ++start;
        while (end != start && isSpace(*end))
            --end;
        return string<Tp, A>(start, ++end);
    }

    template <class Tp, class A>
    typename string<Tp, A>::size_type string<Tp, A>::uSize() const {
        size_type size = 0;
        const_iterator it = this->begin();
        const_iterator end = this->end();
        unicode_t ch;
        while(uNext(it, end, ch)) ++size;
        return size;
    }

    template <class Tp, class A>
    size_t string<Tp, A>::hash() const {
        // djb2 hash algorithm
        using UTp = typename std::make_unsigned<Tp>::type;
        size_t hash = 5381;
        for(const_iterator it = this->begin(); it != this->end(); ++it)
            hash += (hash << 5) + static_cast<UTp>(*it);
        return hash;
    }

    template <class Tp, class A>
    typename string<Tp, A>::value_type* string<Tp, A>::ptr(){
        return this->data();
    }

    template <class Tp, class A>
    const typename string<Tp, A>::value_type* string<Tp, A>::ptr() const{
        return this->data();
    }

    template <class Tp, class A>
    void string<Tp, A>::append(const value_type* str){
        this->insert(this->end(), str, str + std::strlen(str));
    }

    template <class Tp, class A>
    void string<Tp, A>::append(const string<Tp, A>& str){
        this->insert(this->end(), str.begin(), str.end());
    }

    template <class Tp, class A>
    void string<Tp, A>::append(unicode_t ch){
        unsigned size = sm::uGetSize(ch);
        this->reserve(this->size() + size);
        for(; size; --size){
            this->push_back((ch >> 8*(size-1)) & 0xFF);
        }
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::uSubstr(size_type begin, size_type end){
        size_type idx = 0;
        const_iterator last = this->end();
        const_iterator beginIt = this->begin(), endIt;
        unicode_t ch;

        while(idx != begin){
            if(!uNext(beginIt, last, ch))
                return string<Tp, A>();
            ++idx;
        }

        endIt = beginIt;
        while(idx != end){
            if(!uNext(endIt, last, ch))
                return string<Tp, A>(beginIt, last);
            ++idx;
        }

        return string<Tp, A>(beginIt, endIt);
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::uSubstr(size_type begin){
        size_type idx = 0;
        const_iterator last = this->end();
        const_iterator beginIt = this->begin();
        unicode_t ch;

        while(idx != begin){
            if(!uNext(beginIt, last, ch))
                return string<Tp, A>();
            ++idx;
        }

        return string<Tp, A>(beginIt, last);
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::substr(size_type begin, size_type end) const{
        return string<Tp, A>(this->begin() + begin, this->begin() + end);
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::substr(size_type begin) const{
        return string<Tp, A>(this->begin() + begin, this->end());
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::replace(const string<Tp, A>& str, const string<Tp, A>& rep) const{
        string<Tp, A> ret;
        size_type strSize(str.size());
        const_iterator first (this->begin());
        const_iterator last (this->end());
        const_iterator lastInsertion (this->begin());

        while((first = string<Tp, A>::find(first, last, str)) != last){
            ret.insert(ret.end(), lastInsertion, first);
            ret.insert(ret.end(), rep.begin(), rep.end());
            lastInsertion = (first += strSize);
        }
        ret.insert(ret.end(), lastInsertion, last);
        return ret;
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::replaceFirst(const string<Tp, A>& str, const string<Tp, A>& rep) const{
        string<Tp, A> ret;
        const_iterator found;
        const_iterator begin = this->begin();
        const_iterator end = this->end();

        if((found = string<Tp, A>::find(begin, end, str)) != end){
            ret.assign(begin, found);
            ret.insert(ret.end(), rep.begin(), rep.end());
            ret.insert(ret.end(), ++found, end);
        } else {
            ret = *this;
        }
        return ret;
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::replaceChar(value_type ch, value_type rep) const {
        string <Tp, A> str;
        str.reserve(this->size());
        for(const_iterator it = this->begin(); it != this->end(); ++it){
            str.push_back(*it == ch ? rep : *it);
        }
        return str;
    }

    template <class Tp, class A>
    std::vector<string<Tp, A>> string<Tp, A>::split(const string<Tp, A>& str, bool skipEmpty) const{
        std::vector<string<Tp, A>> vec;
        const_iterator last (this->begin());
        const_iterator it = last;
        for(; it != this->end(); ++it){
            for(const_iterator it2 = str.begin(); it2 != str.end(); ++it2){
                if(*it == *it2){
                    if(!skipEmpty || last != it)
                        vec.emplace_back(last, it);
                    last = it+1;
                    goto Continue;
                }
            }
            Continue: ;
        }
        if(!skipEmpty || last != it)
            vec.emplace_back(last, it);
        return vec;
    }

    template <class Tp, class A>
    std::vector<string<Tp, A>> string<Tp, A>::split(const value_type* str, bool skipEmpty) const{
        std::vector<string<Tp, A>> vec;
        const_iterator last (this->begin());
        const_iterator it = last;
        for(; it != this->end(); ++it){
            for(const value_type* it2 = str; *it2; ++it2){
                if(*it == *it2){
                    if(!skipEmpty || last != it)
                        vec.emplace_back(last, it);
                    last = it+1;
                    goto Continue;
                }
            }
            Continue: ;
        }
        if(!skipEmpty || last != it)
            vec.emplace_back(last, it);
        return vec;
    }

    template <class Tp, class A>
    bool string<Tp, A>::contains(const string<Tp, A>& str) const{
        return string<Tp, A>::find(this->begin(), this->end(), str) != this->end();
    }

    template <class Tp, class A>
    bool string<Tp, A>::startsWith(const string<Tp, A>& str) const{
        size_type i = 0, strSize = str.size(), size = this->size();
        while(this->operator[](i) == str[i]){
            if(++i == strSize)
                return true;
            else if(i == size)
                return false;
        }
        return false;
    }

    template <class Tp, class A>
    bool string<Tp, A>::startsWith(const value_type* str) const{
        size_type i = 0, size = this->size();
        while(this->operator[](i) == *(str++)){
            if(*str)
                return true;
            else if(i == size)
                return false;
        }
        return false;
    }

    template <class Tp, class A>
    bool string<Tp, A>::endsWith(const string<Tp, A>& str) const{
        size_type size = this->size(), strSize = str.size();
        size_type i = size, i2 = strSize;
        do {
            if(!i2)
                return true;
            else if(!i)
                return false;
        } while(this->operator[](--i) == str[--i2]);
        return false;
    }

    template <class Tp, class A>
    bool string<Tp, A>::endsWith(const value_type* str) const{
        size_type size = this->size(), strSize = std::strlen(str);
        size_type i = size, i2 = strSize;
        do {
            if(!i2)
                return true;
            else if(!i)
                return false;
        } while(this->operator[](--i) == str[--i2]);
        return false;
    }

    template <class Tp, class A>
    typename string<Tp, A>::value_type string<Tp, A>::charAt(size_type idx) const {
        return operator[](idx);
    }

    template <class Tp, class A>
    unicode_t string<Tp, A>::uCharAt(size_type idx) const {
        size_t i = 0;
        const_iterator beg = this->begin();
        const_iterator end = this->end();
        unicode_t ch;
        while(uNext(beg, end, ch)){
            if(idx == i++){
                return ch;
            }
        }
        return 0;
    }

    template <class Tp, class A>
    void string<Tp, A>::setChar(size_type idx, value_type ch){
        this->operator[](idx) = ch;
    }

    template <class Tp, class A>
    void string<Tp, A>::uSetChar(size_type idx, unicode_t ch){
        size_t i = 0;
        iterator beg = this->begin();
        iterator end = this->end();
        unicode_t _ch;
        while(uNext(beg, end, _ch)){
            if(beg == end){
                return;
            } else if(idx == ++i){
                unsigned oldSz = sm::uGetSkip(*beg) +1;
                unsigned newSz = sm::uGetSize(ch);
                signed diff = oldSz - newSz;
                if(diff > 0){
                    beg = this->erase(beg, beg + diff);
                } else if(diff < 0){
                    beg = this->insert(beg, diff * -1, 0);
                }

                for(; newSz; --newSz, ++beg){
                    *beg = (ch >> (8*(newSz-1))) & 0xFF;
                }
                return;
            }
        }
        // out of index!
    }

    template <class Tp, class A>
    void string<Tp, A>::uSetChar(iterator it, unicode_t ch){
        unsigned oldSz = sm::uGetSkip(*it) +1;
        unsigned newSz = sm::uGetSize(ch);
        signed diff = oldSz - newSz;

        if(diff > 0){
            it = this->erase(it, it + diff);
        } else if(diff < 0){
            it = this->insert(it, diff * -1, 0);
        }

        for(; newSz; --newSz, ++it){
            *it = (ch >> 8*(newSz-1)) & 0xFF;
        }
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::upper() const{
        string<Tp, A> str;
        str.reserve(this->size());
        for(const_iterator it = this->begin(); it != this->end(); ++it){
            str.push_back(upper(*it));
        }
        return str;
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::lower() const{
        string<Tp, A> str;
        str.reserve(this->size());
        for(const_iterator it = this->begin(); it != this->end(); ++it){
            str.push_back(lower(*it));
        }
        return str;
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::uUpper() const{
        string<Tp, A> str;
        const_iterator beg = this->begin();
        const_iterator end = this->end();
        unicode_t ch;

        str.reserve(this->size());
        while(uNext(beg, end, ch)){
            str.append(sm::uUpper(ch));
            if(beg == end)
                return str;
        }
        return str;
    }

    template <class Tp, class A>
    string<Tp, A> string<Tp, A>::uLower() const{
        string<Tp, A> str;
        const_iterator beg = this->begin();
        const_iterator end = this->end();
        unicode_t ch;

        str.reserve(this->size());
        while(uNext(beg, end, ch)){
            str.append(sm::uLower(ch));
            if(beg == end)
                return str;
        }
        return str;
    }

    template <class Tp, class A>
    typename string<Tp, A>::const_iterator string<Tp, A>::find(const string<Tp, A>& str) const{
        size_t i = 0, strSize = str.size();
        const_iterator it, found;
        for(it = this->begin(); it != this->end(); ++it){
            if(i == strSize){
                return found;
            } else if(*it == str[i]){
                if(!i++)
                    found = it;
            } else {
                i = 0;
            }
        }
        return it;
    }

    template <class Tp, class A>
    typename string<Tp, A>::iterator string<Tp, A>::find(const string<Tp, A>& str){
        size_t i = 0, strSize = str.size();
        iterator it, found;
        for(it = this->begin(); it != this->end(); ++it){
            if(i == strSize){
                return found;
            } else if(*it == str[i]){
                if(!i++)
                    found = it;
            } else {
                i = 0;
            }
        }
        return it;
    }

    template <class Tp, class A>
    typename string<Tp, A>::iterator string<Tp, A>::findLast(const string<Tp, A>& str){
        size_t strSize = str.size(), i = strSize;
        reverse_iterator it;
        for(it = this->rbegin(); it != this->rend(); ++it){
            if(!i){
                return it.base();
            } else if(*it == str[i-1]){
                i--;
            } else {
                i = strSize;
            }
        }
        return this->end();
    }

    template <class Tp, class A>
    typename string<Tp, A>::const_iterator string<Tp, A>::findLast(const string<Tp, A>& str) const{
        size_t strSize = str.size(), i = strSize;
        const_reverse_iterator it;
        for(it = this->rbegin(); it != this->rend(); ++it){
            if(!i){
                return it.base();
            } else if(*it == str[i-1]){
                i--;
            } else {
                i = strSize;
            }
        }
        return this->end();
    }

    template <class Tp, class A>
    bool string<Tp, A>::uCheck() const{
        const_iterator beg = this->begin();
        const_iterator end = this->end();
        unicode_t ch;

        while(uNext(beg, end, ch)){
            if(beg == end){
                return true;
            }
        }
        return false;
    }

    template <class Tp, class A>
    Tp string<Tp, A>::upper(Tp ch){
        return (ch >= ascii_firstLow && ch <= ascii_lastLow) ? (ch - ascii_caseDiff) : ch;
    }

    template <class Tp, class A>
    Tp string<Tp, A>::lower(Tp ch){
        return (ch >= ascii_firstUp && ch <= ascii_lastUp) ? (ch + ascii_caseDiff) : ch;
    }

    template <class Tp, class A>
    int string<Tp, A>::compare(Tp ch1, Tp ch2){
        using uTp = typename std::make_unsigned<Tp>::type;
        return static_cast<uTp>(ch1) - static_cast<uTp>(ch2);
    }

    template <class Tp, class A>
    int string<Tp, A>::compareIgnoreCase(Tp ch1, Tp ch2){
        using uTp = typename std::make_unsigned<Tp>::type;
        return static_cast<uTp>(lower(ch1)) - static_cast<uTp>(lower(ch2));
    }

    template <class Tp, class A>
    int string<Tp, A>::uCompareIgnoreCase(unicode_t ch1, unicode_t ch2){
        return sm::uLower(ch1) - sm::uLower(ch2);
    }

    template <class Tp, class A>
    constexpr bool string<Tp, A>::isSpace(Tp ch){
        return ch <= ' ';
    }

    template <class Tp, class A>
    bool string<Tp, A>::uNext(const_iterator& it, const_iterator end, unicode_t& ch){
        if(it == end)
            return false;

        unsigned skip = uGetSkip(*it & 0xFF);
        ch = 0;
        do {
            if(it == end)
                return false;
            ch |= (*it++ & 0xFF) << (8 * skip);
        } while(skip--);
        return true;
    }

    template <class Tp, class A>
    bool string<Tp, A>::uNext(iterator& it, const_iterator end, unicode_t& ch){
        if(it == end)
            return false;

        unsigned skip = uGetSkip(*it & 0xFF);
        ch = 0;
        do {
            if(it == end)
                return false;
            ch |= (*it++ & 0xFF) << (8 * skip);
        } while(skip--);
        return true;
    }

    template <class Tp, class A>
    bool string<Tp, A>::uNextCPtr(const value_type*& ptr, unicode_t& ch){
        unsigned skip = uGetSkip(*ptr);
        ch = 0;
        do {
            if(!(*ptr))
                return false;
            ch |= (*ptr++ & 0xFF) << (8 * skip);
        } while(skip--);
        return true;
    }

    template <class Tp, class A>
    bool string<Tp, A>::uGet(const_iterator it, const_iterator end, unicode_t& ch){
        if(it == end)
            return false;

        unsigned skip = uGetSkip(*it & 0xFF);
        ch = 0;
        do {
            if(it == end)
                return false;
            ch |= (*it++ & 0xFF) << (8 * skip);
        } while(skip--);
        return true;
    }

    template <class Tp, class A>
    void string<Tp, A>::uAppend(std::string& str, unicode_t ch){
        unsigned size = sm::uGetSize(ch);
        str.reserve(str.size() + size);
        for(; size; --size){
            str.push_back((ch >> 8*(size-1)) & 0xFF);
        }
    }

    template <class Tp, class A>
    typename string<Tp, A>::iterator string<Tp, A>::find(iterator first, iterator last, const string<Tp, A>& str){
        size_t i = 0, size = str.size();
        while(first != last){
            if(*first == str[i]){
                if(++i == size){
                    return first - (i-1);
                }
            }
            ++first;
        }
        return last;
    }

    template <class Tp, class A>
    typename string<Tp, A>::const_iterator string<Tp, A>::find(const_iterator first, const_iterator last, const string<Tp, A>& str){
        size_t i = 0, size = str.size();
        while(first != last){
            if(*first == str[i]){
                if(++i == size){
                    return first - (i-1);
                }
            }
            ++first;
        }
        return last;
    }

    template <class Tp, class A>
    size_t string<Tp, A>::uSize(const_iterator first, const_iterator last){
        size_type size = 0;
        unicode_t ch;
        while(uNext(first, last, ch)) ++size;
        return size;
    }

    template <class Tp, class A>
    typename string<Tp, A>::iterator string<Tp, A>::nthChar(iterator first, iterator last, size_t idx){
        size_t i = 0;
        unicode_t ch;
        do {
            if(idx == i++){
                return first;
            }
        } while(uNext(first, last, ch));
        return first;
    }
}

#endif
