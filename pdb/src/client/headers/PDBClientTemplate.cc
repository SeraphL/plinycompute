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
#ifndef PDB_CLIENT_TEMPLATE_CC
#define PDB_CLIENT_TEMPLATE_CC

#include "PDBClient.h"

namespace pdb {


  template <class DataType>
  bool PDBClient::createSet(const std::string &databaseName, const std::string &setName) {

    bool result = catalogClient->template createSet<DataType>(databaseName, setName, returnedMsg);

    if (!result) {
        errorMsg = "Not able to create set: " + returnedMsg;
    } else {
        cout << "Created set.\n";
    }

    return result;
  }

  template <class DataType>
  bool PDBClient::sendData(const std::string &database, const std::string &set, Handle<Vector<Handle<DataType>>> dataToSend) {

    bool result = dispatcherClient->sendData<DataType>(database, set, dataToSend, returnedMsg);

    if (result==false) {
        errorMsg = "Not able to send data: " + returnedMsg;
        exit(-1);
    } else {
        cout << "Data sent.\n";
    }
    return result;
  }

  template<class DataType>
  PDBStorageIteratorPtr<DataType> PDBClient::getSetIterator(std::string dbName, std::string setName) {
    return std::make_shared<PDBStorageIterator<DataType>>(address, port, 5, setName, dbName);
  }

}
#endif
