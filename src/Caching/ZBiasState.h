#ifndef CACHING_Z_BIAS_STATE_H
#define CACHING_Z_BIAS_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"

struct ZBiasState {
  static constexpr std::size_t SlotCount = 1;

  float bias;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const ZBiasState& other) const { return bias == other.bias; }
  CacheResult Apply() const {
    const float nearValue = 0.05f - (0.05f * bias);
    const float farValue = 1.0f - (0.05f * bias);
    glDepthRange(nearValue, farValue);
    return CacheResult::Cache;
  }
};

#endif
