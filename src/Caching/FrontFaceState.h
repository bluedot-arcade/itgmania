#ifndef CACHING_FRONT_FACE_STATE_H
#define CACHING_FRONT_FACE_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"

struct FrontFaceState {
  static constexpr std::size_t SlotCount = 1;

  GLenum mode;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const FrontFaceState& other) const {
    return mode == other.mode;
  }

  CacheResult Apply() const {
    glFrontFace(mode);
    return CacheResult::Cache;
  }
};

#endif
