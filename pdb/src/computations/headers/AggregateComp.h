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

#ifndef AGG_COMP
#define AGG_COMP

#include "AggregateCompBase.h"
#include "MapTupleSetIterator.h"
#include "DepartmentTotal.h"
#include "PreaggregationSink.h"
#include "AggregationCombinerSink.h"

namespace pdb {

// this aggregates items of type InputClass.  To aggregate an item, the result of getKeyProjection () is
// used to extract a key from on input, and the result of getValueProjection () is used to extract a
// value from an input.  Then, all values having the same key are aggregated using the += operation over values.
// Note that keys must have operation == as well has hash () defined.  Also, note that values must have the
// + operation defined.
// 
// Once aggregation is completed, the key-value pairs are converted into OutputClass objects.  An object
// of type OutputClass must have two methods defined: KeyClass &getKey (), as well as ValueClass &getValue ().
// To convert a key-value pair into an OutputClass object, the result of getKey () is set to the desired key,
// and the result of getValue () is set to the desired value.
//
template<class OutputClass, class InputClass, class KeyClass, class ValueClass>
class AggregateComp : public AggregateCompBase {

  // gets the operation tht extracts a key from an input object
  virtual Lambda<KeyClass> getKeyProjection(Handle<InputClass> aggMe) = 0;

  // gets the operation that extracts a value from an input object
  virtual Lambda<ValueClass> getValueProjection(Handle<InputClass> aggMe) = 0;

  // extract the key projection and value projection
  void extractLambdas(std::map<std::string, LambdaObjectPtr> &returnVal) override {
    int suffix = 0;
    Handle<InputClass> checkMe = nullptr;
    Lambda<KeyClass> keyLambda = getKeyProjection(checkMe);
    Lambda<ValueClass> valueLambda = getValueProjection(checkMe);
    keyLambda.toMap(returnVal, suffix);
    valueLambda.toMap(returnVal, suffix);
  }

  // this is an aggregation comp
  std::string getComputationType() override {
    return std::string("AggregationComp");
  }

  int getNumInputs() override {
    return 1;
  }

  // gets the name of the i^th input type...
  std::string getIthInputType(int i) override {
    if (i == 0) {
      return getTypeName<InputClass>();
    } else {
      return "";
    }
  }

  // gets the output type of this query as a string
  std::string getOutputType() override {
    return getTypeName<OutputClass>();
  }

  // below function implements the interface for parsing computation into a TCAP string
  std::string toTCAPString(std::vector<InputTupleSetSpecifier> inputTupleSets,
                           int computationLabel,
                           std::string &outputTupleSetName,
                           std::vector<std::string> &outputColumnNames,
                           std::string &addedOutputColumnName) override {

    if (inputTupleSets.empty()) {
      return "";
    }

    InputTupleSetSpecifier inputTupleSet = inputTupleSets[0];
    return toTCAPString(inputTupleSet.getTupleSetName(),
                        inputTupleSet.getColumnNamesToKeep(),
                        inputTupleSet.getColumnNamesToApply(),
                        computationLabel,
                        outputTupleSetName,
                        outputColumnNames,
                        addedOutputColumnName);
  }

  // to return Aggregate tcap string
  std::string toTCAPString(std::string inputTupleSetName,
                           std::vector<std::string> inputColumnNames,
                           std::vector<std::string> inputColumnsToApply,
                           int computationLabel,
                           std::string &outputTupleSetName,
                           std::vector<std::string> &outputColumnNames,
                           std::string &addedOutputColumnName) {
    return "";
  }

  ComputeSinkPtr getComputeSink(TupleSpec &consumeMe, TupleSpec &projection, uint64_t numberOfPartitions) override {
    return std::make_shared<pdb::PreaggregationSink<KeyClass, ValueClass>>(consumeMe, projection, numberOfPartitions);
  }

  ComputeSourcePtr getComputeSource(const PDBAbstractPageSetPtr &pageSet, size_t chunkSize, uint64_t workerID) override {
    return std::make_shared<pdb::MapTupleSetIterator<KeyClass, ValueClass, OutputClass>> (pageSet, workerID, chunkSize);
  }

  ComputeSinkPtr getAggregationHashMapCombiner(uint64_t workerID) override {
    return std::make_shared<pdb::AggregationCombinerSink<KeyClass, ValueClass>>(workerID);
  }

};

}

#endif
