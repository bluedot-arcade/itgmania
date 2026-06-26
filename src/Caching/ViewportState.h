#ifndef CACHING_VIEWPORT_STATE_H
#define CACHING_VIEWPORT_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"

struct ViewportState {
  static constexpr std::size_t SlotCount = 1;

  GLint x;
  GLint y;
  GLsizei width;
  GLsizei height;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const ViewportState& other) const {
    return x == other.x && y == other.y && width == other.width &&
           height == other.height;
  }

  CacheResult Apply() const {
    glViewport(x, y, width, height);
    return CacheResult::Cache;
  }
};

#endif
