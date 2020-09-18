#pragma once

#include <Object.h>

namespace pdb {

// the sub namespace
namespace ff {

class FFMatrixMeta : public pdb::Object {
 public:

  /**
   * The default constructor
   */
  FFMatrixMeta() = default;

  FFMatrixMeta(int32_t row_id, int32_t col_id) : colID(col_id), rowID(row_id) {}

  ENABLE_DEEP_COPY

  /**
   * The column position of the block
   */
  int32_t colID = 0;

  /**
   * The row position of the block
   */
  int32_t rowID = 0;

  FFMatrixMeta getRowMeta() {
    return FFMatrixMeta{rowID, 0};
  } 

  FFMatrixMeta getColMeta() {
    return FFMatrixMeta{0, colID};
  } 

  bool operator==(const FFMatrixMeta &other) const {
    std::cout << "I am calling normal equal!\n\n";
    return colID == other.colID && rowID == other.rowID;
  }

  int32_t hash() const {
    return 10000 * rowID + colID;
  }
};

}

}
