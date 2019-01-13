#pragma once

#include "Macros.h"

#define STATIC_LIB
//#define UTILITY_EXPORTS

// DLL export
#ifdef STATIC_LIB
#    define UTILITY_EXPORT
#else
#    ifdef UTILITY_EXPORTS
#        define UTILITY_EXPORT    SYMBOL_EXPORT
#    else
#        define UTILITY_EXPORT    SYMBOL_IMPORT
#    endif
#endif
