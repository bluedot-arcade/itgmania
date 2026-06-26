#ifndef CACHING_PROGRAM_STATE_H
#define CACHING_PROGRAM_STATE_H

#include <cstddef>

#include <GL/glew.h>

#include "Caching/CacheResult.h"

struct ProgramState {
  static constexpr std::size_t SlotCount = 1;

  GLhandleARB program;

  std::size_t CacheIndex() const { return 0; }
  bool operator==(const ProgramState& other) const {
    return program == other.program;
  }

  CacheResult Apply() const {
    glUseProgramObjectARB(program);
    return CacheResult::Cache;
  }
};

#endif
