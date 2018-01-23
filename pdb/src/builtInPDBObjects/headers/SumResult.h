/*****************************************************************************
 *                                                                           *
 *  Copyright 2018 Rice University                                           *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *****************************************************************************/
#ifndef SUM_RESULT_H
#define SUM_RESULT_H


#include "Object.h"

// PRELOAD %SumResult%

namespace pdb {

class SumResult : public Object {

public:
    int total;
    int identifier;

    ENABLE_DEEP_COPY

    int getTotal() {
        return total;
    }

    int& getKey() {
        return identifier;
    }

    int& getValue() {
        return total;
    }
};
}


#endif