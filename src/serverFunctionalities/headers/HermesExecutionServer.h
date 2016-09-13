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
/**
 * Author: Jia
 * Sept 12, 2016
 */


#ifndef HERMES_EXECUTION_SERVER_H
#define HERMES_EXECUTION_SERVER_H

#include "ServerFunctionality.h"
#include "SharedMem.h"
#include "Configuration.h"
#include "PageScanner.h"
#include "DataTypes.h"
#include <string>

namespace pdb {

//this server functionality is supposed to run in backend
class HermesExecutionServer : public ServerFunctionality {

public:

	// creates an execution server... the param is the number of threads to use
	// to do computation
        HermesExecutionServer (SharedMemPtr shm, ConfigurationPtr conf, NodeID nodeId) {
            this->shm = shm;
            this->conf = conf;
            this->nodeId = nodeId;
            this->curScanner = nullptr;
        }

        //set the configuration instance;
	void setConf(ConfigurationPtr conf) { this->conf = conf; }

	//return the configuration instance;
	ConfigurationPtr getConf() { return this->conf; }

	//set the shared memory instance;
	void setSharedMem(SharedMemPtr shm) { this->shm = shm; }

	//return the shared memory instance;
	SharedMemPtr getSharedMem() { return this->shm; }

	//set the nodeId for this backend server;
	void setNodeId(NodeID nodeId) { this->nodeId = nodeId; }

	//return the nodeId of this backend server;
	NodeID getNodeId() { return this->nodeId; }


	// from the ServerFunctionality interface... registers the HermesExecutionServer's        // handlers
	void registerHandlers (PDBServer &forMe) override;

	//register the PageScanner for current job; 
        //now only one job is allowed to run in an execution server at a time
	bool setCurPageScanner(PageScannerPtr curPageScanner) {
		if(curPageScanner == nullptr) {
			this->curScanner = nullptr;
			return true;
		}

		if(this->curScanner == nullptr) {
			this->curScanner = curPageScanner;
			cout<<"scanner set for current job\n";
			return true;
		} else {
			//a job is already running
			cout<<"PDBBackEnd: a job is already running...\n";
			return false;
		}
	}

	//return the PageScanner of current job;
	PageScannerPtr getCurPageScanner() { return this->curScanner; }



	// destructor
	~HermesExecutionServer () {}

private:

        ConfigurationPtr conf;
        SharedMemPtr shm;
        NodeID nodeId;
        PageScannerPtr curScanner;
		
};

}

#endif
