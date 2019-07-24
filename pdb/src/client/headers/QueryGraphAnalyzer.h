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
#ifndef QUERY_GRAPH_ANALYZER_HEADER
#define QUERY_GRAPH_ANALYZER_HEADER

#include "Computation.h"
#include "Handle.h"
#include "InputTupleSetSpecifier.h"
#include "PDBVector.h"
#include <vector>

namespace pdb {


// This class encapsulates the analyzer to user query graph
// This class is also called a TCAP compiler, 
// and it translates a user query graph into a TCAP program.

// A TCAP program has following format:
// $TargetTupleSetName($tupleNamesToKeep, $newTupleName) <=
// $TCAPOperatorName($SourceTupleSetName($tupleNamesToApply),
//                   $SourceTupleSetName($tupleNamesToKeep),
//                   $ComputationName,
//                   $LambdaName)            

// Currently supported TCAP operators include:
// Apply, Aggregate, Partition, Join, ScanSet, WriteSet, HashLeft, HashRight
// HashOne, Flatten, Filter and so on.

// Note that the user query graph should not have loops

class QueryGraphAnalyzer {

public:

  // constructor
  explicit QueryGraphAnalyzer(const vector<Handle < Computation>> &queryGraph);

  // to convert user query to a tcap string
  std::string parseTCAPString();

  // to traverse the sub-tree and put each traversed computation to a vector
  void parseComputations(Vector<Handle<Computation>> &computations,
                         Handle<Computation> sink);

  // to convert user query to a pdb::Vector of computations
  // this method will invoke makeObject, but will not allocate allocation blocks
  // you must ensure current allocation block has sufficient memory before
  // invoking this method
  void parseComputations(Vector<Handle<Computation>> &computations);

  // to clear traversal marks on the subtree rooted at sink
  void clearGraphMarks(Handle<Computation> sink);

  // to clear all traversal marks
  void clearGraphMarks();

private:
  // user query graph
  std::vector<Handle<Computation>> queryGraph;
};
}

#endif
