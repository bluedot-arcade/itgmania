#ifndef CACHING_CLIENT_ACTIVE_TEXTURE_STATE_H
#define CACHING_CLIENT_ACTIVE_TEXTURE_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"

struct ClientActiveTextureState {
  static constexpr std::size_t SlotCount = 1;

  GLenum unit;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const ClientActiveTextureState& other) const {
    return unit == other.unit;
  }

  CacheResult Apply() const {
    glClientActiveTextureARB(unit);
    return CacheResult::Cache;
  }
};

#endif
