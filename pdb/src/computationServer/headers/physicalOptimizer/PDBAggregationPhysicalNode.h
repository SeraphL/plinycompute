//
// Created by dimitrije on 2/21/19.
//

#ifndef PDB_PDBAGGREGATIONPIPELINE_H
#define PDB_PDBAGGREGATIONPIPELINE_H

#include "PDBAbstractPhysicalNode.h"

namespace pdb {

class PDBAggregationPhysicalNode : public PDBAbstractPhysicalNode  {

public:
  
  PDBAggregationPhysicalNode(const std::vector<AtomicComputationPtr>& pipeline, size_t computationID, size_t currentNodeIndex) : PDBAbstractPhysicalNode(pipeline, computationID, currentNodeIndex) {};

  ~PDBAggregationPhysicalNode() override = default;

  PDBPipelineType getType() override;

  pdb::PDBPlanningResult generateAlgorithm(const AtomicComputationPtr &startAtomicComputation,
                                           const pdb::Handle<PDBSourcePageSetSpec> &source,
                                           PDBPageSetCosts &sourcesWithIDs,
                                           pdb::Handle<pdb::Vector<pdb::Handle<PDBSourcePageSetSpec>>> &additionalSources,
                                           bool shouldSwapLeftAndRight) override;

  pdb::PDBPlanningResult generatePipelinedAlgorithm(const AtomicComputationPtr &startAtomicComputation,
                                                    const pdb::Handle<PDBSourcePageSetSpec> &source,
                                                    PDBPageSetCosts &sourcesWithIDs,
                                                    pdb::Handle<pdb::Vector<pdb::Handle<PDBSourcePageSetSpec>>> &additionalSources,
                                                    bool shouldSwapLeftAndRight) override;

};

}


#endif //PDB_PDBAGGREGATIONPIPELINE_H
