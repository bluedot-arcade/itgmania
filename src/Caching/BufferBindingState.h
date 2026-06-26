#ifndef CACHING_BUFFER_BINDING_STATE_H
#define CACHING_BUFFER_BINDING_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"

struct BufferBindingState {
  static constexpr std::size_t SlotCount = 3;

  GLenum target;
  GLuint buffer;

  std::size_t CacheIndex() const {
    switch (target) {
      case GL_ARRAY_BUFFER_ARB:
        return 0;
      case GL_ELEMENT_ARRAY_BUFFER_ARB:
        return 1;
      case GL_PIXEL_UNPACK_BUFFER_ARB:
        return 2;
      default:
        return 0;
    }
  }
  bool operator==(const BufferBindingState& other) const {
    return target == other.target && buffer == other.buffer;
  }

  CacheResult Apply() const {
    switch (target) {
      case GL_ARRAY_BUFFER_ARB:
      case GL_ELEMENT_ARRAY_BUFFER_ARB:
      case GL_PIXEL_UNPACK_BUFFER_ARB:
        glBindBufferARB(target, buffer);
        return CacheResult::Cache;
      default:
        glBindBufferARB(target, buffer);
        return CacheResult::DoNotCache;
    }
  }
};

#endif
