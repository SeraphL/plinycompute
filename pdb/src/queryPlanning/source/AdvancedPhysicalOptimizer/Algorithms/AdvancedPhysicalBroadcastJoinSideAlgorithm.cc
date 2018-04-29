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

#include <JobStageBuilders/TupleSetJobStageBuilder.h>
#include <JobStageBuilders/BroadcastJoinBuildHTJobStageBuilder.h>
#include <AdvancedPhysicalOptimizer/Algorithms/AdvancedPhysicalBroadcastJoinSideAlgorithm.h>

AdvancedPhysicalBroadcastJoinSideAlgorithm::AdvancedPhysicalBroadcastJoinSideAlgorithm(const AdvancedPhysicalPipelineNodePtr &handle,
                                                                                       const std::string &jobID,
                                                                                       bool isProbing,
                                                                                       const Handle<SetIdentifier> &source,
                                                                                       const vector<AtomicComputationPtr> &pipeComputations,
                                                                                       const Handle<ComputePlan> &computePlan,
                                                                                       const LogicalPlanPtr &logicalPlan,
                                                                                       const ConfigurationPtr &conf) : AdvancedPhysicalAbstractAlgorithm(handle,
                                                                                                                                                         jobID,
                                                                                                                                                         isProbing,
                                                                                                                                                         source,
                                                                                                                                                         pipeComputations,
                                                                                                                                                         computePlan,
                                                                                                                                                         logicalPlan,
                                                                                                                                                         conf) {}

PhysicalOptimizerResultPtr AdvancedPhysicalBroadcastJoinSideAlgorithm::generate(int nextStageID) {

  // create a analyzer result
  PhysicalOptimizerResultPtr result = make_shared<PhysicalOptimizerResult>();

  // get the source atomic computation
  auto sourceAtomicComputation = this->pipeComputations.front();

  // we get the first atomic computation of the join pipeline that comes after this one.
  // This computation should be the apply join computation
  auto joinAtomicComputation = handle->getConsumer(0)->to<AdvancedPhysicalAbstractPipeline>()->getPipelineComputationAt(0);

  // get the final atomic computation
  string finalAtomicComputationName = this->pipeComputations.back()->getOutputName();

  // the computation specifier of this join
  std::string computationSpecifier = joinAtomicComputation->getComputationName();

  // grab the computation associated with this node
  Handle<Computation> curComp = logicalPlan->getNode(computationSpecifier).getComputationHandle();

  // grab the output of the current node
  std::string outputName = joinAtomicComputation->getOutputName();

  // the set identifier of the set where we store the output of the TupleSetJobStage
  sink = makeObject<SetIdentifier>(jobID, outputName + "_broadcastData");
  sink->setPageSize(conf->getBroadcastPageSize());

  // create a tuple set job stage builder
  TupleSetJobStageBuilderPtr tupleStageBuilder = make_shared<TupleSetJobStageBuilder>();

  // copy the computation names
  for (const auto &it : this->pipeComputations) {

    // we don't need the output set name... (that is jsut the way the pipeline building works)
    if (it->getAtomicComputationTypeID() == WriteSetTypeID) {
      continue;
    }

    // add the set name of the atomic computation to the pipeline
    tupleStageBuilder->addTupleSetToBuildPipeline(it->getOutputName());
  }

  // set the parameters
  tupleStageBuilder->setSourceTupleSetName(sourceAtomicComputation->getOutputName());
  tupleStageBuilder->setSourceContext(source);
  tupleStageBuilder->setInputAggHashOut(source->isAggregationResult());
  tupleStageBuilder->setJobId(jobID);
  tupleStageBuilder->setComputePlan(computePlan);
  tupleStageBuilder->setJobStageId(nextStageID);
  tupleStageBuilder->setTargetTupleSetName(finalAtomicComputationName);
  tupleStageBuilder->setTargetComputationName(computationSpecifier);
  tupleStageBuilder->setOutputTypeName("IntermediateData");
  tupleStageBuilder->setSinkContext(sink);
  tupleStageBuilder->setBroadcasting(true);
  tupleStageBuilder->setAllocatorPolicy(curComp->getAllocatorPolicy());

  // We are setting isBroadcasting to true so that we run a pipeline with broadcast sink
  Handle<TupleSetJobStage> joinPrepStage = tupleStageBuilder->build();

  // add the stage to the list of stages to be executed
  result->physicalPlanToOutput.emplace_back(joinPrepStage);

  // add the sink to the intermediate sets
  result->interGlobalSets.push_back(sink);

  // grab the hash set name
  std::string hashSetName = sink->toSourceSetName();

  // initialize the build hash partition set builder stage
  BroadcastJoinBuildHTJobStageBuilderPtr broadcastBuilder = make_shared<BroadcastJoinBuildHTJobStageBuilder>();

  // set the parameters
  broadcastBuilder->setJobId(jobID);
  broadcastBuilder->setJobStageId(nextStageID);
  broadcastBuilder->setSourceTupleSetName(joinPrepStage->getSourceTupleSetSpecifier());
  broadcastBuilder->setTargetTupleSetName(finalAtomicComputationName);
  broadcastBuilder->setTargetComputationName(computationSpecifier);
  broadcastBuilder->setSourceContext(sink);
  broadcastBuilder->setHashSetName(hashSetName);
  broadcastBuilder->setComputePlan(computePlan);

  // We then create a BroadcastJoinBuildHTStage
  Handle<BroadcastJoinBuildHTJobStage> joinBroadcastStage = broadcastBuilder->build();

  // add the stage to the list of stages to be executed
  result->physicalPlanToOutput.emplace_back(joinBroadcastStage);

  // set the remaining parameters of the result
  result->success = true;
  result->newSourceComputation = nullptr;

  return result;
}

AdvancedPhysicalAbstractAlgorithmTypeID AdvancedPhysicalBroadcastJoinSideAlgorithm::getType() {
  return JOIN_BROADCAST_ALGORITHM;
}

PhysicalOptimizerResultPtr AdvancedPhysicalBroadcastJoinSideAlgorithm::generatePipelined(int nextStageID, std::vector<AdvancedPhysicalPipelineNodePtr> &pipeline) {
  return nullptr;
}


