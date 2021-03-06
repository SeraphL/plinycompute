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

#ifndef SIMPLE_REQ_RES_H
#define SIMPLE_REQ_RES_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include <utility>

// PRELOAD %SimpleRequestResult%

namespace pdb {

// encapsulates a request to obtain a shared library from the catalog
class SimpleRequestResult : public Object {

public:

    SimpleRequestResult() = default;

    ~SimpleRequestResult() = default;

    // generally res should be true on success
    SimpleRequestResult(bool res, const std::string &errMsg) : res(res), errMsg(errMsg) {}

    ENABLE_DEEP_COPY

    std::pair<bool, std::string> getRes() {
        return std::make_pair(res, errMsg);
    }

    bool res = false;

    String errMsg;
};
}

#endif
