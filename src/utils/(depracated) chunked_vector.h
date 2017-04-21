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
 *      File utils/chunked_vector.h
 *
*/

#ifndef _SM__UTILS__CHUNKED_VECTOR_H_
#define _SM__UTILS__CHUNKED_VECTOR_H_

/*
* WARNING: chunked_vector is deprecated: after a simple benchmark,
* std::vector was the winner, and because chunked_vector was created
* to beat it in speed, there are no reasons to choose chunked_vector anymore.
*/

#warning "Depracated: use std::vector instead."

#include <vector>
#include <algorithm>
#include <utility>
#include <memory>
#include <cstdlib>

#include "typedefs.h"
#include "require_cpp11.h"

namespace sm{
    // chunk size will be 2^ChunkSize bytes.
    template <class Tp, size_t ChunkSize, class ChunkVec_t = std::vector<void*>>
    class chunked_vector {
    public:
        using value_type = Tp;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using reference = Tp&;
        using const_reference = const Tp&;
        using pointer = Tp*;

        static_assert(ChunkSize != 0, "ChunkSize cannot be equal to zero.");

        static constexpr size_t ChunkElements = 1 << ChunkSize;
        static constexpr size_t ElementsSize = sizeof(Tp);
        static constexpr size_t ChunkBytes = ChunkElements * ElementsSize;
        static constexpr size_t ElementsLocMask = fillLSB<size_t>(0, ChunkSize-1);

        chunked_vector();
        chunked_vector(size_t);
        chunked_vector(const chunked_vector<Tp, ChunkSize, ChunkVec_t>&);
        chunked_vector(chunked_vector<Tp, ChunkSize, ChunkVec_t>&&) = default;

        chunked_vector& operator=(const chunked_vector<Tp, ChunkSize, ChunkVec_t>&);
        chunked_vector& operator=(chunked_vector<Tp, ChunkSize, ChunkVec_t>&&) = default;

        reference operator[](size_type);
        reference at(size_type);

        const_reference operator[](size_type) const;
        const_reference at(size_type) const;

        reference front();
        reference back();
        const_reference front() const;
        const_reference back() const;

        /*
         * Sorry, no push_font/insert functions 'cause they are
         * too expensive for this type of vector.
        */
        void push_back(const Tp&);
        void push_back(Tp&&);
        void pop_back();

        template <class... Args>
        void emplace_back(Args&&...);

        typename ChunkVec_t::size_type size() const;
        bool empty() const;

        void swap(chunked_vector<Tp, ChunkSize, ChunkVec_t>&);
        void clear();

        ~chunked_vector();
    private:
        ChunkVec_t chunks;
        size_type back_size;
        size_type back_chunk;
    };

    template <class Tp, size_t N>
    struct free_chunk {
        inline void operator()(void* ptr) const{
            Tp* beg = static_cast<Tp*>(ptr);
            Tp* end = beg + N;
            while(beg != end){
                (beg++)->~Tp();
            }
            std::free(ptr);
        }
    };

    template <class Tp, size_t Cs, class Cv>
    chunked_vector<Tp, Cs, Cv>::chunked_vector() : back_size(0), back_chunk(0) {}

    template <class Tp, size_t Cs, class Cv>
    chunked_vector<Tp, Cs, Cv>::chunked_vector(size_t sz) {
        if(sz){
            sz--;
            size_t n_chunks = (sz >> Cs) +1;
            chunks.resize(n_chunks);
            for(size_t i = 0; i != n_chunks; ++i){
                chunks[i] = std::malloc(ChunkBytes);
                size_t j = 0;
                pointer ptr = static_cast<pointer>(chunks[i]);
                do {
                    new (ptr++) Tp();
                } while(++j != ChunkElements);
            }
            back_size = (sz & ElementsLocMask) +1;
            back_chunk = n_chunks-1;
        } else {
            back_size = 0;
            back_chunk = 0;
        }
    }

