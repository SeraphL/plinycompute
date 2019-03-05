//
// Created by dimitrije on 3/4/19.
//

#include <ExJob.h>
#include <HeapRequestHandler.h>
#include <ExRunJob.h>
#include "ExecutionServerFrontend.h"
#include "SimpleRequestResult.h"

void pdb::ExecutionServerFrontend::registerHandlers(pdb::PDBServer &forMe) {

  forMe.registerHandler(
      ExJob_TYPEID,
      make_shared<pdb::HeapRequestHandler<pdb::ExJob>>(
          [&](Handle<pdb::ExJob> request, PDBCommunicatorPtr sendUsingMe) {

            std::string error;

            /// 1. Do the setup

            // make an allocation block
            const pdb::UseTemporaryAllocationBlock tempBlock{1024};

            // create an allocation block to hold the response
            pdb::Handle<pdb::SimpleRequestResult> response = pdb::makeObject<pdb::SimpleRequestResult>(true, "");

            // sends result to requester
            sendUsingMe->sendObject(response, error);

            /// 2. Do the run

            // make an allocation block
            {
              bool success;

              // want this to be destroyed
              Handle<pdb::ExRunJob> result = sendUsingMe->getNextObject<pdb::ExRunJob> (success, error);
              if (!success) {

                // we are done here does not work
                return make_pair(false, error);
              }
            }

            // sends result to requester
            sendUsingMe->sendObject(response, error);

            // just finish
            return make_pair(true, error);
      }));

}

void pdb::ExecutionServerFrontend::init() {}