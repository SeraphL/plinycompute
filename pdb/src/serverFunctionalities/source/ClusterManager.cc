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

#include <ClusterManager.h>
#include <SimpleRequestResult.h>
#include <SimpleRequest.h>

#include "CatalogClient.h"
#include "ClusterManager.h"
#include "CluSyncRequest.h"
#include "SimpleRequestHandler.h"

namespace pdb {

// just map the stuff to get the system info to something reasonable
using MemoryInfo = struct sysinfo;
const auto &getMemoryInfo = sysinfo;
const auto &getCPUCores = std::thread::hardware_concurrency;

ClusterManager::ClusterManager(std::string address, int32_t port, bool isManager)
    : address(std::move(address)), port(port), isManager(isManager) {

  // grab the system info
  MemoryInfo memoryInfo{};
  getMemoryInfo(&memoryInfo);

  // grab the number of cores
  numCores = getCPUCores();

  // grab the total memory on this machine
  totalMemory = memoryInfo.totalram / 1024;

  // create the logger
  logger = make_shared<PDBLogger>("clusterManager.log");
}

void ClusterManager::registerHandlers(PDBServer &forMe) {
  forMe.registerHandler(
      CluSyncRequest_TYPEID,
      make_shared<SimpleRequestHandler<CluSyncRequest>>(
          [&](Handle<CluSyncRequest> request, PDBCommunicatorPtr sendUsingMe) {

            // lock the catalog server
            std::lock_guard<std::mutex> guard(serverMutex);

            if (!isManager) {

              // create the response
              std::string error = "A worker node can not sync the cluster only a manager can!";
              Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(false, error);

              // sends result to requester
              bool success = sendUsingMe->sendObject(response, error);

              // return the result
              return make_pair(success, error);
            }

            // generate the node identifier
            std::string nodeIdentifier = (std::string) request->nodeIP + ":" + std::to_string(request->nodePort);

            // sync the catalog server on this node with the one on the one that is requesting it.
            std::string error;
            bool success = getFunctionality<CatalogClient>().syncWithNode(std::make_shared<PDBCatalogNode>(nodeIdentifier,
                                                                                                           request->nodeIP,
                                                                                                           request->nodePort,
                                                                                                           request->nodeType,
                                                                                                           request->nodeNumCores,
                                                                                                           request->nodeMemory), error);

            // create the response
            Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(success, error);

            // sends result to requester
            success = sendUsingMe->sendObject(response, error) && success;

            // return the result
            return make_pair(success, error);
          }));
}

bool ClusterManager::syncCluster(const std::string &managerAddress, int managerPort, std::string &error) {

  // figure out the type of the node
  std::string type = isManager ? "manager" : "worker";

  return simpleRequest<CluSyncRequest, SimpleRequestResult, bool>(
      logger, managerPort, managerAddress, false, 1024,
      [&](Handle<SimpleRequestResult> result) {
        if (result != nullptr) {
          if (!result->getRes().first) {
            error = "ClusterManager : Could not sink with the manager";
            logger->error(error);
            return false;
          }
          return true;
        }

        error = "ClusterManager : Could not sink with the manager";
        return false;
      },
      address, port, type, totalMemory, numCores);
}

}