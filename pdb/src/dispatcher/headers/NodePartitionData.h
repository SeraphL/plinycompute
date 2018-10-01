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

#ifndef OBJECTQUERYMODEL_NODEPARTITIONDATA_H
#define OBJECTQUERYMODEL_NODEPARTITIONDATA_H

#include "DataTypes.h"
#include "NodeDispatcherData.h"

#include <string>

namespace pdb {

class NodePartitionData;
typedef std::shared_ptr<NodePartitionData> NodePartitionDataPtr;

class NodePartitionData {
public:
    NodePartitionData(std::string nodeId,
                      int port,
                      std::string address,
                      std::pair<std::string, std::string> setAndDatabaseName);

    std::string toString() const;
    std::string getNodeId() const;
    int getPort() const;
    std::string getAddress() const;
    std::string getSetName() const;
    std::string getDatabaseName() const;
    size_t getTotalBytesSent() const;

    bool operator==(const NodePartitionData& other) {
        return this->port == other.getPort() && this->address == other.getAddress();
    }

private:
    std::string nodeId;
    int port;
    std::string address;
    std::string setName;
    std::string databaseName;

    size_t totalBytesSent;
};
}


#endif  // OBJECTQUERYMODEL_NODEPARTITIONDATA_H
