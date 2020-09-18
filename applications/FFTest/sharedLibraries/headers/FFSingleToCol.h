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
#pragma once

#include "DoubleVector.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "MultiSelectionComp.h"
#include "FFMatrixBlock.h"
#include "PDBVector.h"
#include <cstdlib>
#include <ctime>

namespace pdb {

// the sub namespace
namespace ff {

// FFSingleToCol will
class FFSingleToCol : public MultiSelectionComp<FFMatrixBlock, FFMatrixBlock> {

public:

  ENABLE_DEEP_COPY

  FFSingleToCol() = default;
  FFSingleToCol(int32_t smallColNum) : smallColNum(smallColNum) {

  }

  // srand has already been invoked in server
  Lambda<bool> getSelection(Handle<FFMatrixBlock> checkMe) override {
    return makeLambda(checkMe, [&](Handle<FFMatrixBlock> &checkMe) { return true; });
  }

  Lambda<Vector<Handle<FFMatrixBlock>>> getProjection(Handle<FFMatrixBlock> checkMe) override {
    return makeLambda(checkMe, [&](Handle<FFMatrixBlock> &checkMe) {

      Vector<Handle<FFMatrixBlock>> out;
      auto data = checkMe->data->data->c_ptr();
      int32_t numRows = checkMe->getNumRows();
      int32_t numCols = checkMe->getNumCols();

	// assume sub-block has size numRows x smallColNum
	int32_t smallColID = numCols / smallColNum;
	int32_t i = 0;

	if (checkMe->data->bias == nullptr) {
		for (int32_t j = 0; j < smallColID; j++) {
		Handle<FFMatrixBlock> smallBlock = makeObject<FFMatrixBlock>(0, j, numRows, smallColNum);
		auto outData = smallBlock->data->data->c_ptr();
		      for (int32_t m = 0; m < numRows; m++) {
			for (int32_t n = 0; n < smallColNum; n++) {
				outData[m * smallColNum + n] = data[smallColNum * j + m * numCols + n];
			}
		      }
		out.push_back(smallBlock);
		}
	}
	else {
		// only add bias to the last row blockss
		auto bias = checkMe->data->bias->c_ptr();
		for (int32_t j = 0; j < smallColID; j++) {
		Handle<FFMatrixBlock> smallBlock = makeObject<FFMatrixBlock>(0, j, numRows, smallColNum);
		auto outData = smallBlock->data->data->c_ptr();
		      for (int32_t m = 0; m < numRows; m++) {
			for (int32_t n = 0; n < smallColNum; n++) {
				outData[m * smallColNum + n] = data[smallColNum * j + m * numCols + n];
			}
		      }
		smallBlock->data->bias = makeObject<Vector<float>>(smallColNum, smallColNum);
		auto outBias = smallBlock->data->bias->c_ptr();
		for (int32_t n = 0; n < smallColNum; n++) {
			outBias[n] = bias[n + smallColNum * j];
		}
		out.push_back(smallBlock);
		}
	}

      return out;
    });
  }

  // how many rows does the whole matrix have
  int32_t smallColNum{};

};
}
}
