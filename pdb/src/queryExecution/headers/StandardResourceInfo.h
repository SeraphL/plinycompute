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

#ifndef STANDARD_RESOURCE_INFO_H
#define STANDARD_RESOURCE_INFO_H

#include <memory>
namespace pdb {

class StandardResourceInfo;
typedef std::shared_ptr<StandardResourceInfo> StandardResourceInfoPtr;


/**
 * This class abstracts each node into a resource
 */
class StandardResourceInfo {

public:
    StandardResourceInfo() = default;

    ~StandardResourceInfo() = default;

    StandardResourceInfo(int numCores, int memSize, std::string address, int port, std::string nodeId)
        : numCores(numCores), memSize(memSize), address(std::move(address)), port(port), nodeId(std::move(nodeId)) {}

    // To get number of CPUs in this resource
    // <=0 for unknown
    int getNumCores() {
        return this->numCores;
    }
    // <=0 for unknown
    void setNumCores(int numCores) {
        this->numCores = numCores;
    }

    // To get size of memory in this resource
    // <= 0 for unknown
    int getMemSize() {
        return this->memSize;
    }
    // <= 0 for unknown
    void setMemSize(int memSize) {
        this->memSize = memSize;
    }

    std::string& getAddress() {
        return address;
    }

    void setAddress(std::string& address) {
        this->address = address;
    }

    std::string getNodeId() {
        return this->nodeId;
    }

    void setNodeId(std::string nodeId) {
        this->nodeId = std::move(nodeId);
    }

    int getPort() {
        return port;
    }

    void setPort(int port) {
        this->port = port;
    }

private:
    // number of CPU cores
    int numCores = -1;

    // size of memory in MB
    int memSize = -1;

    // hostname or IP address of the PDB server
    std::string address;

    // port of the PDB server
    int port = -1;

    // NodeID of the PDB server
    std::string nodeId;
};
}

#endif
