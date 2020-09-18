#pragma once

#include <Object.h>
#include <PDBVector.h>

namespace pdb {

// the sub namespace
namespace ff {

class FFMatrixData : public pdb::Object {
 public:

  /**
   * The default constructor
   */
  FFMatrixData() = default;

  FFMatrixData(uint32_t numRows, uint32_t numCols, int32_t rowID, int32_t colID) : numRows(numRows),
                                                                                     numCols(numCols),
                                                                                     rowID(rowID),
                                                                                     colID(colID) {
    // allocate the data
    data = makeObject<Vector<float>>(numRows * numCols, numRows * numCols);
  }

  ENABLE_DEEP_COPY

  /**
   * The row id of the matrix
   */
  int32_t rowID;

  /**
   * the column id of the matrix
   */
  int32_t colID;

  /**
   * The number of rows in the block
   */
  uint32_t numRows = 0;

  /**
   * The number of columns in the block
   */
  uint32_t numCols = 0;

  /**
   * The values of the block
   */
  Handle<Vector<float>> data;

  /**
   * The values of the bias
   */
  Handle<Vector<float>> bias;

  #ifdef BLOCK_TO_ROW_STRIP
  /**
   * Does the aggregate from block to row-strip
   * @param other - the other
   * @return
   */
  FFMatrixData& operator+(FFMatrixData& other) {

    // get the data
    float *myData = NULL;
    float *otherData = NULL;

    std::cout << "I am calling block_to_row_strip aggregate!\n\n";

    std::cout << "my rowID: " << rowID << "\n";
    std::cout << "my colID: " << colID << "\n";
    std::cout << "other rowID: " << other.rowID << "\n";
    std::cout << "other colID: " << other.colID << "\n";

	if (data->c_ptr() == nullptr)
		std::cout << "data is null!\n";
	if (other.data->c_ptr() == nullptr)
		std::cout << "other data is null!\n";

    // we have several conditions
    //
    // condition 1: both matrices are just one block
    //
    if (colID >= 0 && other.colID >= 0) {
	std::cout << "I enter condition1\n";
	int32_t bigColID = 0;
	int32_t smallColID = 0;
	if (data->c_ptr() == nullptr)
		std::cout << "data is null!\n";
	if (other.data->c_ptr() == nullptr)
		std::cout << "other data is null!\n";
   	if (colID > other.colID) {
		bigColID = colID;
		smallColID = other.colID;
		myData = data->c_ptr();
		otherData = other.data->c_ptr();
	}
	else {
		bigColID = other.colID;
		smallColID = colID;
		myData = other.data->c_ptr();
		otherData = data->c_ptr();
	}

	// create a new big matrixData
	//
	std::cout << "debug information: \n";
	std::cout << "numRows: " << numRows << "\n";
	std::cout << "numCols: " << numCols << "\n";
	std::cout << "big colID: " << bigColID << "\n";
	std::cout << "small colID: " << smallColID << "\n";

	int32_t newNumCols = numCols * (bigColID + 1);
	//Handle<FFMatrixData> newMatrixData = makeObject<FFMatrixData>(numRows, newNumCols, rowID, -bigColID);
	

	//std::cout << "new numCols: " << newNumCols << "\n";
	//std::cout << "create a new matrix successfully!\n";


	//float *newData = newMatrixData->data->c_ptr();
	Handle<Vector<float>> newData = makeObject<Vector<float>>(numRows * newNumCols, numRows * newNumCols);
	Handle<Vector<float>> newBias = nullptr;

	// find the pos for this and other
	for (int i = 0; i < numRows; i++) {
		for (int j = 0; j < numCols; j++) {
			(newData->c_ptr())[i * newNumCols + numCols * bigColID + j] = (myData)[i * numCols + j];
			(newData->c_ptr())[i * newNumCols + numCols * smallColID + j] = (otherData)[i * numCols + j];
		}
	}

	// sup up the bios if we need to
	if(bias != nullptr && other.bias != nullptr) {
		std::cout << "I am in bias!\n";
		if (colID > other.colID) {
			myData = bias->c_ptr();
			otherData = other.bias->c_ptr();
		}
		else {
			myData = other.bias->c_ptr();
			otherData = bias->c_ptr();
		}
		uint32_t newBiasSize = bias->size() * (bigColID + 1);
		//newMatrixData->bias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
		newBias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
		for (int i = 0; i < bias->size(); i++) {
			(newBias->c_ptr())[bias->size() * bigColID + i] = (myData)[i];
			(newBias->c_ptr())[bias->size() * smallColID + i] = (otherData)[i];
		}
	}
	else if(bias != nullptr && other.bias == nullptr) {
		std::cout << "I am in bias2!\n";
		uint32_t newBiasSize = bias->size() * (bigColID + 1);
		//newMatrixData->bias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
		newBias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
		for (int i = 0; i < bias->size(); i++) {
			(newBias->c_ptr())[bias->size() * colID + i] = (bias->c_ptr())[i];
		}
	}
	else if(bias == nullptr && other.bias != nullptr) {
		std::cout << "I am in bias3!\n";
		uint32_t newBiasSize = other.bias->size() * (bigColID + 1);
		//newMatrixData->bias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
		newBias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
		for (int i = 0; i < other.bias->size(); i++) {
			(newBias->c_ptr())[other.bias->size() * other.colID + i] = (other.bias->c_ptr())[i];
		}
	}


	// assign new values to this.data
	data = makeObject<Vector<float>>(numRows * newNumCols, numRows * newNumCols);
        for (int i = 0; i < numRows * newNumCols; i++) {
		(data->c_ptr())[i] = (newData->c_ptr())[i];
        }
	
	// assign new values to this.bias
	if(newBias != nullptr) {
		bias = makeObject<Vector<float>>(newBias->size(), newBias->size());
	        for (int i = 0; i < newBias->size(); i++) {
			(bias->c_ptr())[i] = (newBias->c_ptr())[i];
        	}
	}

	numCols = newNumCols;
	colID = -bigColID;

        std::cout << "new rowID in condition1: " << (*this).rowID << "\n";
        std::cout << "new colID in condition1: " << (*this).colID << "\n";
	return *this;
    }

    // condition 2: one matrix is one block, the other matrix is an aggregated big block
    //
    else if ((colID >= 0 && other.colID < 0) || (colID < 0 && other.colID >= 0)) {
	int32_t bigColID = 0;
        int32_t smallColID = 0;
	uint32_t bigNumCols = 0;
	uint32_t smallNumCols = 0;
	FFMatrixData& result = *this;

	if (colID >= 0 && other.colID < 0) {
		bigNumCols = other.numCols;
                smallNumCols = numCols;
		myData = data->c_ptr();
                otherData = other.data->c_ptr();
		if (colID < -other.colID) {
			bigColID = other.colID;
			smallColID = colID;
			result = other;
		}	
		else {
			bigColID = colID;
			smallColID = -other.colID;
		}
	}
	// colID < 0 && other.colID >= 0
	else {
		bigNumCols = numCols;
                smallNumCols = other.numCols;
		myData = other.data->c_ptr();
                otherData = data->c_ptr();
		if (other.colID < -colID) {
			bigColID = colID;
			smallColID = other.colID;
			//result = *this;
		}	
		else {
			bigColID = other.colID;
			smallColID = -colID;
		}
	}

	// if we do not need to create a new matrix
	// that is, the matrix block is within the aggregated matrix
	if (bigColID < 0) {
		for (int i = 0; i < numRows; i++) {
			for (int j = 0; j < smallNumCols; j++) {
				(otherData)[i * bigNumCols + smallNumCols * smallColID + j] = (myData)[i * smallNumCols + j];
			}
		}
		// sup up the bios if we need to
		if(bias != nullptr && other.bias != nullptr) {
			uint32_t smallBiasSize = 0;
			if (bigColID != colID) {
				myData = bias->c_ptr();
                        	otherData = other.bias->c_ptr();
				smallBiasSize = bias->size();
			}
			else {
				otherData = bias->c_ptr();
                        	myData = other.bias->c_ptr();
				smallBiasSize = other.bias->size();
			}

			for (int i = 0; i < smallBiasSize; i++) {
				(otherData)[smallBiasSize * smallColID + i] = (myData)[i];
			}
		}
		else if(bias != nullptr && other.bias == nullptr) {
			if (bigColID != colID) {
				uint32_t newBiasSize = bias->size() * (-bigColID + 1);
				other.bias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
				for (int i = 0; i < bias->size(); i++) { 
                                	(other.bias->c_ptr())[bias->size() * smallColID + i] = (bias->c_ptr())[i];
                        	}

			}
			// otherwise, we do not need to assgin bias
		}		
        	else if(bias == nullptr && other.bias != nullptr) {
			if (bigColID == colID) {
				uint32_t newBiasSize = other.bias->size() * (-bigColID + 1);
				bias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
				for (int i = 0; i < other.bias->size(); i++) { 
                                	(bias->c_ptr())[other.bias->size() * smallColID + i] = (other.bias->c_ptr())[i];
                        	}

			}
			// otherwise, we do not need to assgin bias
		}

        	std::cout << "new rowID in condition2.1: " << result.rowID << "\n";
        	std::cout << "new colID in condition2.1: " << result.colID << "\n";
	
		return result;
	}
	// we need to create a new matrix
	else {
		// create a new big matrixData
		uint32_t newNumCols = smallNumCols * (bigColID + 1);
		Handle<Vector<float>> newData = makeObject<Vector<float>>(numRows * newNumCols, numRows * newNumCols);
        	Handle<Vector<float>> newBias = nullptr;

		//Handle<FFMatrixData> newMatrixData = makeObject<FFMatrixData>(numRows, newNumCols, rowID, -bigColID);
		// find the pos for this and other
		for (int i = 0; i < numRows; i++) {
			// assign data for the small matrix
			for (int j = 0; j < smallNumCols; j++) {
				(newData->c_ptr())[i * newNumCols + smallNumCols * bigColID + j] = (myData)[i * smallNumCols + j];
			}
			// assign data for the big matrix
			for (int j = 0; j < bigNumCols; j++) {
				(newData->c_ptr())[i * newNumCols + j] = (otherData)[i * bigNumCols + j];
			}
		}

		// sup up the bios if we need to
		if(bias != nullptr && other.bias != nullptr) {
			uint32_t smallBiasSize = 0;
			uint32_t bigBiasSize = 0;
			if (bigColID == colID) {
				myData = bias->c_ptr();
                        	otherData = other.bias->c_ptr();
				smallBiasSize = bias->size();
				bigBiasSize = other.bias->size();
			}
			else {
				otherData = bias->c_ptr();
                        	myData = other.bias->c_ptr();
				smallBiasSize = other.bias->size();
				bigBiasSize = bias->size();
			}

			uint32_t newBiasSize = smallBiasSize * (bigColID + 1);
			newBias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
			// assign data for the small matrix
			for (int i = 0; i < smallBiasSize; i++) {
				(newBias->c_ptr())[smallBiasSize * bigColID + i] = (myData)[i];
			}
			// assign data for the big matrix
			for (int i = 0; i < bigBiasSize; i++) {
				(newBias->c_ptr())[i] = (otherData)[i];
			}
		}
		else if(bias != nullptr && other.bias == nullptr) {
			uint32_t newBiasSize = 0;
			if(bigColID == colID) {
				newBiasSize = bias->size() * (bigColID + 1);
                        	newBias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
				for (int i = 0; i < bias->size(); i++) {
					(newBias->c_ptr())[bias->size() * colID + i] = (bias->c_ptr())[i];
				}
			}
			else {
				newBiasSize = bias->size() / (-colID+1) * (bigColID + 1);
				newBias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
				for (int i = 0; i < bias->size(); i++) {
					(newBias->c_ptr())[i] = (bias->c_ptr())[i];
				}	
			}
        	}
		else if(bias == nullptr && other.bias != nullptr) {
			uint32_t newBiasSize = 0;
			if(bigColID == other.colID) {
				newBiasSize = other.bias->size() * (bigColID + 1);
                        	newBias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
				for (int i = 0; i < other.bias->size(); i++) {
					(newBias->c_ptr())[other.bias->size() * other.colID + i] = (other.bias->c_ptr())[i];
				}
			}
			else {
				newBiasSize = other.bias->size() / (-other.colID+1) * (bigColID + 1);
				newBias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
				for (int i = 0; i < other.bias->size(); i++) {
					(newBias->c_ptr())[i] = (other.bias->c_ptr())[i];
				}	
			}
		}

		// assign new values to this.data
		data = makeObject<Vector<float>>(numRows * newNumCols, numRows * newNumCols);
		for (int i = 0; i < numRows * newNumCols; i++) {
			(data->c_ptr())[i] = (newData->c_ptr())[i];
		}

		// assign new values to this.bias
		if(newBias != nullptr) {
			bias = makeObject<Vector<float>>(newBias->size(), newBias->size());
			for (int i = 0; i < newBias->size(); i++) {
				(bias->c_ptr())[i] = (newBias->c_ptr())[i];
			}
		}

		numCols = newNumCols;
		colID = -bigColID;

		std::cout << "new rowID in condition2.2: " << (*this).rowID << "\n";
		std::cout << "new colID in condition2.2: " << (*this).colID << "\n";
		return *this;
	}

    }

	
    // condition 3: both matrices are aggregated big blocks
    //
    else {
	// this is a bigger matrix, assign other to this
	if (colID < other.colID) {
		myData = data->c_ptr();
                otherData = other.data->c_ptr();

                for (int i = 0; i < numRows; i++) {
                        for (int j = 0; j < other.numCols; j++) {
                                (myData)[i * numCols + j] += (otherData)[i * other.numCols + j];
                        }
                }
                // sup up the bios if we need to
                if(bias != nullptr && other.bias != nullptr) {
                        myData = bias->c_ptr();
                        otherData = other.bias->c_ptr();
			for (int i = 0; i < other.bias->size(); i++) {
				(myData)[i] += (otherData)[i];
			}
                }
                else if(bias == nullptr && other.bias != nullptr) {
                	uint32_t newBiasSize = other.bias->size() / (-other.colID+1) * (-colID + 1);;
                	bias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
                	for (int i = 0; i < other.bias->size(); i++) {
                        	(bias->c_ptr())[i] = (other.bias->c_ptr())[i];
                	}
                }

        std::cout << "new rowID in condition3.1: " << (*this).rowID << "\n";
        std::cout << "new colID in condition3.1: " << (*this).colID << "\n";
	
                return *this;
	}
	// other is a bigger matrix, assign this to other
	else {
		myData = other.data->c_ptr();
                otherData = data->c_ptr();

                for (int i = 0; i < numRows; i++) {
                        for (int j = 0; j < numCols; j++) {
                                (myData)[i * other.numCols + j] += (otherData)[i * numCols + j];
                        }
                }
                // sup up the bios if we need to
                if(bias != nullptr && other.bias != nullptr) {
                        otherData = bias->c_ptr();
                        myData = other.bias->c_ptr();
                        for (int i = 0; i < bias->size(); i++) {
                                (myData)[i] += (otherData)[i];
                        }
                }
		else if(bias != nullptr && other.bias == nullptr) {
                        uint32_t newBiasSize = bias->size() / (-colID+1) * (-other.colID + 1);;
                        other.bias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
                        for (int i = 0; i < bias->size(); i++) {
                                (other.bias->c_ptr())[i] = (bias->c_ptr())[i];
                        }
                }
        std::cout << "new rowID in condition3.2: " << other.rowID << "\n";
        std::cout << "new colID in condition3.2: " << other.colID << "\n";
	
                return other;
	}
    }

    // return me
    //return *this;
  }

