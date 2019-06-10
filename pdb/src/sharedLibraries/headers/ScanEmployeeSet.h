//
// Created by dimitrije on 2/20/19.
//

#ifndef PDB_SCANEMPLOYEESET_H
#define PDB_SCANEMPLOYEESET_H

#include <Employee.h>
#include <SetScanner.h>
#include <LambdaCreationFunctions.h>
#include <VectorTupleSetIterator.h>

class ScanEmployeeSet : public pdb::SetScanner<pdb::Employee> {

 public:

  ENABLE_DEEP_COPY

  ScanEmployeeSet() = default;

  ScanEmployeeSet(const std::string &db, const std::string &set) : SetScanner(db, set) {}

};

#endif //PDB_SCANEMPLOYEESET_H
