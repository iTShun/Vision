#pragma once

#define PROFILING_ENABLED   1

#if defined(DEBUG) || defined(_DEBUG)
#   define DEBUG_MODE  1
#else
#   define DEBUG_MODE  0
#endif
