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
 *      File lib/system.cpp
 *
*/

#include <cstdio>
#include "lib/stdlib.h"
#include "runtime/casts.h"

namespace sm {
    namespace lib {
        smNativeFunc(system){
            if(args.empty() || args[0].type != ObjectType::STRING)
                return Object();
            std::string cmd(args[0].s_ptr->str.begin(), args[0].s_ptr->str.end());
            return makeInteger(std::system(cmd.c_str()));
        }

        smLibDecl(system){
            smInitBox

            smFunc(run, system);
            smIdFunc(runtime::roundId, system);

            smFunc(check, smLambda {
                return makeBool(std::system(nullptr));
            })

            smFunc(exit, smLambda {
                if(args.empty())
                    std::exit(0);
                else if(args[0].type == ObjectType::INTEGER)
                    std::exit(args[0].i);
                else
                    std::exit(runtime::implicitToBool(args[0]));
                return Object();
            })

            smReturnBox
        }
    }
}
