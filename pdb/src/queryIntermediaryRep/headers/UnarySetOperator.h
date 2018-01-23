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
#ifndef PDB_QUERYINTERMEDIARYREP_UNARYSETOPERATOR_H
#define PDB_QUERYINTERMEDIARYREP_UNARYSETOPERATOR_H

#include "Handle.h"
#include "SetExpressionIr.h"

using pdb::Handle;

using pdb_detail::SetExpressionIr;

namespace pdb_detail {
class UnarySetOperator {

public:
    UnarySetOperator(Handle<SetExpressionIr> operand);

    Handle<SetExpressionIr> getOperand();

private:
    Handle<SetExpressionIr> _operand;
};
}

#endif  // PDB_QUERYINTERMEDIARYREP_UNARYSETOPERATOR_H