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

#ifndef PDBCLIENT_CC
#define PDBCLIENT_CC

#include <ShutDown.h>
#include "PDBClient.h"

namespace pdb {

PDBClient::PDBClient(int portIn, std::string addressIn) : port(portIn), address(addressIn) {

  // init the logger
  logger = make_shared<PDBLogger>("clientLog");

  // init the catalog client
  catalogClient = std::make_shared<pdb::PDBCatalogClient>(portIn, addressIn, logger);

  // init the dispatcher client
  dispatcherClient = std::make_shared<pdb::PDBDispatcherClient>(portIn, addressIn, logger);

  // init the distributed client
  distributedStorageClient = std::make_shared<pdb::DistributedStorageManagerClient>(portIn, addressIn, logger);

  // init the query client
  queryClient = std::make_shared<pdb::QueryClient>(portIn, addressIn, logger, true);
}

string PDBClient::getErrorMessage() {
  return errorMsg;
}

/****
 * Methods for invoking DistributedStorageManager-related operations
 */
bool PDBClient::createDatabase(const std::string &databaseName) {

  bool result = catalogClient->createDatabase(databaseName, returnedMsg);

  if (!result) {

    errorMsg = "Not able to create database: " + returnedMsg;

  } else {
    cout << "Created database.\n";
  }
  return result;
}

bool PDBClient::createSet(const std::string &databaseName, const std::string &setName, const std::string &typeName) {

  bool result = distributedStorageClient->createSet(databaseName, setName, typeName, returnedMsg, DEFAULT_PAGE_SIZE);

  if (!result) {
    errorMsg = "Not able to create set: " + returnedMsg;
    exit(-1);
  } else {
    cout << "Created set.\n";
  }
  return result;
}

bool PDBClient::createSet(const std::string &databaseName, const std::string &setName, const std::string &typeName, size_t pageSize) {

  bool result = distributedStorageClient->createSet(databaseName, setName, typeName,
                                                    returnedMsg, pageSize);
  if (!result) {
    errorMsg = "Not able to create set: " + returnedMsg;
    exit(-1);
  } else {
    cout << "Created set.\n";
  }
  return result;
}

bool PDBClient::createTempSet(const std::string &databaseName,
                              const std::string &setName,
                              const std::string &typeName,
                              size_t pageSize) {

  bool result = distributedStorageClient->createTempSet(databaseName, setName, typeName, returnedMsg, pageSize);
  if (!result) {
    errorMsg = "Not able to create temp set: " + returnedMsg;
    exit(-1);
  } else {
    cout << "Created temp set.\n";
  }
  return result;
}

bool PDBClient::removeDatabase(const std::string &databaseName) {

  bool result = distributedStorageClient->removeDatabase(databaseName, returnedMsg);
  if (!result) {
    errorMsg = "Not able to remove database: " + returnedMsg;
  } else {
    cout << "Database has been removed.\n";
  }
  return result;
}

bool PDBClient::removeSet(const std::string &databaseName,
                          const std::string &setName) {

  bool result = distributedStorageClient->removeSet(databaseName, setName, returnedMsg);
  if (!result) {
    errorMsg = "Not able to remove set: " + returnedMsg;
  } else {
    cout << "Set has been removed.\n";
  }
  return result;
}

bool PDBClient::removeTempSet(const std::string &databaseName,
                              const std::string &setName,
                              const std::string &typeName) {

  bool result = distributedStorageClient->removeTempSet(databaseName, setName, typeName,
                                                        returnedMsg);
  if (!result) {
    errorMsg = "Not able to remove Temp set: " + returnedMsg;
  } else {
    cout << "Temp set removed.\n";
  }
  return result;
}

bool PDBClient::exportSet(const std::string &databaseName,
                          const std::string &setName,
                          const std::string &outputFilePath,
                          const std::string &format) {

  bool result = distributedStorageClient->exportSet(databaseName, setName,
                                                    outputFilePath, format, returnedMsg);
  if (!result) {
    errorMsg = "Not able to export set: " + returnedMsg;
    exit(-1);
  } else {
    cout << "Set has been exported.\n";
  }
  return result;
}

// makes a request to shut down a PDB server /// TODO this should be moved
bool PDBClient::shutDownServer(std::string &errMsg) {

  // get the workers
  auto workers = catalogClient->getActiveWorkerNodes();

  // shutdown the workers
  bool success = true;
  for(const auto &w : workers) {
    success = success && RequestFactory::heapRequest<ShutDown, SimpleRequestResult, bool>(logger, w->port, w->address, false, 1024,
     [&](Handle<SimpleRequestResult> result) {

       // do we have a result
       if(result == nullptr) {

         errMsg = "Error getting type name: got nothing back from catalog";
         return false;
       }

       // did we succeed
       if (!result->getRes().first) {

         errMsg = "Error shutting down server: " + result->getRes().second;
         logger->error("Error shutting down server: " + result->getRes().second);

         return false;
       }

       // we succeeded
       return true;
     });
  }

  // shutdown
  return success && RequestFactory::heapRequest<ShutDown, SimpleRequestResult, bool>(logger, port, address, false, 1024,
      [&](Handle<SimpleRequestResult> result) {

        // do we have a result
        if(result == nullptr) {

          errMsg = "Error getting type name: got nothing back from catalog";
          return false;
        }

        // did we succeed
        if (!result->getRes().first) {

          errMsg = "Error shutting down server: " + result->getRes().second;
          logger->error("Error shutting down server: " + result->getRes().second);

          return false;
        }

        // we succeeded
        return true;
      });
}

bool PDBClient::clearSet(const std::string &databaseName,
                         const std::string &setName,
                         const std::string &typeName) {

  bool result = distributedStorageClient->clearSet(databaseName, setName, typeName,
                                                   returnedMsg);
  if (!result) {
    errorMsg = "Not able to clear set: " + returnedMsg;
  } else {
    cout << "Set has been cleared.\n";
  }
  return result;
}

bool PDBClient::flushData() {

  bool result = distributedStorageClient->flushData(returnedMsg);
  if (!result) {
    errorMsg = "Not able to flush data: " + returnedMsg;
    exit(-1);
  } else {
    cout << "Data has been flushed.\n";
  }
  return result;
}

std::function<bool(Handle<SimpleRequestResult>)>
PDBClient::generateResponseHandler(std::string description, std::string &returnedMsg) {

  return [&](Handle<SimpleRequestResult> result) {
    if (result != nullptr) {
      if (!result->getRes().first) {
        errorMsg = description + result->getRes().second;
        logger->error(description + ": " + result->getRes().second);
        return false;
      }
      return true;
    }
    errorMsg = "Received nullptr as response";
    logger->error(description + ": received nullptr as response");
    return false;
  };
}

bool PDBClient::registerType(const std::string &fileContainingSharedLib) {

  bool result = catalogClient->registerType(fileContainingSharedLib, returnedMsg);
  if (!result) {
    errorMsg = "Not able to register type: " + returnedMsg;
    exit(-1);
  } else {
    cout << "Type has been registered.\n";
  }
  return result;
}

void PDBClient::printCatalogMetadata(
    pdb::Handle<pdb::CatPrintCatalogRequest> itemToSearch) {
  cout << catalogClient->printCatalogMetadata(
      itemToSearch,
      returnedMsg);
}

void PDBClient::listAllRegisteredMetadata() {
  cout << catalogClient->listAllRegisteredMetadata(returnedMsg);
}

void PDBClient::listRegisteredDatabases() {
  cout << catalogClient->listRegisteredDatabases(returnedMsg);
}

void PDBClient::listRegisteredSetsForADatabase(const std::string &databaseName) {
  cout << catalogClient->listRegisteredSetsForADatabase(databaseName, returnedMsg);
}

void PDBClient::listNodesInCluster() {
  cout << catalogClient->listNodesInCluster(returnedMsg);
}

void PDBClient::listUserDefinedTypes() {
  cout << catalogClient->listUserDefinedTypes(returnedMsg);
}

/****
 * Methods for invoking Query-related operations
 */
bool PDBClient::deleteSet(std::string databaseName, std::string setName) {

  bool result = queryClient->deleteSet(databaseName, setName);

  if (!result) {
    errorMsg = "Not able to delete set: ";
    exit(-1);
  } else {
    cout << "Set has been deleted.\n";
  }
  return result;
}
}

#endif
