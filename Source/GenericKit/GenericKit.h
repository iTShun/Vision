#pragma once

#include "GenericKit/Platform.h"
#include "GenericKit/Macros.h"
#include "GenericKit/General.h"
#include "GenericKit/Debug.h"
#include "GenericKit/Allocator.h"
#include "GenericKit/DefaultAllocator.h"
#include "GenericKit/StringUtils.h"
#include "GenericKit/String.h"

#define Log(format, ...)  GenericKit::DebugPrintf(format "\n", ## __VA_ARGS__)