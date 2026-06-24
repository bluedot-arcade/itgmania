#ifndef CACHING_ALPHA_TEST_STATE_H
#define CACHING_ALPHA_TEST_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"

struct AlphaTestState {
  static constexpr std::size_t SlotCount = 1;

  bool enabled;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const AlphaTestState& other) const {
    return enabled == other.enabled;
  }
  CacheResult Apply() const {
    glAlphaFunc(GL_GREATER, 0.00390625f);
    if (enabled) {
      glEnable(GL_ALPHA_TEST);
    } else {
      glDisable(GL_ALPHA_TEST);
    }
    return CacheResult::Cache;
  }
};

#endif
