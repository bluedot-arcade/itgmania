#ifndef CACHING_BLEND_MODE_STATE_H
#define CACHING_BLEND_MODE_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "global.h"

struct BlendModeState {
  static constexpr std::size_t SlotCount = 1;

  BlendMode mode;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const BlendModeState& other) const {
    return mode == other.mode;
  }

  CacheResult Apply() const {
    glEnable(GL_BLEND);

    if (glBlendEquation != nullptr) {
      if (mode == BLEND_INVERT_DEST) {
        glBlendEquation(GL_FUNC_SUBTRACT);
      } else if (mode == BLEND_SUBTRACT) {
        glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
      } else {
        glBlendEquation(GL_FUNC_ADD);
      }
    }

    int sourceRGB;
    int destRGB;
    int sourceAlpha = GL_ONE;
    int destAlpha = GL_ONE_MINUS_SRC_ALPHA;
    switch (mode) {
      case BLEND_NORMAL:
        sourceRGB = GL_SRC_ALPHA;
        destRGB = GL_ONE_MINUS_SRC_ALPHA;
        break;
      case BLEND_ADD:
        sourceRGB = GL_SRC_ALPHA;
        destRGB = GL_ONE;
        break;
      case BLEND_SUBTRACT:
        sourceRGB = GL_SRC_ALPHA;
        destRGB = GL_ONE_MINUS_SRC_ALPHA;
        break;
      case BLEND_MODULATE:
        sourceRGB = GL_ZERO;
        destRGB = GL_SRC_COLOR;
        break;
      case BLEND_COPY_SRC:
        sourceRGB = GL_ONE;
        destRGB = GL_ZERO;
        sourceAlpha = GL_ONE;
        destAlpha = GL_ZERO;
        break;
      case BLEND_ALPHA_MASK:
        sourceRGB = GL_ZERO;
        destRGB = GL_ONE;
        sourceAlpha = GL_ZERO;
        destAlpha = GL_SRC_ALPHA;
        break;
      case BLEND_ALPHA_KNOCK_OUT:
        sourceRGB = GL_ZERO;
        destRGB = GL_ONE;
        sourceAlpha = GL_ZERO;
        destAlpha = GL_ONE_MINUS_SRC_ALPHA;
        break;
      case BLEND_ALPHA_MULTIPLY:
        sourceRGB = GL_SRC_ALPHA;
        destRGB = GL_ZERO;
        break;
      case BLEND_WEIGHTED_MULTIPLY:
        sourceRGB = GL_DST_COLOR;
        destRGB = GL_SRC_COLOR;
        break;
      case BLEND_INVERT_DEST:
        sourceRGB = GL_ONE;
        destRGB = GL_ONE;
        break;
      case BLEND_NO_EFFECT:
        sourceRGB = GL_ZERO;
        destRGB = GL_ONE;
        sourceAlpha = GL_ZERO;
        destAlpha = GL_ONE;
        break;
      default:
        FAIL_M(ssprintf("Invalid BlendMode: %i", mode));
    }

    if (GLEW_EXT_blend_equation_separate) {
      glBlendFuncSeparateEXT(sourceRGB, destRGB, sourceAlpha, destAlpha);
    } else {
      glBlendFunc(sourceRGB, destRGB);
    }
    return CacheResult::Cache;
  }
};

#endif
