#ifndef CACHING_CLEAR_COLOR_STATE_H
#define CACHING_CLEAR_COLOR_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"

struct ClearColorState {
  static constexpr std::size_t SlotCount = 1;

  float r;
  float g;
  float b;
  float a;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const ClearColorState& other) const {
    return r == other.r && g == other.g && b == other.b && a == other.a;
  }

  CacheResult Apply() const {
    glClearColor(r, g, b, a);
    return CacheResult::Cache;
  }
};

#endif
