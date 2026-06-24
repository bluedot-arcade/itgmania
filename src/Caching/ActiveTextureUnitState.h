#ifndef CACHING_ACTIVE_TEXTURE_UNIT_STATE_H
#define CACHING_ACTIVE_TEXTURE_UNIT_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"
#include "RageDisplay.h"

struct ActiveTextureUnitState {
  static constexpr std::size_t SlotCount = 1;

  TextureUnit unit;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const ActiveTextureUnitState& other) const {
    return unit == other.unit;
  }
  CacheResult Apply() const {
    glActiveTextureARB(
        static_cast<GLenum>(GL_TEXTURE0_ARB + static_cast<int>(unit)));
    return CacheResult::Cache;
  }
};

#endif
