#ifndef ViewerExports_h
#define ViewerExports_h

#include "Viewer/Config.h"

// disable VisualStudio warnings
#if defined(_MSC_VER) && defined(DISABLE_MSVC_WARNINGS)
#   pragma warning( disable : 4244 )
#   pragma warning( disable : 4251 )
#   pragma warning( disable : 4275 )
#   pragma warning( disable : 4512 )
#   pragma warning( disable : 4267 )
#   pragma warning( disable : 4702 )
#   pragma warning( disable : 4511 )
#   pragma warning( disable : 4996 )
#endif


#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#  if defined( VIEWER_LIBRARY_STATIC )
#    define API_VIEWER
#  elif defined( VIEWER_LIBRARY )
#    define API_VIEWER   __declspec(dllexport)
#  else
#    define API_VIEWER   __declspec(dllimport)
#  endif
#else
#  define API_VIEWER
#endif


#endif /* ViewerExports_h */
