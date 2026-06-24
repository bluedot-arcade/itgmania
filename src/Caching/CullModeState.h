#ifndef CACHING_CULL_MODE_STATE_H
#define CACHING_CULL_MODE_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "global.h"

struct CullModeState {
  static constexpr std::size_t SlotCount = 1;

  CullMode mode;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const CullModeState& other) const {
    return mode == other.mode;
  }
  CacheResult Apply() const {
    if (mode != CULL_NONE) {
      glEnable(GL_CULL_FACE);
    }
    switch (mode) {
      case CULL_BACK:
        glCullFace(GL_BACK);
        break;
      case CULL_FRONT:
        glCullFace(GL_FRONT);
        break;
      case CULL_NONE:
        glDisable(GL_CULL_FACE);
        break;
      default:
        FAIL_M(ssprintf("Invalid CullMode: %i", mode));
    }
    return CacheResult::Cache;
  }
};

#endif
