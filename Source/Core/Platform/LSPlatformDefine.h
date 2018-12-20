#pragma once

#define LS_VERSION_MAJOR    1
#define LS_VERSION_MINOR    1

#define PROFILING_ENABLED   1

#if defined(DEBUG) || defined(_DEBUG)
#   define DEBUG_MODE  1
#else
#   define DEBUG_MODE  0
#endif
