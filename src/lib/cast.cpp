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
 *      File lib/cast.cpp
 *
*/

#include <cstdlib>
#include "lib/stdlib.h"
#include "runtime/casts.h"

namespace sm{
    namespace lib{
        smLibDecl(cast){
            smInitBox

            smFunc(int, smLambda {
                if(args.empty())
                    return Object();
                const Object& obj = args[0];
                switch(obj.type){
                    case ObjectType::INTEGER:
                        return obj;
                    case ObjectType::FLOAT:
                        return makeInteger(obj.f);
                    case ObjectType::STRING: {
                        std::string str (obj.s_ptr->str.begin(), obj.s_ptr->str.end());
                        return makeInteger(std::atol(str.c_str()));
                    }
                }
                return Object();
            })

            smFunc(float, smLambda {
                if(args.empty())
                    return Object();
                const Object& obj = args[0];
                switch(obj.type){
                    case ObjectType::INTEGER:
                        return makeFloat(obj.i);
                    case ObjectType::FLOAT:
                        return obj;
                    case ObjectType::STRING: {
                        std::string str (obj.s_ptr->str.begin(), obj.s_ptr->str.end());
                        return makeFloat(std::atof(str.c_str()));
                    }
                }
                return Object();
            })

            smFunc(string, smLambda {
                if(args.empty())
                    return Object();
                return runtime::implicitToString(intp, args[0]);
            })

            smReturnBox
        }
    }
}
