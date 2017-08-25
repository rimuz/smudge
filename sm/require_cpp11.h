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
 *      File require_cpp11.h
 *
*/

#ifndef _SM__REQUIRE_CPP11_H
#define _SM__REQUIRE_CPP11_H

#if !defined(__cplusplus) || __cplusplus < 201103L
#error Smudge requires C++11 to work correctly.
#endif

#endif // _SM__REQUIRE_CPP11_H
