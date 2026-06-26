#ifndef CACHING_OPENGL_STATE_CACHE_H
#define CACHING_OPENGL_STATE_CACHE_H

#include "Caching/ActiveTextureUnitState.h"
#include "Caching/AlphaTestState.h"
#include "Caching/BlendModeState.h"
#include "Caching/BufferBindingState.h"
#include "Caching/CachedGLState.h"
#include "Caching/ClearColorState.h"
#include "Caching/ClientActiveTextureState.h"
#include "Caching/ClientState.h"
#include "Caching/CullModeState.h"
#include "Caching/FrontFaceState.h"
#include "Caching/LightingState.h"
#include "Caching/MatrixModeState.h"
#include "Caching/OpenGLStateCachePreference.h"
#include "Caching/ProgramState.h"
#include "Caching/TextureGenState.h"
#include "Caching/TextureModeState.h"
#include "Caching/ViewportState.h"
#include "Caching/ZBiasState.h"
#include "Caching/ZTestState.h"
#include "Caching/ZWriteState.h"

#include <map>
#include <string>

class OpenGLStateCache {
 public:
  void SetActiveTextureUnit(TextureUnit unit) {
    activeTextureUnit_.Set(CachingEnabled(), unit);
  }

  void SetTextureMode(TextureUnit unit, TextureMode mode) {
    const CacheResult result = textureMode_.Set(CachingEnabled(), unit, mode);
    if (result == CacheResult::DoNotCache && mode == TextureMode_Glow) {
      blendMode_.Invalidate();
    }
  }