  #elif BLOCK_TO_COL_STRIP
  /**
   * Does the aggregate from block to row-strip
   * @param other - the other
   * @return
   */
  FFMatrixData& operator+(FFMatrixData& other) {

    // get the data
    float *myData = NULL;
    float *otherData = NULL;

    std::cout << "I am calling block_to_col_strip aggregate!\n\n";

    std::cout << "my rowID: " << rowID << "\n";
    std::cout << "my colID: " << colID << "\n";
    std::cout << "other rowID: " << other.rowID << "\n";
    std::cout << "other colID: " << other.colID << "\n";

    // we have several conditions
    //
    // condition 1: both matrices are just one block
    //
    if (rowID >= 0 && other.rowID >= 0) {
	std::cout << "I enter condition1\n";
	int32_t bigRowID = 0;
	int32_t smallRowID = 0;
   	if (rowID > other.rowID) {
		bigRowID = rowID;
		smallRowID = other.rowID;
		myData = data->c_ptr();
		otherData = other.data->c_ptr();
	}
	else {
		bigRowID = other.rowID;
		smallRowID = rowID;
		myData = other.data->c_ptr();
		otherData = data->c_ptr();
	}

	// create a new big matrixData
	//
	std::cout << "debug information: \n";
	std::cout << "numRows: " << numRows << "\n";
	std::cout << "numCols: " << numCols << "\n";
	std::cout << "big rowID: " << bigRowID << "\n";
	std::cout << "small rowID: " << smallRowID << "\n";

	int32_t newNumRows = numRows * (bigRowID + 1);

	Handle<Vector<float>> newData = makeObject<Vector<float>>(newNumRows * numCols, newNumRows * numCols);
	//Handle<Vector<float>> newBias = nullptr;

	// find the pos for this and other
	for (int i = 0; i < numRows; i++) {
		for (int j = 0; j < numCols; j++) {
			(newData->c_ptr())[bigRowID * numRows * numCols + i * numCols + j] = (myData)[i * numCols + j];
			(newData->c_ptr())[smallRowID * numRows * numCols + i * numCols + j] = (otherData)[i * numCols + j];
		}
	}

	// sup up the bios if we need to
	// Note that all row blocks should have only one bias
	/*
	else if(bias != nullptr && other.bias == nullptr) {
		std::cout << "I am in bias2!\n";
		//uint32_t newBiasSize = bias->size() * (bigRowID + 1);
		newBias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
		for (int i = 0; i < bias->size(); i++) {
			(newBias->c_ptr())[bias->size() * colID + i] = (bias->c_ptr())[i];
		}
	}
	*/
	if(bias == nullptr && other.bias != nullptr) {
		std::cout << "I am in bias3!\n";
		//uint32_t newBiasSize = other.bias->size() * (bigColID + 1);
		bias = makeObject<Vector<float>>(other.bias->size(), other.bias->size());
		for (int i = 0; i < other.bias->size(); i++) {
			(bias->c_ptr())[i] = (other.bias->c_ptr())[i];
		}
	}
	else if(bias != nullptr && other.bias != nullptr) {
		std::cout << "I am in bias! This should not happen!!\n";
	}



	// assign new values to this.data
	data = makeObject<Vector<float>>(newNumRows * numCols, newNumRows * numCols);
        for (int i = 0; i < newNumRows * numCols; i++) {
		(data->c_ptr())[i] = (newData->c_ptr())[i];
        }
	
	// assign new values to this.bias
	/*
	if(newBias != nullptr) {
		bias = makeObject<Vector<float>>(newBias->size(), newBias->size());
	        for (int i = 0; i < newBias->size(); i++) {
			(bias->c_ptr())[i] = (newBias->c_ptr())[i];
        	}
	}
	*/

	numRows = newNumRows;
	rowID = -bigRowID;

        std::cout << "new rowID in condition1: " << (*this).rowID << "\n";
        std::cout << "new colID in condition1: " << (*this).colID << "\n";
	return *this;
    }

    // condition 2: one matrix is one block, the other matrix is an aggregated big block
    //
    else if ((rowID >= 0 && other.rowID < 0) || (rowID < 0 && other.rowID >= 0)) {
	int32_t bigRowID = 0;
        int32_t smallRowID = 0;
	uint32_t bigNumRows = 0;
	uint32_t smallNumRows = 0;
	FFMatrixData& result = *this;

	if (rowID >= 0 && other.rowID < 0) {
		bigNumRows = other.numRows;
                smallNumRows = numRows;
		myData = data->c_ptr();
                otherData = other.data->c_ptr();
		if (rowID < -other.rowID) {
			bigRowID = other.rowID;
			smallRowID = rowID;
			result = other;
		}	
		else {
			bigRowID = rowID;
			smallRowID = -other.rowID;
		}
	}
	// rowID < 0 && other.rowID >= 0
	else {
		bigNumRows = numRows;
                smallNumRows = other.numRows;
		myData = other.data->c_ptr();
                otherData = data->c_ptr();
		if (other.rowID < -rowID) {
			bigRowID = rowID;
			smallRowID = other.rowID;
			//result = *this;
		}	
		else {
			bigRowID = other.rowID;
			smallRowID = -rowID;
		}
	}

	// if we do not need to create a new matrix
	// that is, the matrix block is within the aggregated matrix
	if (bigRowID < 0) {
		for (int i = 0; i < smallNumRows; i++) {
			for (int j = 0; j < numCols; j++) {
				(otherData)[smallRowID * smallNumRows * numCols + i * numCols + j] = (myData)[i * numCols + j];
			}
		}
		// sup up the bios if we need to
		if(bias != nullptr && other.bias == nullptr) {
			if (bigRowID != rowID) {
				//uint32_t newBiasSize = bias->size() * (-bigRowID + 1);
				other.bias = makeObject<Vector<float>>(bias->size(), bias->size());
				for (int i = 0; i < bias->size(); i++) { 
                                	(other.bias->c_ptr())[i] = (bias->c_ptr())[i];
                        	}

			}
			// otherwise, we do not need to assgin bias
		}		
        	else if(bias == nullptr && other.bias != nullptr) {
			if (bigRowID == rowID) {
				//uint32_t newBiasSize = other.bias->size() * (-bigColID + 1);
				bias = makeObject<Vector<float>>(other.bias->size(), other.bias->size());
				for (int i = 0; i < other.bias->size(); i++) { 
                                	(bias->c_ptr())[i] = (other.bias->c_ptr())[i];
                        	}

			}
			// otherwise, we do not need to assgin bias
		}
		else if(bias != nullptr && other.bias != nullptr) {
			std::cout << "I am in bias! This should not happen!!\n";
		}


        	std::cout << "new rowID in condition2.1: " << result.rowID << "\n";
        	std::cout << "new colID in condition2.1: " << result.colID << "\n";
	
		return result;
	}
	// we need to create a new matrix
	// bigRowID > 0
	else {
		// create a new big matrixData
		uint32_t newNumRows = smallNumRows * (bigRowID + 1);
	//std::cout << "debug information in condition 2.2: \n";
	//std::cout << "small numRows: " << smallNumRows << "\n";
	//std::cout << "big numRows: " << bigNumRows << "\n";
	//std::cout << "numCols: " << numCols << "\n";
	//std::cout << "new numRows: " << newNumRows << "\n";
	//std::cout << "small numRows: " << smallNumRows << "\n";


		Handle<Vector<float>> newData = makeObject<Vector<float>>(newNumRows * numCols, newNumRows * numCols);

		// find the pos for this and other
		for (int i = 0; i < smallNumRows; i++) {
			// assign data for the small matrix
			for (int j = 0; j < numCols; j++) {
				(newData->c_ptr())[bigRowID * smallNumRows * numCols + i * numCols + j] = (myData)[i * numCols + j];
			}
		}

		for (int i = 0; i < bigNumRows; i++) {
			// assign data for the big matrix
			for (int j = 0; j < numCols; j++) {
				(newData->c_ptr())[i * numCols + j] = (otherData)[i * numCols + j];
			}
		}

		// sup up the bios if we need to
		/*
		if(bias != nullptr && other.bias != nullptr) {
			std::cout << "I am in bias! This should not happen!!\n";
		}
		else if(bias != nullptr && other.bias == nullptr) {
			uint32_t newBiasSize = 0;
			if(bigRowID == rowID) {
				//newBiasSize = bias->size() * (bigRowID + 1);
                        	newBias = makeObject<Vector<float>>(bias->size(), bias->size());
				for (int i = 0; i < bias->size(); i++) {
					(newBias->c_ptr())[i] = (bias->c_ptr())[i];
				}
			}
			else {
				newBiasSize = bias->size() / (-colID+1) * (bigColID + 1);
				newBias = makeObject<Vector<float>>(newBiasSize, newBiasSize);
				for (int i = 0; i < bias->size(); i++) {
					(newBias->c_ptr())[i] = (bias->c_ptr())[i];
				}	
			}
        	}*/
		if(bias == nullptr && other.bias != nullptr) {
			bias = makeObject<Vector<float>>(other.bias->size(), other.bias->size());
			for (int i = 0; i < other.bias->size(); i++) {
					(bias->c_ptr())[i] = (other.bias->c_ptr())[i];
			}
		}
		else if(bias != nullptr && other.bias != nullptr) {
			std::cout << "I am in bias! This should not happen!!\n";
		}


		// assign new values to this.data
		data = makeObject<Vector<float>>(newNumRows * numCols, newNumRows * numCols);
		for (int i = 0; i < newNumRows * numCols; i++) {
			(data->c_ptr())[i] = (newData->c_ptr())[i];
		}

		// assign new values to this.bias
		/*
		if(newBias != nullptr) {
			bias = makeObject<Vector<float>>(newBias->size(), newBias->size());
			for (int i = 0; i < newBias->size(); i++) {
				(bias->c_ptr())[i] = (newBias->c_ptr())[i];
			}
		}*/

		numRows = newNumRows;
		rowID = -bigRowID;

		std::cout << "new rowID in condition2.2: " << (*this).rowID << "\n";
		std::cout << "new colID in condition2.2: " << (*this).colID << "\n";
		return *this;
	}

    }

	
    // condition 3: both matrices are aggregated big blocks
    //
    else {
	// this is a bigger matrix, assign other to this
	if (rowID < other.rowID) {
		myData = data->c_ptr();
                otherData = other.data->c_ptr();

                for (int i = 0; i < other.numRows; i++) {
                        for (int j = 0; j < numCols; j++) {
                                (myData)[i * numCols + j] += (otherData)[i * numCols + j];
                        }
                }
                // sup up the bios if we need to
                if(bias == nullptr && other.bias != nullptr) {
                	bias = makeObject<Vector<float>>(other.bias->size(), other.bias->size());
                	for (int i = 0; i < other.bias->size(); i++) {
                        	(bias->c_ptr())[i] = (other.bias->c_ptr())[i];
                	}
                }
		else if(bias != nullptr && other.bias != nullptr) {
			std::cout << "I am in bias! This should not happen!!\n";
                }


        std::cout << "new rowID in condition3.1: " << (*this).rowID << "\n";
        std::cout << "new colID in condition3.1: " << (*this).colID << "\n";
	
                return *this;
	}
	// other is a bigger matrix, assign this to other
	else {
		myData = other.data->c_ptr();
                otherData = data->c_ptr();

                for (int i = 0; i < numRows; i++) {
                        for (int j = 0; j < numCols; j++) {
                                (myData)[i * numCols + j] += (otherData)[i * numCols + j];
                        }
                }
                // sup up the bios if we need to
		if(bias != nullptr && other.bias == nullptr) {
                        other.bias = makeObject<Vector<float>>(bias->size(), bias->size());
                        for (int i = 0; i < bias->size(); i++) {
                                (other.bias->c_ptr())[i] = (bias->c_ptr())[i];
                        }
                }
		else if(bias != nullptr && other.bias != nullptr) {
			std::cout << "I am in bias! This should not happen!!\n";
                }

        std::cout << "new rowID in condition3.2: " << other.rowID << "\n";
        std::cout << "new colID in condition3.2: " << other.colID << "\n";
	
                return other;
	}
    }

    // return me
    //return *this;
  }


  #else
  /**
   * Does the summation of the data
   * @param other - the other
   * @return
   */
  FFMatrixData& operator+(FFMatrixData& other) {

    // get the data
    float *myData = data->c_ptr();
    float *otherData = other.data->c_ptr();

    std::cout << "I am calling normal aggregate!\n\n";

    // sum up the data
    for (int i = 0; i < numRows * numCols; i++) {
      (myData)[i] += (otherData)[i];
    }

    // sup up the bios if we need to
    if(bias != nullptr && other.bias != nullptr) {
      myData = bias->c_ptr();
      otherData = other.bias->c_ptr();
      for (int i = 0; i < other.bias->size(); i++) {
        (myData)[i] += (otherData)[i];
      }
    }

    // return me
    return *this;
  }

  #endif
};

}

}
