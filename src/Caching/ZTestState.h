#ifndef CACHING_Z_TEST_STATE_H
#define CACHING_Z_TEST_STATE_H

#include <cstddef>
#include <optional>

#include <GL/glew.h>

#include "Caching/CacheResult.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "global.h"

struct ZTestState {
  static constexpr std::size_t SlotCount = 1;

  ZTestMode mode;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const ZTestState& other) const { return mode == other.mode; }

  CacheResult Apply() const {
    glEnable(GL_DEPTH_TEST);
    switch (mode) {
      case ZTEST_OFF:
        glDepthFunc(GL_ALWAYS);
        break;
      case ZTEST_WRITE_ON_PASS:
        glDepthFunc(GL_LEQUAL);
        break;
      case ZTEST_WRITE_ON_FAIL:
        glDepthFunc(GL_GREATER);
        break;
      default:
        FAIL_M(ssprintf("Invalid ZTestMode: %i", mode));
    }
    return CacheResult::Cache;
  }

  static std::optional<ZTestMode> ReadCurrent(bool& enabled) {
    GLint function = GL_ALWAYS;
    glGetIntegerv(GL_DEPTH_FUNC, &function);
    enabled = function != GL_ALWAYS;
    switch (function) {
      case GL_ALWAYS:
        return ZTEST_OFF;
      case GL_LEQUAL:
        return ZTEST_WRITE_ON_PASS;
      case GL_GREATER:
        return ZTEST_WRITE_ON_FAIL;
      default:
        return std::nullopt;
    }
  }
};

#endif
