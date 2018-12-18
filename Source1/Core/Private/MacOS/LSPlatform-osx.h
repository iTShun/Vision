#pragma once

#include "Platform/LSPlatform.h"

#if PLATFORM_OSX

#include <Cocoa/Cocoa.h>

namespace ls
{
    /** @addtogroup Platform-Internal
     *  @{
     */
    
    /** Contains various macOS specific platform functionality. */
    class LS_CORE_EXPORT OSXPlatform : public Platform
    {
        
    };
}

#endif // PLATFORM_OSX
