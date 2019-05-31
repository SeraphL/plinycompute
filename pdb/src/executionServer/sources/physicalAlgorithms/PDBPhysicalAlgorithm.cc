#include <PDBPhysicalAlgorithm.h>
#include <PDBStorageManagerBackend.h>
#include <AtomicComputationClasses.h>
#include <AtomicComputation.h>

namespace pdb {

PDBPhysicalAlgorithm::PDBPhysicalAlgorithm(const AtomicComputationPtr &fistAtomicComputation,
                                           const AtomicComputationPtr &finalAtomicComputation,
                                           const pdb::Handle<PDBSourcePageSetSpec> &source,
                                           const pdb::Handle<PDBSinkPageSetSpec> &sink,
                                           const pdb::Handle<pdb::Vector<pdb::Handle<PDBSourcePageSetSpec>>> &secondarySources,
                                           const pdb::Handle<pdb::Vector<PDBSetObject>> &setsToMaterialize,
                                           bool swapLHSandRHS) : firstTupleSet(fistAtomicComputation->getOutputName()),
                                                                 finalTupleSet(finalAtomicComputation->getOutputName()),
                                                                 source(source),
                                                                 sink(sink),
                                                                 secondarySources(secondarySources),
                                                                 setsToMaterialize(setsToMaterialize),
                                                                 swapLHSandRHS(swapLHSandRHS) {

  // check if we are scanning a set if we are fill in sourceSet field
  if(fistAtomicComputation->getAtomicComputationTypeID() == ScanSetAtomicTypeID) {

    // cast to a scan set
    auto scanSet = (ScanSet*) fistAtomicComputation.get();

    // get the set info
    sourceSet = pdb::makeObject<PDBSetObject>(scanSet->getDBName(), scanSet->getSetName());
  }
}

PDBAbstractPageSetPtr PDBPhysicalAlgorithm::getSourcePageSet(std::shared_ptr<pdb::PDBStorageManagerBackend> &storage) {

  // get the source computation
  auto srcNode = logicalPlan->getComputations().getProducingAtomicComputation(firstTupleSet);

  // if this is a scan set get the page set from a real set
  PDBAbstractPageSetPtr sourcePageSet;
  if (srcNode->getAtomicComputationTypeID() == ScanSetAtomicTypeID) {

    // cast it to a scan
    auto scanNode = std::dynamic_pointer_cast<ScanSet>(srcNode);

    // get the page set
    sourcePageSet = storage->createPageSetFromPDBSet(scanNode->getDBName(),
                                                     scanNode->getSetName(),
                                                     std::make_pair(source->pageSetIdentifier.first,
                                                                    source->pageSetIdentifier.second));
    sourcePageSet->resetPageSet();
  } else {

    // we are reading from an existing page set get it
    sourcePageSet = storage->getPageSet(source->pageSetIdentifier);
    sourcePageSet->resetPageSet();
  }

  // return the page set
  return sourcePageSet;
}

std::shared_ptr<JoinArguments> PDBPhysicalAlgorithm::getJoinArguments(std::shared_ptr<pdb::PDBStorageManagerBackend> &storage) {

  // go through each of the additional sources and add them to the join arguments
  auto joinArguments = std::make_shared<JoinArguments>();
  for(int i = 0; i < this->secondarySources->size(); ++i) {

    // grab the source identifier and with it the page set of the additional source
    auto &sourceIdentifier = *(*this->secondarySources)[i];
    auto additionalSource = storage->getPageSet(std::make_pair(sourceIdentifier.pageSetIdentifier.first, sourceIdentifier.pageSetIdentifier.second));

    // do we have have a page set for that
    if(additionalSource == nullptr) {
      return nullptr;
    }

    // insert the join argument
    joinArguments->hashTables[sourceIdentifier.pageSetIdentifier.second] = std::make_shared<JoinArg>(additionalSource);
  }

  return joinArguments;
}

}