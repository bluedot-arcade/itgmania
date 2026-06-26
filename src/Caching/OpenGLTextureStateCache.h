#ifndef CACHING_OPENGL_TEXTURE_STATE_CACHE_H
#define CACHING_OPENGL_TEXTURE_STATE_CACHE_H

#include <array>
#include <cstddef>
#include <map>
#include <mutex>

#include <GL/glew.h>

#include "Caching/OpenGLStateCachePreference.h"
#include "RageDisplay.h"

struct OpenGLTextureObjectState {
  explicit OpenGLTextureObjectState(bool hasMipmaps)
      : hasMipmaps(hasMipmaps) {}

  bool hasMipmaps;
  GLint minFilter = -1;
  GLint magFilter = -1;
  GLint wrapMode = -1;
};

class OpenGLTextureStateCache {
 public:
  void RegisterTexture(uintptr_t handle, bool hasMipmaps) {
    if (handle == 0) {
      return;
    }
    if (!CachingEnabled()) {
      return;
    }

    std::lock_guard<std::mutex> lock(textureStatesMutex_);
    textureStates_.erase(handle);
    textureStates_.emplace(handle, OpenGLTextureObjectState(hasMipmaps));
  }

  void ForgetTexture(uintptr_t handle) {
    if (!CachingEnabled()) {
      return;
    }

    std::lock_guard<std::mutex> lock(textureStatesMutex_);
    textureStates_.erase(handle);
    for (uintptr_t& bound : boundTextures_) {
      if (bound == handle) {
        bound = 0;
      }
    }
  }

  void ForgetAllTextures() {
    if (!CachingEnabled()) {
      return;
    }

    std::lock_guard<std::mutex> lock(textureStatesMutex_);
    textureStates_.clear();
    boundTextures_.fill(0);
  }

  void BindTexture(TextureUnit unit, uintptr_t handle) {
    if (!CachingEnabled()) {
      return;
    }

    boundTextures_[static_cast<std::size_t>(unit)] = handle;
  }

  uintptr_t GetBoundTexture(TextureUnit unit) const {
    return boundTextures_[static_cast<std::size_t>(unit)];
  }

  void SetFiltering(TextureUnit unit, bool enabled, bool trilinearFiltering) {
    if (!CachingEnabled()) {
      ApplyFilteringDirect(enabled, trilinearFiltering);
      return;
    }

    const uintptr_t texture = GetBoundTexture(unit);
    if (texture == 0) {
      return;
    }

    const GLint magFilter = enabled ? GL_LINEAR : GL_NEAREST;
    GLint minFilter = GL_NEAREST;
    bool setMagFilter = true;
    bool setMinFilter = true;
    bool knownTexture = false;

    {
      std::lock_guard<std::mutex> lock(textureStatesMutex_);
      const auto it = textureStates_.find(texture);
      if (it != textureStates_.end()) {
        knownTexture = true;
        if (enabled) {
          if (it->second.hasMipmaps) {
            minFilter = trilinearFiltering ? GL_LINEAR_MIPMAP_LINEAR
                                           : GL_LINEAR_MIPMAP_NEAREST;
          } else {
            minFilter = GL_LINEAR;
          }
        }

        setMagFilter = it->second.magFilter != magFilter;
        setMinFilter = it->second.minFilter != minFilter;
        it->second.magFilter = magFilter;
        it->second.minFilter = minFilter;
      }
    }

    if (!knownTexture && enabled) {
      GLint widthLevel0 = -1;
      GLint widthLevel1 = -1;
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &widthLevel0);
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 1, GL_TEXTURE_WIDTH, &widthLevel1);
      if (widthLevel0 > 1 && widthLevel1 != 0) {
        minFilter = trilinearFiltering ? GL_LINEAR_MIPMAP_LINEAR
                                       : GL_LINEAR_MIPMAP_NEAREST;
      } else {
        minFilter = GL_LINEAR;
      }
    }

    if (setMagFilter) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    }
    if (setMinFilter) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    }
  }

  void SetWrapping(TextureUnit unit, bool enabled) {
    const GLint mode = enabled ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    if (!CachingEnabled()) {
      ApplyWrapping(mode);
      return;
    }

    const uintptr_t texture = GetBoundTexture(unit);
    bool setWrapping = true;

    {
      std::lock_guard<std::mutex> lock(textureStatesMutex_);
      const auto it = textureStates_.find(texture);
      if (it != textureStates_.end()) {
        setWrapping = it->second.wrapMode != mode;
        it->second.wrapMode = mode;
      }
    }

    if (!setWrapping) {
      return;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
  }

 private:
  bool CachingEnabled() {
    const bool enabled = g_bEnableOpenGLStateCache.Get();
    if (!preferenceKnown_ || enabled != lastPreferenceValue_) {
      ClearState();
      preferenceKnown_ = true;
      lastPreferenceValue_ = enabled;
    }
    return enabled;
  }

  void ClearState() {
    std::lock_guard<std::mutex> lock(textureStatesMutex_);
    textureStates_.clear();
    boundTextures_.fill(0);
  }

  static void ApplyFilteringDirect(bool enabled, bool trilinearFiltering) {
    const GLint magFilter = enabled ? GL_LINEAR : GL_NEAREST;
    GLint minFilter = GL_NEAREST;

    if (enabled) {
      GLint widthLevel0 = -1;
      GLint widthLevel1 = -1;
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &widthLevel0);
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 1, GL_TEXTURE_WIDTH, &widthLevel1);
      if (widthLevel0 > 1 && widthLevel1 != 0) {
        minFilter = trilinearFiltering ? GL_LINEAR_MIPMAP_LINEAR
                                       : GL_LINEAR_MIPMAP_NEAREST;
      } else {
        minFilter = GL_LINEAR;
      }
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
  }

  static void ApplyWrapping(GLint mode) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
  }

  static inline std::map<uintptr_t, OpenGLTextureObjectState> textureStates_;
  static inline std::mutex textureStatesMutex_;
  std::array<uintptr_t, NUM_TextureUnit> boundTextures_{};
  bool preferenceKnown_ = false;
  bool lastPreferenceValue_ = false;
};

inline OpenGLTextureStateCache& GetOpenGLTextureStateCache() {
  static thread_local OpenGLTextureStateCache cache;
  return cache;
}

#endif
