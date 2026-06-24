#ifndef CACHING_OPENGL_STATE_CACHE_H
#define CACHING_OPENGL_STATE_CACHE_H

#include "Caching/ActiveTextureUnitState.h"
#include "Caching/AlphaTestState.h"
#include "Caching/BlendModeState.h"
#include "Caching/CachedGLState.h"
#include "Caching/CullModeState.h"
#include "Caching/LightingState.h"
#include "Caching/TextureModeState.h"
#include "Caching/ZBiasState.h"
#include "Caching/ZTestState.h"
#include "Caching/ZWriteState.h"
#include "Preference.h"

inline Preference<bool> g_bEnableOpenGLStateCache(
    "EnableOpenGLStateCache", false);

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
};

inline OpenGLStateCache& GetOpenGLStateCache() {
  static thread_local OpenGLStateCache cache;
  return cache;
}

#endif
