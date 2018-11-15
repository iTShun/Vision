#pragma once

#include "GenericKit/General.h"

namespace GenericKit
{
	///
	void DebugBreak();

	///
	void DebugOutput(const char* out);

	///
	void DebugPrintfVargs(const char* format, va_list argList);

	///
	void DebugPrintf(const char* format, ...);

	///
	void DebugPrintfData(const void* data, uint32_t size, const char* format, ...);

}