  void SetBlendMode(BlendMode mode) {
    blendMode_.Set(CachingEnabled(), mode);
  }
  void SetZWrite(bool enabled) {
    zWrite_.Set(CachingEnabled(), enabled);
  }
  void SetZBias(float bias) { zBias_.Set(CachingEnabled(), bias); }
  void SetZTestMode(ZTestMode mode) {
    zTest_.Set(CachingEnabled(), mode);
  }
  void SetLighting(bool enabled) {
    lighting_.Set(CachingEnabled(), enabled);
  }
  void SetCullMode(CullMode mode) {
    cullMode_.Set(CachingEnabled(), mode);
  }
  void SetAlphaTest(bool enabled) {
    alphaTest_.Set(CachingEnabled(), enabled);
  }
  void SetTextureGen(TextureUnit unit, bool enabled) {
    textureGen_.Set(CachingEnabled(), unit, enabled);
  }
  void SetProgram(GLhandleARB program) {
    program_.Set(CachingEnabled(), program);
  }
  void SetMatrixMode(GLenum mode) {
    matrixMode_.Set(CachingEnabled(), mode);
  }
  void SetViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    viewport_.Set(CachingEnabled(), x, y, width, height);
  }
  void SetClearColor(float r, float g, float b, float a) {
    clearColor_.Set(CachingEnabled(), r, g, b, a);
  }
  void SetFrontFace(GLenum mode) {
    frontFace_.Set(CachingEnabled(), mode);
  }
  void SetClientActiveTexture(GLenum unit) {
    clientActiveTexture_.Set(CachingEnabled(), unit);
    if (CachingEnabled()) {
      clientActiveTextureKnown_ = true;
      clientActiveTextureUnit_ = unit;
    } else {
      clientActiveTextureKnown_ = false;
    }
  }
  void SetClientState(GLenum state, bool enabled) {
    clientState_.Set(
        CachingEnabled(), state, ClientTextureUnitIndex(), enabled);
  }
  void BindBuffer(GLenum target, GLuint buffer) {
    bufferBinding_.Set(CachingEnabled(), target, buffer);
  }

  GLint GetUniformLocation(GLhandleARB program, const char* name) {
    if (!CachingEnabled()) {
      return glGetUniformLocationARB(program, name);
    }

    auto& programLocations = uniformLocations_[program];
    const auto found = programLocations.find(name);
    if (found != programLocations.end()) {
      return found->second;
    }

    const GLint location = glGetUniformLocationARB(program, name);
    programLocations[name] = location;
    return location;
  }

  bool IsZWriteEnabled() {
    const bool cachingEnabled = CachingEnabled();
    if (cachingEnabled) {
      if (const ZWriteState* state = zWrite_.Get(0)) {
        return state->enabled;
      }
    }

    const bool enabled = ZWriteState::ReadCurrent();
    zWrite_.Remember(cachingEnabled, ZWriteState{enabled});
    return enabled;
  }

  bool IsZTestEnabled() {
    const bool cachingEnabled = CachingEnabled();
    if (cachingEnabled) {
      if (const ZTestState* state = zTest_.Get(0)) {
        return state->mode != ZTEST_OFF;
      }
    }

    bool enabled = false;
    const std::optional<ZTestMode> mode = ZTestState::ReadCurrent(enabled);
    if (mode) {
      zTest_.Remember(cachingEnabled, ZTestState{*mode});
    } else {
      zTest_.Invalidate();
    }
    return enabled;
  }

  bool IsLightingEnabled() {
    const bool cachingEnabled = CachingEnabled();
    if (cachingEnabled) {
      if (const LightingState* state = lighting_.Get(0)) {
        return state->enabled;
      }
    }

    const bool enabled = LightingState::ReadCurrent();
    lighting_.Remember(cachingEnabled, LightingState{enabled});
    return enabled;
  }

  bool IsTextureGenEnabled(TextureUnit unit) {
    const bool cachingEnabled = CachingEnabled();
    if (cachingEnabled) {
      if (const TextureGenState* state =
              textureGen_.Get(static_cast<std::size_t>(unit))) {
        return state->enabled;
      }
    }

    const bool enabled = TextureGenState::ReadCurrent();
    textureGen_.Remember(cachingEnabled, TextureGenState{unit, enabled});
    return enabled;
  }

  void InvalidateAll() {
    activeTextureUnit_.Invalidate();
    textureMode_.Invalidate();
    blendMode_.Invalidate();
    zWrite_.Invalidate();
    zBias_.Invalidate();
    zTest_.Invalidate();
    lighting_.Invalidate();
    cullMode_.Invalidate();
    alphaTest_.Invalidate();
    textureGen_.Invalidate();
    program_.Invalidate();
    matrixMode_.Invalidate();
    viewport_.Invalidate();
    clearColor_.Invalidate();
    frontFace_.Invalidate();
    clientActiveTexture_.Invalidate();
    clientState_.Invalidate();
    bufferBinding_.Invalidate();
    clientActiveTextureKnown_ = false;
    uniformLocations_.clear();
  }

 private:
  static Preference<bool>& CachePreference() {
    return g_bEnableOpenGLStateCache;
  }

  bool CachingEnabled() {
    const bool enabled = CachePreference().Get();
    if (!preferenceKnown_ || enabled != lastPreferenceValue_) {
      InvalidateAll();
      preferenceKnown_ = true;
      lastPreferenceValue_ = enabled;
    }
    return enabled;
  }

  std::size_t ClientTextureUnitIndex() const {
    if (!clientActiveTextureKnown_) {
      return 0;
    }

    const int index = static_cast<int>(clientActiveTextureUnit_) -
                      static_cast<int>(GL_TEXTURE0_ARB);
    if (index < 0 || index >= NUM_TextureUnit) {
      return 0;
    }
    return static_cast<std::size_t>(index);
  }

  bool preferenceKnown_ = false;
  bool lastPreferenceValue_ = false;
  CachedGLState<ActiveTextureUnitState> activeTextureUnit_;
  CachedGLState<TextureModeState> textureMode_;
  CachedGLState<BlendModeState> blendMode_;
  CachedGLState<ZWriteState> zWrite_;
  CachedGLState<ZBiasState> zBias_;
  CachedGLState<ZTestState> zTest_;
  CachedGLState<LightingState> lighting_;
  CachedGLState<CullModeState> cullMode_;
  CachedGLState<AlphaTestState> alphaTest_;
  CachedGLState<TextureGenState> textureGen_;
  CachedGLState<ProgramState> program_;
  CachedGLState<MatrixModeState> matrixMode_;
  CachedGLState<ViewportState> viewport_;
  CachedGLState<ClearColorState> clearColor_;
  CachedGLState<FrontFaceState> frontFace_;
  CachedGLState<ClientActiveTextureState> clientActiveTexture_;
  CachedGLState<ClientState> clientState_;
  CachedGLState<BufferBindingState> bufferBinding_;
  bool clientActiveTextureKnown_ = false;
  GLenum clientActiveTextureUnit_ = GL_TEXTURE0_ARB;
  std::map<GLhandleARB, std::map<std::string, GLint>> uniformLocations_;
};

inline OpenGLStateCache& GetOpenGLStateCache() {
  static thread_local OpenGLStateCache cache;
  return cache;
}

#endif
