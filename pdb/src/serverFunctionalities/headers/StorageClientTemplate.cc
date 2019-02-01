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

#ifndef STORAGE_CLIENT_TEMPLATE_CC
#define STORAGE_CLIENT_TEMPLATE_CC

#include "StorageClient.h"
#include "StorageAddData.h"
#include "StorageAddSet.h"
#include "StorageRemoveUserSet.h"
#include "StorageGetData.h"
#include "StorageGetDataResponse.h"
#include "StorageTestSetScan.h"
#include "StorageTestSetCopy.h"
#include "SimpleRequestResult.h"
#include "SimpleSendDataRequest.h"
#include "DataTypes.h"
#include <cstddef>
#include <fcntl.h>
#include <fstream>
#include <iostream>

namespace pdb {

template <class DataType>
bool StorageClient::storeData(Handle<Vector<Handle<DataType>>> data,
                              std::string databaseName,
                              std::string setName,
                              std::string& errMsg,
                              bool typeCheck) {
    return simpleSendDataRequest<StorageAddData, Handle<DataType>, SimpleRequestResult, bool>(
        myLogger,
        port,
        address,
        false,
        1024,
        [&](Handle<SimpleRequestResult> result) {
            if (result != nullptr)
                if (!result->getRes().first) {
                    myLogger->error("Error sending data: " + result->getRes().second);
                    errMsg = "Error sending data: " + result->getRes().second;
                }
            return true;
        },
        data,
        databaseName,
        setName,
        getTypeName<DataType>(),
        typeCheck);
}

template <class DataType>
bool StorageClient::createSet(std::string databaseName, std::string setName, std::string& errMsg) {
    if (usePangea == false) {
        return myHelper.createSet<DataType>(databaseName, setName, errMsg);
    } else {
        std::string typeName = getTypeName<DataType>();
        std::cout << "typeName for set to create =" << typeName << std::endl;
        return RequestFactory::heapRequest< StorageAddSet, SimpleRequestResult, bool>(
            myLogger,
            port,
            address,
            false,
            1024,
            [&](Handle<SimpleRequestResult> result) {
                if (result != nullptr) {
                    if (!result->getRes().first) {
                        errMsg = "Error creating set: " + result->getRes().second;
                        myLogger->error("Error creating set: " + result->getRes().second);
                        return false;
                    }
                    return true;
                }
                errMsg = "Error getting type name: got nothing back from catalog";
                return false;
            },
            databaseName,
            setName,
            typeName);
    }
}


template <class DataType>
bool StorageClient::removeSet(std::string databaseName, std::string setName, std::string& errMsg) {
    if (this->usePangea == true) {

        std::string typeName = getTypeName<DataType>();
        std::cout << "typeName for set to remove =" << typeName << std::endl;
        return RequestFactory::heapRequest<StorageRemoveUserSet, SimpleRequestResult, bool>(
            myLogger,
            port,
            address,
            false,
            1024,
            [&](Handle<SimpleRequestResult> result) {
                if (result != nullptr) {
                    if (!result->getRes().first) {
                        errMsg = "Error removing set: " + result->getRes().second;
                        myLogger->error("Error removing set: " + result->getRes().second);
                        return false;
                    }
                    return true;
                }
                errMsg = "Error getting type name: got nothing back from catalog";
                return false;

            },
            databaseName,
            setName,
            typeName);

    } else {

        return false;
    }
}

}
#endif