    template <class Tp, size_t Cs, class Cv>
    chunked_vector<Tp, Cs, Cv>::chunked_vector(const chunked_vector<Tp, Cs, Cv>& rhs)
            : back_size(rhs.back_size), back_chunk(rhs.back_chunk){
        size_t n_chunks = rhs.chunks.size();
        chunks.resize(n_chunks);
        for(size_t i = 0; i != n_chunks; ++i){
            chunks[i] = std::malloc(ChunkBytes);
            size_t j = 0;
            pointer ptr = static_cast<pointer>(chunks[i]);
            pointer src = static_cast<pointer>(rhs.chunks[i]);
            do {
                new (ptr++) Tp(*src++);
            } while(++j != ChunkElements);
        }
    }

    template <class Tp, size_t Cs, class Cv>
    chunked_vector<Tp, Cs, Cv>& chunked_vector<Tp, Cs, Cv>::operator=(const chunked_vector<Tp, Cs, Cv>& rhs){
        std::for_each(chunks.begin(), chunks.end(), std::free);
        size_t n_chunks = rhs.chunks.size();
        chunks.resize(n_chunks);
        for(size_t i = 0; i != n_chunks; ++i){
            chunks[i] = std::malloc(ChunkBytes);
            size_t j = 0;
            pointer ptr = static_cast<pointer>(chunks[i]);
            pointer src = static_cast<pointer>(rhs.chunks[i]);
            do {
                new (ptr++) Tp(*src++);
            } while(++j != ChunkElements);
        }
        back_size = rhs.back_size;
        back_chunk = rhs.back_chunk;
        return *this;
    }

    template <class Tp, size_t Cs, class Cv>
    typename chunked_vector<Tp, Cs, Cv>::reference chunked_vector<Tp, Cs, Cv>::operator[](size_type sz){
        return static_cast<pointer>(chunks[sz >> Cs])[sz & ElementsLocMask];
    }

    template <class Tp, size_t Cs, class Cv>
    typename chunked_vector<Tp, Cs, Cv>::reference chunked_vector<Tp, Cs, Cv>::at(size_type sz){
        return static_cast<pointer>(chunks.at(sz >> Cs))[sz & ElementsLocMask];
    }

    template <class Tp, size_t Cs, class Cv>
    typename chunked_vector<Tp, Cs, Cv>::const_reference chunked_vector<Tp, Cs, Cv>::operator[](size_type sz) const{
        return static_cast<pointer>(chunks[sz >> Cs])[sz & ElementsLocMask];
    }

    template <class Tp, size_t Cs, class Cv>
    typename chunked_vector<Tp, Cs, Cv>::const_reference chunked_vector<Tp, Cs, Cv>::at(size_type sz) const{
        return static_cast<pointer>(chunks.at(sz >> Cs))[sz & ElementsLocMask];
    }

    template <class Tp, size_t Cs, class Cv>
    typename chunked_vector<Tp, Cs, Cv>::reference chunked_vector<Tp, Cs, Cv>::front(){
        return static_cast<pointer>(chunks.front())[0];
    }

    template <class Tp, size_t Cs, class Cv>
    typename chunked_vector<Tp, Cs, Cv>::reference chunked_vector<Tp, Cs, Cv>::back(){
        return static_cast<pointer>(chunks[back_chunk])[back_size-1];
    }

    template <class Tp, size_t Cs, class Cv>
    typename chunked_vector<Tp, Cs, Cv>::const_reference chunked_vector<Tp, Cs, Cv>::front() const{
        return static_cast<pointer>(chunks.front())[0];
    }

    template <class Tp, size_t Cs, class Cv>
    typename chunked_vector<Tp, Cs, Cv>::const_reference chunked_vector<Tp, Cs, Cv>::back() const{
        return static_cast<pointer>(chunks[back_chunk])[back_size-1];
    }

