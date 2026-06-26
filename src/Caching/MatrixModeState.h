#ifndef CACHING_MATRIX_MODE_STATE_H
#define CACHING_MATRIX_MODE_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"

struct MatrixModeState {
  static constexpr std::size_t SlotCount = 1;

  GLenum mode;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const MatrixModeState& other) const {
    return mode == other.mode;
  }

  CacheResult Apply() const {
    glMatrixMode(mode);
    return CacheResult::Cache;
  }
};

#endif
