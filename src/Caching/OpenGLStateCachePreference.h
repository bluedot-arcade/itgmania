#ifndef CACHING_OPENGL_STATE_CACHE_PREFERENCE_H
#define CACHING_OPENGL_STATE_CACHE_PREFERENCE_H

#include "Preference.h"

inline Preference<bool> g_bEnableOpenGLStateCache(
    "EnableOpenGLStateCache", false);

#endif