    template <class Tp, size_t Cs, class Cv>
    void chunked_vector<Tp, Cs, Cv>::push_back(const Tp& obj){
        if(empty()){
            void* newChunk = std::malloc(ChunkBytes);
            new (newChunk) Tp(obj);
            chunks.push_back(newChunk);
            back_size = 1;
        } else if(back_size++ == ChunkElements){
            if(++back_chunk == chunks.size()){
                void* newChunk = std::malloc(ChunkBytes);
                new (newChunk) Tp(obj);
                chunks.push_back(newChunk);
            } else {
                new (chunks[back_chunk]) Tp(obj);
            }
            back_size = 1;
        } else {
            new (static_cast<pointer>(chunks.back()) + back_size-1) Tp(obj);
        }
    }

    template <class Tp, size_t Cs, class Cv>
    void chunked_vector<Tp, Cs, Cv>::push_back(Tp&& obj){
        if(empty()){
            void* newChunk = std::malloc(ChunkBytes);
            new (newChunk) Tp(std::move(obj));
            chunks.push_back(newChunk);
            back_size = 1;
        } else if(back_size++ == ChunkElements){
            if(++back_chunk == chunks.size()){
                void* newChunk = std::malloc(ChunkBytes);
                new (newChunk) Tp(std::move(obj));
                chunks.push_back(newChunk);
            } else {
                new (chunks[back_chunk]) Tp(std::move(obj));
            }
            back_size = 1;
        } else {
            new (static_cast<pointer>(chunks.back()) + back_size-1) Tp(std::move(obj));
        }
    }

    template <class Tp, size_t Cs, class Cv>
    template <class... Args>
    void chunked_vector<Tp, Cs, Cv>::emplace_back(Args&&... args){
        if(empty()){
            void* newChunk = std::malloc(ChunkBytes);
            new (newChunk) Tp(std::forward<Args>(args)...);
            chunks.push_back(newChunk);
            back_size = 1;
        } else if(back_size++ == ChunkElements){
            if(++back_chunk == chunks.size()){
                void* newChunk = std::malloc(ChunkBytes);
                new (newChunk) Tp(std::forward<Args>(args)...);
                chunks.push_back(newChunk);
            } else {
                new (chunks[back_chunk]) Tp(std::forward<Args>(args)...);
            }
            back_size = 1;
        } else {
            new (static_cast<pointer>(chunks.back()) + back_size-1) Tp(std::forward<Args>(args)...);
        }
    }

    template <class Tp, size_t Cs, class Cv>
    void chunked_vector<Tp, Cs, Cv>::pop_back(){
        back().~Tp();
        if(!back_size--){
            back_chunk--;
            back_size = ElementsLocMask;
        }
    }

    template <class Tp, size_t Cs, class Cv>
    typename Cv::size_type chunked_vector<Tp, Cs, Cv>::size() const{
        return (back_chunk * ChunkElements) + back_size;
    }

    template <class Tp, size_t Cs, class Cv>
    bool chunked_vector<Tp, Cs, Cv>::empty() const{
        return !back_chunk && !back_size;
    }

    template <class Tp, size_t Cs, class Cv>
    void chunked_vector<Tp, Cs, Cv>::swap(chunked_vector<Tp, Cs, Cv>& rhs){
        size_t _back_chunk = back_chunk, _back_size = back_size;
        chunks.swap(rhs.chunks);

        back_chunk = rhs.back_chunk;
        back_size = rhs.back_size;
        rhs.back_chunk = _back_chunk;
        rhs.back_size = _back_size;
    }

    template <class Tp, size_t Cs, class Cv>
    void chunked_vector<Tp, Cs, Cv>::clear(){
        std::for_each(chunks.begin(), chunks.end(), free_chunk<value_type, ChunkElements>());
        back_chunk = 0;
        back_size = 0;
    }

    template <class Tp, size_t Cs, class Cv>
    chunked_vector<Tp, Cs, Cv>::~chunked_vector(){
        std::for_each(chunks.begin(), chunks.end(), free_chunk<value_type, ChunkElements>());
    }
}

#endif
