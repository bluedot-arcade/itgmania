#ifndef CACHING_CACHED_GL_STATE_H
#define CACHING_CACHED_GL_STATE_H

#include <array>
#include <cstddef>
#include <optional>
#include <utility>

#include "Caching/CacheResult.h"

template<typename State>
class CachedGLState {
 public:
  template<typename... Args>
  CacheResult Set(bool cachingEnabled, Args&&... args) {
    State requested{std::forward<Args>(args)...};
    auto& current = states_[requested.CacheIndex()];

    if (cachingEnabled && current && *current == requested) {
      return CacheResult::Cache;
    }

    const CacheResult result = requested.Apply();
    if (cachingEnabled && result == CacheResult::Cache) {
      current = std::move(requested);
    } else {
      current.reset();
    }
    return result;
  }

  const State* Get(std::size_t index) const {
    const auto& state = states_[index];
    return state ? &*state : nullptr;
  }

  void Remember(bool cachingEnabled, State state) {
    auto& current = states_[state.CacheIndex()];
    if (cachingEnabled) {
      current = std::move(state);
    } else {
      current.reset();
    }
  }

  void Invalidate() {
    for (auto& state : states_) {
      state.reset();
    }
  }

 private:
  std::array<std::optional<State>, State::SlotCount> states_;
};

#endif
