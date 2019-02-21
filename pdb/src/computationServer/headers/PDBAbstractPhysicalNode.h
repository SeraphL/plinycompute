//
// Created by dimitrije on 2/21/19.
//

#ifndef PDB_PDBABSTRACTPIPELINE_H
#define PDB_PDBABSTRACTPIPELINE_H

#include <AtomicComputation.h>
#include <list>


enum PDBPipelineType {

  PDB_STRAIGHT_PIPELINE,
  PDB_AGGREGATION_PIPELINE

};

namespace pdb {

class PDBAbstractPhysicalNode;


typedef std::shared_ptr<PDBAbstractPhysicalNode> PDBAbstractPhysicalNodePtr;
typedef std::shared_ptr<PDBAbstractPhysicalNode> PDBAbstractPhysicalNodeWeakPtr;

class PDBAbstractPhysicalNode {

public:

  // TODO
  PDBAbstractPhysicalNode(const std::vector<AtomicComputationPtr>& pipeline, size_t currentNodeIndex) {};

  virtual ~PDBAbstractPhysicalNode() = default;

  /**
   * Where the pipeline begins
   */
  std::vector<AtomicComputationPtr> pipeline;

  /**
   * A list of consumers of this node
   */
  std::list<PDBAbstractPhysicalNodePtr> consumers;

  /**
   * A list of producers of this node
   */
  std::list<PDBAbstractPhysicalNodeWeakPtr> producers;

  /**
   * A shared pointer to an instance of this node
   */
  PDBAbstractPhysicalNodeWeakPtr handle;

  bool hasScanSet() { return false;};

  /**
 * Returns a shared pointer handle to this node
 * @return the shared pointer handle
 */
  PDBAbstractPhysicalNodePtr getHandle() {

    // if we do not have a handle to this node already
    if(handle == nullptr) {
      handle = std::shared_ptr<PDBAbstractPhysicalNode> (this);
    }

    return handle;
  }

  /**
   * Removes a consumer of this node
   * @param consumer the consumer we want to remove
   */
  virtual void removeConsumer(const PDBAbstractPhysicalNodePtr &consumer) {
    consumers.remove(consumer);
    consumer->producers.remove(getHandle());
  }

  /**
  * Adds a consumer to the node
  * @param consumer the consumer
  */
  virtual void addConsumer(const pdb::PDBAbstractPhysicalNodePtr &consumer) {
    consumers.push_back(consumer);
    consumer->producers.push_back(getHandle());
  }

  /**
   * Returns the cost of running this pipeline
   * @return the cost
   */
  virtual size_t getCost() {
    throw std::runtime_error("");
  }

  /**
   * Returns the type of the pipeline
   * @return the type
   */
  virtual PDBPipelineType getType() = 0;

  virtual std::string getNodeIdentifier() = 0;

  virtual std::vector<AtomicComputationPtr>& getPipeComputations() { return pipeline; }
};

}
#endif //PDB_PDBABSTRACTPIPELINE_H
