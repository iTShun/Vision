#include "Window/LSVideoMode.h"

#if PLATFORM_OSX

#import <ApplicationServices/ApplicationServices.h>
#import <AppKit/AppKit.h>
#include "Logger/LSLogger.h"
#include <algorithm>

namespace ls
{
    ////////////////////////////////////////////////////////////
    size_t modeBitsPerPixel(CGDisplayModeRef mode)
    {
        size_t bpp = 0; // no match
        
        // Compare encoding.
        CFStringRef pixEnc = CGDisplayModeCopyPixelEncoding(mode);
        if(CFStringCompare(pixEnc, CFSTR(IO32BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
            bpp = 32;
        else if(CFStringCompare(pixEnc, CFSTR(IO16BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
            bpp = 16;
        else if(CFStringCompare(pixEnc, CFSTR(IO8BitIndexedPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
            bpp = 8;
        
        // Clean up memory.
        CFRelease(pixEnc);
        
        return bpp;
    }
    
    ////////////////////////////////////////////////////////////
    size_t displayBitsPerPixel(CGDirectDisplayID displayId)
    {
        // Get the display mode.
        CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displayId);
        
        // Get bpp for the mode.
        const size_t bpp = modeBitsPerPixel(mode);
        
        // Clean up Memory.
        CGDisplayModeRelease(mode);
        
        return bpp;
    }
    
    ////////////////////////////////////////////////////////////
    VideoMode convertCGModeToSFMode(CGDisplayModeRef cgmode)
    {
        // The main documentation says the sizes returned by
        // CGDisplayModeGetWidth and CGDisplayModeGetHeight
        // are expressed in pixels. However, some additional
        // documentation [1] states they actually return
        // values in points starting with 10.8.
        //
        // We therefore needs to use the scaling factor to
        // convert the dimensions properly.
        //
        // [1]: "APIs for Supporting High Resolution" > "Additions and Changes for OS X v10.8"
        // https://developer.apple.com/library/mac/documentation/GraphicsAnimation/Conceptual/HighResolutionOSX/APIs/APIs.html#//apple_ref/doc/uid/TP40012302-CH5-SW27
        VideoMode mode(CGDisplayModeGetWidth(cgmode), CGDisplayModeGetHeight(cgmode), modeBitsPerPixel(cgmode));
        CGFloat fcator = [[NSScreen mainScreen] backingScaleFactor];
        mode.width *= fcator;
        mode.height *= fcator;
        return mode;
    }
    
    ////////////////////////////////////////////////////////////
    VideoMode VideoMode::getDesktopMode()
    {
        VideoMode mode; // RVO
        
        // Rely exclusively on mode and convertCGModeToSFMode
        // instead of display id and CGDisplayPixelsHigh/Wide.
        
        CGDirectDisplayID display = CGMainDisplayID();
        CGDisplayModeRef cgmode = CGDisplayCopyDisplayMode(display);
        
        mode = convertCGModeToSFMode(cgmode);
        
        CGDisplayModeRelease(cgmode);
        
        return mode;
    }
    
    ////////////////////////////////////////////////////////////
    const Vector<VideoMode>& VideoMode::getFullscreenModes()
    {
        static Vector<VideoMode> modes;
        
        // Populate the array on first call
        if (modes.empty())
        {
            // Retrieve all modes available for main screen only.
            CFArrayRef cgmodes = CGDisplayCopyAllDisplayModes(CGMainDisplayID(), NULL);
            
            if (cgmodes == NULL)
            {
                LOGERR("Couldn't get VideoMode for main display.");
                return modes;
            }
            
            VideoMode desktop = getDesktopMode();
            
            // Loop on each mode and convert it into a sf::VideoMode object.
            const CFIndex modesCount = CFArrayGetCount(cgmodes);
            for (CFIndex i = 0; i < modesCount; i++)
            {
                CGDisplayModeRef cgmode = (CGDisplayModeRef)CFArrayGetValueAtIndex(cgmodes, i);
                
                VideoMode mode = convertCGModeToSFMode(cgmode);
                
                // Skip if bigger than desktop as we currently don't perform hard resolution switch
                if ((mode.width > desktop.width) || (mode.height > desktop.height))
                    continue;
                
                // If not yet listed we add it to our modes array.
                if (std::find(modes.begin(), modes.end(), mode) == modes.end())
                    modes.push_back(mode);
            }
            
            // Clean up memory.
            CFRelease(cgmodes);
            std::sort(modes.begin(), modes.end(), std::greater<VideoMode>());
        }
        
        return modes;
    }
}

#endif // PLATFORM_OSX
