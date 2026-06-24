#ifndef CACHING_TEXTURE_MODE_STATE_H
#define CACHING_TEXTURE_MODE_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"
#include "RageDisplay.h"

struct TextureModeState {
  static constexpr std::size_t SlotCount = NUM_TextureUnit;

  TextureUnit unit;
  TextureMode mode;

  std::size_t CacheIndex() const { return static_cast<std::size_t>(unit); }
  bool operator==(const TextureModeState& other) const {
    return unit == other.unit && mode == other.mode;
  }

  CacheResult Apply() const {
    switch (mode) {
      case TextureMode_Modulate:
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        return CacheResult::Cache;
      case TextureMode_Add:
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
        return CacheResult::Cache;
      case TextureMode_Glow:
        if (!GLEW_ARB_texture_env_combine && !GLEW_EXT_texture_env_combine) {
          glBlendFunc(GL_SRC_ALPHA, GL_ONE);
          return CacheResult::DoNotCache;
        }

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
        glTexEnvi(GL_TEXTURE_ENV, GLenum(GL_COMBINE_RGB_EXT), GL_REPLACE);
        glTexEnvi(
            GL_TEXTURE_ENV, GLenum(GL_SOURCE0_RGB_EXT), GL_PRIMARY_COLOR_EXT);
        glTexEnvi(GL_TEXTURE_ENV, GLenum(GL_COMBINE_ALPHA_EXT), GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV, GLenum(GL_OPERAND0_ALPHA_EXT), GL_SRC_ALPHA);
        glTexEnvi(
            GL_TEXTURE_ENV, GLenum(GL_SOURCE0_ALPHA_EXT), GL_PRIMARY_COLOR_EXT);
        glTexEnvi(GL_TEXTURE_ENV, GLenum(GL_OPERAND1_ALPHA_EXT), GL_SRC_ALPHA);
        glTexEnvi(GL_TEXTURE_ENV, GLenum(GL_SOURCE1_ALPHA_EXT), GL_TEXTURE);
        return CacheResult::Cache;
      default:
        return CacheResult::DoNotCache;
    }
  }
};

#endif
