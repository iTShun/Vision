#pragma once

// Architecture
#define ARCH_32BIT 0
#define ARCH_64BIT 0

// Compiler
#define COMPILER_CLANG           0
#define COMPILER_CLANG_ANALYZER  0
#define COMPILER_GCC             0
#define COMPILER_MSVC            0

// Endianess
#define CPU_ENDIAN_BIG    0
#define CPU_ENDIAN_LITTLE 0

// CPU
#define CPU_ARM   0
#define CPU_JIT   0
#define CPU_MIPS  0
#define CPU_PPC   0
#define CPU_RISCV 0
#define CPU_X86   0

// C Runtime
#define CRT_BIONIC 0
#define CRT_GLIBC  0
#define CRT_LIBCXX 0
#define CRT_MINGW  0
#define CRT_MSVC   0
#define CRT_NEWLIB 0

#ifndef CRT_MUSL
#	define CRT_MUSL 0
#endif // CRT_MUSL

#ifndef CRT_NONE
#	define CRT_NONE 0
#endif // CRT_NONE

// Platform
#define PLATFORM_ANDROID    0
#define PLATFORM_BSD        0
#define PLATFORM_EMSCRIPTEN 0
#define PLATFORM_HURD       0
#define PLATFORM_IOS        0
#define PLATFORM_LINUX      0
#define PLATFORM_NX         0
#define PLATFORM_OSX        0
#define PLATFORM_PS4        0
#define PLATFORM_RPI        0
#define PLATFORM_STEAMLINK  0
#define PLATFORM_WINDOWS    0
#define PLATFORM_WINRT      0
#define PLATFORM_XBOXONE    0


// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Compilers
#if defined(__clang__)
// clang defines __GNUC__ or _MSC_VER
#	undef  COMPILER_CLANG
#	define COMPILER_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#	if defined(__clang_analyzer__)
#		undef  COMPILER_CLANG_ANALYZER
#		define COMPILER_CLANG_ANALYZER 1
#	endif // defined(__clang_analyzer__)
#elif defined(_MSC_VER)
#	undef  COMPILER_MSVC
#	define COMPILER_MSVC _MSC_VER
#elif defined(__GNUC__)
#	undef  COMPILER_GCC
#	define COMPILER_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#	error "COMPILER_* is not defined!"
#endif //

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Architectures
#if defined(__arm__)     || \
	defined(__aarch64__) || \
	defined(_M_ARM)
#	undef  CPU_ARM
#	define CPU_ARM 1
#	define CACHE_LINE_SIZE 64
#elif defined(__MIPSEL__)     || \
	  defined(__mips_isa_rev) || \
	  defined(__mips64)
#	undef  CPU_MIPS
#	define CPU_MIPS 1
#	define CACHE_LINE_SIZE 64
#elif defined(_M_PPC)        || \
	  defined(__powerpc__)   || \
	  defined(__powerpc64__)
#	undef  CPU_PPC
#	define CPU_PPC 1
#	define CACHE_LINE_SIZE 128
#elif defined(__riscv)   || \
	  defined(__riscv__) || \
	  defined(RISCVEL)
#	undef  CPU_RISCV
#	define CPU_RISCV 1
#	define CACHE_LINE_SIZE 64
#elif defined(_M_IX86)    || \
	  defined(_M_X64)     || \
	  defined(__i386__)   || \
	  defined(__x86_64__)
#	undef  CPU_X86
#	define CPU_X86 1
#	define CACHE_LINE_SIZE 64
#else // PNaCl doesn't have CPU defined.
#	undef  CPU_JIT
#	define CPU_JIT 1
#	define CACHE_LINE_SIZE 64
#endif //

#if defined(__x86_64__)    || \
	defined(_M_X64)        || \
	defined(__aarch64__)   || \
	defined(__64BIT__)     || \
	defined(__mips64)      || \
	defined(__powerpc64__) || \
	defined(__ppc64__)     || \
	defined(__LP64__)
#	undef  ARCH_64BIT
#	define ARCH_64BIT 64
#else
#	undef  ARCH_32BIT
#	define ARCH_32BIT 32
#endif //

#if CPU_PPC
#	undef  CPU_ENDIAN_BIG
#	define CPU_ENDIAN_BIG 1
#else
#	undef  CPU_ENDIAN_LITTLE
#	define CPU_ENDIAN_LITTLE 1
#endif // PLATFORM_


// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Operating_Systems
#if defined(_DURANGO) || defined(_XBOX_ONE)
#	undef  PLATFORM_XBOXONE
#	define PLATFORM_XBOXONE 1
#elif defined(_WIN32) || defined(_WIN64)
// http://msdn.microsoft.com/en-us/library/6sehtctf.aspx
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif // NOMINMAX
//  If _USING_V110_SDK71_ is defined it means we are using the v110_xp or v120_xp toolset.
#	if defined(_MSC_VER) && (_MSC_VER >= 1700) && (!_USING_V110_SDK71_)
#		include <winapifamily.h>
#	endif // defined(_MSC_VER) && (_MSC_VER >= 1700) && (!_USING_V110_SDK71_)
#	if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#		undef  PLATFORM_WINDOWS
#		if !defined(WINVER) && !defined(_WIN32_WINNT)
#			if ARCH_64BIT
//				When building 64-bit target Win7 and above.
#				define WINVER 0x0601
#				define _WIN32_WINNT 0x0601
#			else
//				Windows Server 2003 with SP1, Windows XP with SP2 and above
#				define WINVER 0x0502
#				define _WIN32_WINNT 0x0502
#			endif // ARCH_64BIT
#		endif // !defined(WINVER) && !defined(_WIN32_WINNT)
#		define PLATFORM_WINDOWS _WIN32_WINNT
#	else
#		undef  PLATFORM_WINRT
#		define PLATFORM_WINRT 1
#	endif
#elif defined(__ANDROID__)
// Android compiler defines __linux__
#	include <sys/cdefs.h> // Defines __BIONIC__ and includes android/api-level.h
#	undef  PLATFORM_ANDROID
#	define PLATFORM_ANDROID __ANDROID_API__
#elif defined(__STEAMLINK__)
// SteamLink compiler defines __linux__
#	undef  PLATFORM_STEAMLINK
#	define PLATFORM_STEAMLINK 1
#elif defined(__VCCOREVER__)
// RaspberryPi compiler defines __linux__
#	undef  PLATFORM_RPI
#	define PLATFORM_RPI 1
#elif  defined(__linux__)
#	undef  PLATFORM_LINUX
#	define PLATFORM_LINUX 1
#elif  defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) \
	|| defined(__ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__)
#	undef  PLATFORM_IOS
#	define PLATFORM_IOS 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#	undef  PLATFORM_OSX
#	define PLATFORM_OSX __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#elif defined(__EMSCRIPTEN__)
#	undef  PLATFORM_EMSCRIPTEN
#	define PLATFORM_EMSCRIPTEN 1
#elif defined(__ORBIS__)
#	undef  PLATFORM_PS4
#	define PLATFORM_PS4 1
#elif  defined(__FreeBSD__)        \
	|| defined(__FreeBSD_kernel__) \
	|| defined(__NetBSD__)         \
	|| defined(__OpenBSD__)        \
	|| defined(__DragonFly__)
#	undef  PLATFORM_BSD
#	define PLATFORM_BSD 1
#elif defined(__GNU__)
#	undef  PLATFORM_HURD
#	define PLATFORM_HURD 1
#elif defined(__NX__)
#	undef PLATFORM_NX
#	define PLATFORM_NX 1
#endif //


#if !CRT_NONE
// https://sourceforge.net/p/predef/wiki/Libraries/
#	if defined(__BIONIC__)
#		undef  CRT_BIONIC
#		define CRT_BIONIC 1
#	elif defined(_MSC_VER)
#		undef  CRT_MSVC
#		define CRT_MSVC 1
#	elif defined(__GLIBC__)
#		undef  CRT_GLIBC
#		define CRT_GLIBC (__GLIBC__ * 10000 + __GLIBC_MINOR__ * 100)
#	elif defined(__MINGW32__) || defined(__MINGW64__)
#		undef  CRT_MINGW
#		define CRT_MINGW 1
#	elif defined(__apple_build_version__) || defined(__ORBIS__) || defined(__EMSCRIPTEN__) || defined(__llvm__)
#		undef  CRT_LIBCXX
#		define CRT_LIBCXX 1
#	endif //

#	if !CRT_BIONIC \
	&& !CRT_GLIBC  \
	&& !CRT_LIBCXX \
	&& !CRT_MINGW  \
	&& !CRT_MSVC   \
	&& !CRT_MUSL   \
	&& !CRT_NEWLIB
#		undef  CRT_NONE
#		define CRT_NONE 1
#	endif // CRT_*
#endif // !CRT_NONE

#define PLATFORM_POSIX (0      \
		|| PLATFORM_ANDROID    \
		|| PLATFORM_BSD        \
		|| PLATFORM_EMSCRIPTEN \
		|| PLATFORM_HURD       \
		|| PLATFORM_IOS        \
		|| PLATFORM_LINUX      \
		|| PLATFORM_NX         \
		|| PLATFORM_OSX        \
		|| PLATFORM_PS4        \
		|| PLATFORM_RPI        \
		|| PLATFORM_STEAMLINK  \
		)

#define PLATFORM_NONE !(0      \
		|| PLATFORM_ANDROID    \
		|| PLATFORM_BSD        \
		|| PLATFORM_EMSCRIPTEN \
		|| PLATFORM_HURD       \
		|| PLATFORM_IOS        \
		|| PLATFORM_LINUX      \
		|| PLATFORM_NX         \
		|| PLATFORM_OSX        \
		|| PLATFORM_PS4        \
		|| PLATFORM_RPI        \
		|| PLATFORM_STEAMLINK  \
		|| PLATFORM_WINDOWS    \
		|| PLATFORM_WINRT      \
		|| PLATFORM_XBOXONE    \
		)