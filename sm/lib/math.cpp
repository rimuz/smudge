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
 *      File lib/math.cpp
 *
*/

#include <random>
#include "sm/lib/stdlib.h"


namespace sm{
    constexpr float_t PI = 3.141592653589793;
    constexpr float_t DOUBLE_PI = 3.141592653589793*2;
    constexpr float_t E = 2.718281828459045;

    namespace lib{
        namespace random {
            std::random_device random_device;
            std::mt19937 mt(random_device());

            std::uniform_real_distribution<> urd(0, 1);
        }

        smLibDecl(math){
            smInitBox

            smVar(PI, makeFloat(PI));
            smVar(DOUBLE_PI, makeFloat(DOUBLE_PI));
            smVar(E, makeFloat(E));
            smVar(NAN, makeFloat(NAN));
            smVar(INF, makeFloat(INFINITY))

            smFunc(rand, smLambda {
                return makeFloat(random::urd(random::mt));
            })

            smFunc(rand_int, smLambda {
                if(args.size() < 2 || args[0].type != ObjectType::INTEGER
                        || args[1].type != ObjectType::INTEGER)
                    return Object();
                integer_t min = args[0].i, max = args[1].i;
                if(max < min)
                    return Object();
                else if(max == min)
                    return makeInteger(max);
                integer_t distance = max - min;
                return makeInteger(min + static_cast<integer_t>(random::urd(random::mt) * distance));
            })

            #define __SmBind(fnName, realName) \
                smFunc(fnName, smLambda { \
                    if(args.empty()) \
                        return Object(); \
                    else if(args[0].type == ObjectType::INTEGER) \
                        return makeFloat(std::realName(static_cast<float_t>(args[0].i))); \
                    else if(args[0].type == ObjectType::FLOAT) \
                        return makeFloat(std::realName(args[0].f)); \
                    return Object(); \
                })
            #define __SmSnBind(fnName) __SmBind(fnName, fnName)
                __SmSnBind(cos);
                __SmSnBind(sin);
                __SmSnBind(tan);
                __SmSnBind(acos);
                __SmSnBind(asin);
                __SmSnBind(atan);
                __SmSnBind(cosh);
                __SmSnBind(sinh);
                __SmSnBind(tanh);
                __SmSnBind(acosh);
                __SmSnBind(asinh);
                __SmSnBind(atanh);
                __SmSnBind(exp);
                __SmSnBind(exp2);
                __SmSnBind(log);
                __SmSnBind(log2);
                __SmSnBind(log10);
                __SmSnBind(sqrt);
                __SmSnBind(cbrt);
                __SmSnBind(ceil);
                __SmSnBind(floor);
                __SmSnBind(round);
                __SmSnBind(trunc);
            #undef __SmBind
            #undef __SmSnBind

            smFunc(atan2, smLambda {
                if(args.empty())
                    return Object();
                float_t x, y;

                if(args[0].type == ObjectType::INTEGER)
                    x = args[0].i;
                else if(args[0].type == ObjectType::FLOAT)
                    x = args[0].f;
                else return Object();

                if(args[1].type == ObjectType::INTEGER)
                    y = args[1].i;
                else if(args[1].type == ObjectType::FLOAT)
                    y = args[1].f;
                else return Object();

                return makeFloat(std::atan2(x, y));
            })

            smFunc(frexp, smLambda {
                if(args.empty())
                    return Object();

                float_t signif;
                int exp;

                if(args[0].type == ObjectType::INTEGER)
                    signif = std::frexp(static_cast<float_t>(args[0].i), &exp);
                else if(args[0].type == ObjectType::FLOAT)
                    signif = std::frexp(args[0].f, &exp);
                else
                    return Object();

                return makeList(intp, ObjectVec_t {makeFloat(signif), makeInteger(exp)});
            })

            smFunc(pow, smLambda {
                if(args.empty())
                    return Object();
                float_t x, y;

                if(args[0].type == ObjectType::INTEGER)
                    x = args[0].i;
                else if(args[0].type == ObjectType::FLOAT)
                    x = args[0].f;
                else return Object();

                if(args[1].type == ObjectType::INTEGER)
                    y = args[1].i;
                else if(args[1].type == ObjectType::FLOAT)
                    y = args[1].f;
                else return Object();

                return makeFloat(std::pow(x, y));
            })

            smFunc(hypot, smLambda {
                if(args.empty())
                    return Object();
                float_t x, y;

                if(args[0].type == ObjectType::INTEGER)
                    x = args[0].i;
                else if(args[0].type == ObjectType::FLOAT)
                    x = args[0].f;
                else return Object();

                if(args[1].type == ObjectType::INTEGER)
                    y = args[1].i;
                else if(args[1].type == ObjectType::FLOAT)
                    y = args[1].f;
                else return Object();

                return makeFloat(std::hypot(x, y));
            })

            smFunc(round_int, smLambda {
                if(args.empty())
                    return Object();
                else if(args[0].type == ObjectType::INTEGER)
                    return makeInteger(std::lround(static_cast<float_t>(args[0].i)));
                else if(args[0].type == ObjectType::FLOAT)
                    return makeInteger(std::lround(args[0].f));
                return Object();
            })

            smFunc(is_nan, smLambda {
                if(args.empty() || args[0].type != ObjectType::FLOAT)
                    return makeFalse();
                return makeBool(std::isnan(args[0].f));
            })

            smFunc(is_inf, smLambda {
                if(args.empty() || args[0].type != ObjectType::FLOAT)
                    return makeFalse();
                return makeBool(std::isinf(args[0].f));
            })

            smFunc(deg, smLambda {
                if(args[0].type == ObjectType::FLOAT)
                    return makeFloat(args[0].f / DOUBLE_PI * 360.f);
                else if(args[0].type == ObjectType::INTEGER)
                    return makeFloat(args[0].i / DOUBLE_PI * 360.f);
                return Object();
            })

            smFunc(rad, smLambda {
                if(args[0].type == ObjectType::FLOAT)
                    return makeFloat(args[0].f / 360.f * DOUBLE_PI);
                else if(args[0].type == ObjectType::INTEGER)
                    return makeFloat(args[0].i / 360.f * DOUBLE_PI);
                return Object();
            })

            smReturnBox
        }
    }
}
