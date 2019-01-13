#pragma once

#include "Platform.h"

#if defined(_DEBUG) || defined(DEBUG)
#	define DEBUG_MODE 1
#else
#	define DEBUG_MODE 0
#endif

#if PLATFORM_WINDOWS
#   define SYMBOL_EXPORT __declspec(dllexport)
#   define SYMBOL_IMPORT __declspec(dllimport)
#else
#   define SYMBOL_EXPORT __attribute__((visibility("default")))
#   define SYMBOL_IMPORT
#endif // PLATFORM_WINDOWS

///
#define STRINGIZE(_x) STRINGIZE_(_x)
#define STRINGIZE_(_x) #_x

///
#define CONCATENATE(_x, _y) CONCATENATE_(_x, _y)
#define CONCATENATE_(_x, _y) _x ## _y

///
#define FILE_LINE_LITERAL "" __FILE__ "(" STRINGIZE(__LINE__) "): "

#define ALIGNOF(_type) __alignof(_type)

#if defined(__has_feature)
#    define CLANG_HAS_FEATURE(_x) __has_feature(_x)
#else
#    define CLANG_HAS_FEATURE(_x) 0
#endif // defined(__has_feature)

#if defined(__has_extension)
#    define CLANG_HAS_EXTENSION(_x) __has_extension(_x)
#else
#    define CLANG_HAS_EXTENSION(_x) 0
#endif // defined(__has_extension)

#if COMPILER_GCC || COMPILER_CLANG
#    define ALIGN_DECL(_align, _decl) _decl __attribute__( (aligned(_align) ) )
#    define ALLOW_UNUSED __attribute__( (unused) )
#    define FORCE_INLINE inline __attribute__( (__always_inline__) )
#    define FUNCTION __PRETTY_FUNCTION__
#    define LIKELY(_x)   __builtin_expect(!!(_x), 1)
#    define UNLIKELY(_x) __builtin_expect(!!(_x), 0)
#    define NO_INLINE   __attribute__( (noinline) )
#    define NO_RETURN   __attribute__( (noreturn) )
#    define CONST_FUNC  __attribute__( (const) )

#    if COMPILER_GCC >= 70000
#        define FALLTHROUGH __attribute__( (fallthrough) )
#    else
#        define FALLTHROUGH NOOP()
#    endif // COMPILER_GCC >= 70000

#    define NO_VTABLE
#    define PRINTF_ARGS(_format, _args) __attribute__( (format(__printf__, _format, _args) ) )

#    if CLANG_HAS_FEATURE(cxx_thread_local)
#        define THREAD_LOCAL __thread
#    endif // COMPILER_CLANG

#    if (!PLATFORM_OSX && (COMPILER_GCC >= 40200)) || (COMPILER_GCC >= 40500)
#        define THREAD_LOCAL __thread
#    endif // COMPILER_GCC

#    define ATTRIBUTE(_x) __attribute__( (_x) )

#    if CRT_MSVC
#        define __stdcall
#    endif // CRT_MSVC
#elif COMPILER_MSVC
#    define ALIGN_DECL(_align, _decl) __declspec(align(_align) ) _decl
#    define ALLOW_UNUSED
#    define FORCE_INLINE __forceinline
#    define FUNCTION __FUNCTION__
#    define LIKELY(_x)   (_x)
#    define UNLIKELY(_x) (_x)
#    define NO_INLINE __declspec(noinline)
#    define NO_RETURN
#    define CONST_FUNC  __declspec(noalias)
#    define FALLTHROUGH NOOP()
#    define NO_VTABLE __declspec(novtable)
#    define PRINTF_ARGS(_format, _args)
#    define THREAD_LOCAL __declspec(thread)
#    define ATTRIBUTE(_x)
#else
#    error "Unknown COMPILER_?"
#endif

///
#define MACRO_BLOCK_BEGIN for(;;) {
#define MACRO_BLOCK_END break; }
#define NOOP(...) MACRO_BLOCK_BEGIN MACRO_BLOCK_END


