#pragma once

#include "GenericKit/Platform.h"

///
#define STRINGIZE(_x) STRINGIZE_(_x)
#define STRINGIZE_(_x) #_x

///
#define CONCATENATE(_x, _y) CONCATENATE_(_x, _y)
#define CONCATENATE_(_x, _y) _x ## _y

///
#define FILE_LINE_LITERAL "" __FILE__ "(" STRINGIZE(__LINE__) "): "

///
#define ALIGN_MASK(_value, _mask) ( ( (_value)+(_mask) ) & ( (~0)&(~(_mask) ) ) )
#define ALIGN_16(_value) ALIGN_MASK(_value, 0xf)
#define ALIGN_256(_value) ALIGN_MASK(_value, 0xff)
#define ALIGN_4096(_value) ALIGN_MASK(_value, 0xfff)

#define ALIGNOF(_type) __alignof(_type)