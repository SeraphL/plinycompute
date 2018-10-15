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

#ifndef STO_FREE_ANON_PAGE_REQ_H
#define STO_FREE_ANON_PAGE_REQ_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "../../objectModel/headers/PDBString.h"
#include "../../../../../../../../usr/include/c++/7/cstddef"

// PRELOAD %StoReturnPageRequest%

namespace pdb {

// request to get an anonymous page
class StoReturnPageRequest : public Object {

public:

  StoReturnPageRequest(const std::string &setName, const std::string &databaseName, const size_t &pageNumber)
      : databaseName(databaseName), setName(setName), pageNumber(pageNumber) {}

  StoReturnPageRequest() = default;

  ~StoReturnPageRequest() = default;

  ENABLE_DEEP_COPY;

  /**
   * The database name
   */
  pdb::String databaseName;

  /**
   * The set name
   */
  pdb::String setName;

  /**
   * The page number
   */
  size_t pageNumber = 0;

};
}

#endif
