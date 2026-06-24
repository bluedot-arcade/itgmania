#ifndef CACHING_Z_WRITE_STATE_H
#define CACHING_Z_WRITE_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"

struct ZWriteState {
  static constexpr std::size_t SlotCount = 1;

  bool enabled;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const ZWriteState& other) const {
    return enabled == other.enabled;
  }
  CacheResult Apply() const {
    glDepthMask(enabled ? GL_TRUE : GL_FALSE);
    return CacheResult::Cache;
  }
  static bool ReadCurrent() {
    GLboolean enabled = GL_FALSE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &enabled);
    return enabled != GL_FALSE;
  }
};

#endif
