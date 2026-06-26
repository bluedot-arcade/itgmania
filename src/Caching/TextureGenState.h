#ifndef CACHING_TEXTURE_GEN_STATE_H
#define CACHING_TEXTURE_GEN_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"
#include "RageDisplay.h"

struct TextureGenState {
  static constexpr std::size_t SlotCount = NUM_TextureUnit;

  TextureUnit unit;
  bool enabled;

  std::size_t CacheIndex() const { return static_cast<std::size_t>(unit); }
  bool operator==(const TextureGenState& other) const {
    return unit == other.unit && enabled == other.enabled;
  }

  CacheResult Apply() const {
    if (enabled) {
      glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      glEnable(GL_TEXTURE_GEN_S);
      glEnable(GL_TEXTURE_GEN_T);
    } else {
      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);
    }
    return CacheResult::Cache;
  }

  static bool ReadCurrent() {
    GLboolean textureGenS = GL_FALSE;
    GLboolean textureGenT = GL_FALSE;
    glGetBooleanv(GL_TEXTURE_GEN_S, &textureGenS);
    glGetBooleanv(GL_TEXTURE_GEN_T, &textureGenT);
    return textureGenS != GL_FALSE || textureGenT != GL_FALSE;
  }
};

#endif
