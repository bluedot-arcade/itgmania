#ifndef CACHING_LIGHTING_STATE_H
#define CACHING_LIGHTING_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"

struct LightingState {
  static constexpr std::size_t SlotCount = 1;

  bool enabled;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const LightingState& other) const {
    return enabled == other.enabled;
  }
  CacheResult Apply() const {
    if (enabled) {
      glEnable(GL_LIGHTING);
    } else {
      glDisable(GL_LIGHTING);
    }
    return CacheResult::Cache;
  }
  static bool ReadCurrent() {
    GLboolean enabled = GL_FALSE;
    glGetBooleanv(GL_LIGHTING, &enabled);
    return enabled != GL_FALSE;
  }
};

#endif
