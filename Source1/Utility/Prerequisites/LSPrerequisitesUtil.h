#pragma once

#include <assert.h>

// 0 - No thread support
// 1 - Render system is thread safe (TODO: NOT WORKING and will probably be removed)
// 2 - Thread support but render system can only be accessed from main thread
#define LS_THREAD_SUPPORT 2

#define LS_STATIC_LIB

//#define LS_UTILITY_EXPORTS

#define LS_PROFILING_ENABLED 1

// Config from the build system
//#include <LSFrameworkConfig.h> Todo

// in FrameworkConfig.h
#define LS_VERSION_MAJOR	1
#define LS_VERSION_MINOR	0

// Platform-specific stuff
#include "Prerequisites/LSPlatformDefines.h"

#if COMPILER_MSVC

// TODO - This is not deactivated anywhere, therefore it applies to any file that includes this header.
//      - Right now I don't have an easier way to apply these warnings globally so I'm keeping it this way.

#	if PLATFORM_WINDOWS
#		pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#	endif

// Secure versions aren't multiplatform, so we won't be using them
#if !defined(_CRT_SECURE_NO_WARNINGS)
	#define _CRT_SECURE_NO_WARNINGS
#endif

// disable: "<type> needs to have dll-interface to be used by clients'
// Happens on STL member variables which are not public therefore is ok
#	pragma warning (disable: 4251)

// disable: 'X' Function call with parameters that may be unsafe
#	pragma warning(disable: 4996)

// disable: decorated name length exceeded, name was truncated
// Happens with really long type names. Even fairly standard use
// of std::unordered_map with custom parameters, meaning I can't
// really do much to avoid it. It shouldn't effect execution
// but might cause problems if you compile library
// with one compiler and use it in another.
#	pragma warning(disable: 4503)

// disable: C++ exception handler used, but unwind semantics are not enabled
// We don't care about this as any exception is meant to crash the program.
#	pragma warning(disable: 4530)

#endif

#if DEBUG_MODES
#	define LS_DEBUG_ONLY(x) x
#	define LS_ASSERT(x)		assert(x)
#else
#	define LS_DEBUG_ONLY(x)
#	define LS_ASSERT(x)
#endif

// Script binding defines

/**
 * @page scriptBindingMacro Script binding exports
 *
 * Marks the specific type or a method to be exported to the scripting API. Supports a variety of options which can
 * be specified in the "option:value" format, where multiple options are separated by commas, with no whitespace.
 *
 * Supported options:
 *  - n - Specify a different name for the type in the scripting API (e.g. "n:MyName"). Usable on types and methods.
 *  - v - Specify a different visibility (default is public). Supported values are "public", "internal" and "private".
 *		  Usable on types and methods.
 *  - f - Specify the name of the output file(s) for the script object and its potential wrappers. If not specified
 *		  the name of the type will be used for the file. Usable on types only.
 *	- pl - Specify whether the type is plain or not (default is false). Supported values are "true" or "false". Plain
 *		  types don't have script interop objects generated, instead they are generated in script code as plain data
 *		  types. No methods are exposed, but all data members and constructors are copied. Usable on types only.
 *	- e - Specify that a method is external and is to be appended to some script class. Such methods must be static
 *		  and as the first parameter accept the instance of the class they operate on. Value of this option should be
 *		  the name of the class to attach this method to. Methods with this parameter must also be part of a class
 *		  with this option. Usable on types and methods.
 *	- ec - Similar to "e", but specifies an external constructor. Such method must have a return value that returns
 *		   an instance of the class its registered for. Value of this option should be the name of the class to attach
 *		   this method to. Methods with this parameter must also be part of a class with the "e" option. Usable on methods
 *		   only.
 *	- pr - Specify the method should be exported as a property in script code. Supported values are "getter" or "setter".
 *		  Getter methods must return a single value and accept no parameters, while setter methods must accept one
 *		  parameter and return no values. Usable on methods only.
 *	- ed - Specify that a type should be exported for use in the editor only. Supported values are "true" or "false".
 *		   Usable on types only.
 *  - ex - Excludes an enum or struct member from being generated in script code. Supported values are "true" or "false".
 *		   By default all struct & enum members are exported.
 *  - in - When enabled ensures only the interop C# method is generated, but not a public one. It is instead expected
 *		   the user will manually implement the public method. Supported values are "true" or "false". Default is "false".
 *		   Only supported on methods.
 *  - m  - Specifies the name of the module to place the entry in. This determines the documentation group, and may also
 *		   determine namespace and/or module (e.g. m:Animation to place it in the Animation module). Usable on types.
 */

#if COMPILER_CLANG
	/** @ref scriptBindingMacro */
	#define LS_SCRIPT_EXPORT(...) __attribute__((annotate("se," #__VA_ARGS__)))

	/** 
	 * When applied to a parameter, makes it a variable argument parameter in the scripting interface (if supported
	 * by the scripting language.
	 */
	#define LS_PARAMS __attribute__((annotate("params")))

	/** 
	 * When applied to a parameter or a field of ResourceHandle type, makes that element be exported as a raw resource in
	 * script code.
	 */
	#define LS_NORREF __attribute__((annotate("norref")))
#else
	/** @ref scriptBindingMacro */
	#define LS_SCRIPT_EXPORT(...)

	/** 
	 * When applied to a parameter, makes it a variable argument parameter in the scripting interface (if supported
	 * by the scripting language).
	 */
	#define LS_PARAMS

	/** 
	 * When applied to a parameter or a field of ResourceHandle type, makes that element be exported as a raw resource in
	 * script code.
	 */
	#define LS_NORREF
#endif

// Short-hand names for various built-in types
#include "Prerequisites/LSTypes.h"

#include "Allocators/LSMemoryAllocator.h"

// Common threading functionality
#include "Thread/LSThread.h"

// Commonly used standard headers
#include "Prerequisites/LSStdHeaders.h"

// Forward declarations
#include "Prerequisites/LSFwdDeclUtil.h"

#include "Prerequisites/LSRTTIPrerequisites.h"

#include "String/LSString.h"
#include "General/LSMessageHandlerFwd.h"
#include "General/LSFlags.h"
#include "General/LSUtil.h"
#include "General/LSEvent.h"
#include "General/LSPlatformUtility.h"
#include "General/LSNonCopyable.h"
#include "General/LSSmallVector.h"
#include "FileSystem/LSPath.h"
#include "Error/LSCrashHandler.h"
