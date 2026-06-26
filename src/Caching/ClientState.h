#ifndef CACHING_CLIENT_STATE_H
#define CACHING_CLIENT_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"
#include "RageDisplay.h"

struct ClientState {
  static constexpr std::size_t SlotCount = 3 + NUM_TextureUnit;

  GLenum state;
  std::size_t textureUnitIndex;
  bool enabled;

  std::size_t CacheIndex() const {
    switch (state) {
      case GL_VERTEX_ARRAY:
        return 0;
      case GL_COLOR_ARRAY:
        return 1;
      case GL_NORMAL_ARRAY:
        return 2;
      case GL_TEXTURE_COORD_ARRAY:
        return 3 + textureUnitIndex;
      default:
        return 0;
    }
  }
  bool operator==(const ClientState& other) const {
    return state == other.state && textureUnitIndex == other.textureUnitIndex &&
           enabled == other.enabled;
  }

  CacheResult Apply() const {
    switch (state) {
      case GL_VERTEX_ARRAY:
      case GL_COLOR_ARRAY:
      case GL_NORMAL_ARRAY:
      case GL_TEXTURE_COORD_ARRAY:
        if (enabled) {
          glEnableClientState(state);
        } else {
          glDisableClientState(state);
        }
        return CacheResult::Cache;
      default:
        if (enabled) {
          glEnableClientState(state);
        } else {
          glDisableClientState(state);
        }
        return CacheResult::DoNotCache;
    }
  }
};

#endif
