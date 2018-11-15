#include "VideoDevice.h"
using namespace GenericKit;

#if PLATFORM_OSX

/* We need this for IODisplayCreateInfoDictionary and kIODisplayOnlyPreferredName */
#import <IOKit/graphics/IOGraphicsLib.h>

/* We need this for CVDisplayLinkGetNominalOutputVideoRefreshPeriod */
#import <CoreVideo/CVBase.h>
#import <CoreVideo/CVDisplayLink.h>

/* we need this for ShowMenuBar() and HideMenuBar(). */
#import <Carbon/Carbon.h>

/* This gets us MAC_OS_X_VERSION_MIN_REQUIRED... */
#import <AvailabilityMacros.h>

static void CG_SetError(const char *prefix, CGDisplayErr result);
static bool GetDisplayMode(CGDisplayModeRef vidmode, CVDisplayLinkRef link, AppKit::DisplayMode *mode);
static const char* Cocoa_GetDisplayName(CGDirectDisplayID displayID);

namespace AppKit
{
    struct DisplayData
    {
        CGDirectDisplayID display;
    };
    
    struct DisplayModeData
    {
        CGDisplayModeRef moderef;
    };
    
    VideoDevice::VideoDevice()
    {
        
    }
    
    VideoDevice::~VideoDevice()
    {
        Quit();
    }
    
    VideoDevice* VideoDevice::Create()
    {
        VideoDevice* ret = new (std::nothrow)VideoDevice();
        if (ret && ret->Init())
        {
            
        }
        else
        {
            delete ret;
            ret = nullptr;
        }
        
        return ret;
    }
    
    bool VideoDevice::Init()
    {
        do {
            if (!InitModes()) break;
            
            
            
            
            return true;
        } while (false);
        
        return false;
    }
    
    void VideoDevice::Quit()
    {
        
    }
    
    bool VideoDevice::InitModes()
    { @autoreleasepool {
        do {
            CGDisplayErr result;
            CGDirectDisplayID *displays;
            CGDisplayCount numDisplays;
            int pass, i;
            
            result = CGGetOnlineDisplayList(0, NULL, &numDisplays);
            if (result != kCGErrorSuccess) {
                CG_SetError("CGGetOnlineDisplayList()", result);
                break;
            }
            displays = new CGDirectDisplayID[numDisplays];
            result = CGGetOnlineDisplayList(numDisplays, displays, &numDisplays);
            if (result != kCGErrorSuccess) {
                CG_SetError("CGGetOnlineDisplayList()", result);
                delete[] displays;
                break;
            }
            
            /* Pick up the primary display in the first pass, then get the rest */
            for (pass = 0; pass < 2; ++pass) {
                for (i = 0; i < numDisplays; ++i) {
                    VideoDisplay display;
                    DisplayData *displaydata = nullptr;
                    DisplayMode mode;
                    CGDisplayModeRef moderef = NULL;
                    CVDisplayLinkRef link = NULL;

                    if (pass == 0) {
                        if (!CGDisplayIsMain(displays[i])) {
                            continue;
                        }
                    } else {
                        if (CGDisplayIsMain(displays[i])) {
                            continue;
                        }
                    }

                    if (CGDisplayMirrorsDisplay(displays[i]) != kCGNullDirectDisplay) {
                        continue;
                    }

                    moderef = CGDisplayCopyDisplayMode(displays[i]);

                    if (!moderef) {
                        continue;
                    }

                    displaydata = new DisplayData();
                    if (!displaydata) {
                        CGDisplayModeRelease(moderef);
                        continue;
                    }
                    displaydata->display = displays[i];

                    CVDisplayLinkCreateWithCGDisplay(displays[i], &link);

                    MemSet(&display, 0, sizeof(display));
                    /* this returns a stddup'ed string */
                    display.Name = (char *)Cocoa_GetDisplayName(displays[i]);
                    if (!GetDisplayMode(moderef, link, &mode)) {
                        CVDisplayLinkRelease(link);
                        CGDisplayModeRelease(moderef);
                        delete[] display.Name;
                        delete displaydata;
                        continue;
                    }

                    CVDisplayLinkRelease(link);

                    display.DesktopMode = mode;
                    display.CurrentMode = mode;
                    display.DriverData = displaydata;
//                    SDL_AddVideoDisplay(&display);
                    delete[] display.Name;
                }
            }
            delete[] displays;
            
            return true;
        } while (false);
        
        return false;
    }}
}

void CG_SetError(const char *prefix, CGDisplayErr result)
{
    const char *error;
    
    switch (result) {
        case kCGErrorFailure:
            error = "kCGErrorFailure";
            break;
        case kCGErrorIllegalArgument:
            error = "kCGErrorIllegalArgument";
            break;
        case kCGErrorInvalidConnection:
            error = "kCGErrorInvalidConnection";
            break;
        case kCGErrorInvalidContext:
            error = "kCGErrorInvalidContext";
            break;
        case kCGErrorCannotComplete:
            error = "kCGErrorCannotComplete";
            break;
        case kCGErrorNotImplemented:
            error = "kCGErrorNotImplemented";
            break;
        case kCGErrorRangeCheck:
            error = "kCGErrorRangeCheck";
            break;
        case kCGErrorTypeCheck:
            error = "kCGErrorTypeCheck";
            break;
        case kCGErrorInvalidOperation:
            error = "kCGErrorInvalidOperation";
            break;
        case kCGErrorNoneAvailable:
            error = "kCGErrorNoneAvailable";
            break;
        default:
            error = "Unknown Error";
            break;
    }
    
    DebugPrintf("%s: %s", prefix, error);
}

#define SDL_DEFINE_PIXELFORMAT(type, order, layout, bits, bytes) \
((1 << 28) | ((type) << 24) | ((order) << 20) | ((layout) << 16) | \
((bits) << 8) | ((bytes) << 0))

bool GetDisplayMode(CGDisplayModeRef vidmode, CVDisplayLinkRef link, AppKit::DisplayMode *mode)
{
    AppKit::DisplayModeData *data = nullptr;
    int width = 0;
    int height = 0;
    int bpp = 0;
    int refreshRate = 0;
    CFStringRef fmt;
    
    data = new AppKit::DisplayModeData();
    if (!data) {
        return false;
    }
    data->moderef = vidmode;
    
    fmt = CGDisplayModeCopyPixelEncoding(vidmode);
    width = (int) CGDisplayModeGetWidth(vidmode);
    height = (int) CGDisplayModeGetHeight(vidmode);
    refreshRate = (int) (CGDisplayModeGetRefreshRate(vidmode) + 0.5);
    
    if (CFStringCompare(fmt, CFSTR(IO32BitDirectPixels),
                        kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
        bpp = 32;
    } else if (CFStringCompare(fmt, CFSTR(IO16BitDirectPixels),
                               kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
        bpp = 16;
    } else if (CFStringCompare(fmt, CFSTR(kIO30BitDirectPixels),
                               kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
        bpp = 30;
    } else {
        bpp = 0;  /* ignore 8-bit and such for now. */
    }
    
    CFRelease(fmt);
    
    /* CGDisplayModeGetRefreshRate returns 0 for many non-CRT displays. */
    if (refreshRate == 0 && link != NULL) {
        CVTime time = CVDisplayLinkGetNominalOutputVideoRefreshPeriod(link);
        if ((time.flags & kCVTimeIsIndefinite) == 0 && time.timeValue != 0) {
            refreshRate = (int) ((time.timeScale / (double) time.timeValue) + 0.5);
        }
    }
    
    mode->Format = 0;//SDL_PIXELFORMAT_UNKNOWN;
    switch (bpp) {
        case 16:
            mode->Format = 16;//SDL_PIXELFORMAT_ARGB1555;
            break;
        case 30:
            mode->Format = 30;//SDL_PIXELFORMAT_ARGB2101010;
            break;
        case 32:
            mode->Format = 32;//SDL_PIXELFORMAT_ARGB8888;
            break;
        case 8: /* We don't support palettized modes now */
        default: /* Totally unrecognizable bit depth. */
            delete data;
            data = nullptr;
            return false;
    }
    mode->Width = width;
    mode->Height = height;
    mode->RefreshRate = refreshRate;
    mode->DriverData = data;
    return true;
}

const char* Cocoa_GetDisplayName(CGDirectDisplayID displayID)
{
    CFDictionaryRef deviceInfo = IODisplayCreateInfoDictionary(CGDisplayIOServicePort(displayID), kIODisplayOnlyPreferredName);
    NSDictionary *localizedNames = [(NSDictionary *)deviceInfo objectForKey:[NSString stringWithUTF8String:kDisplayProductName]];
    const char* displayName = NULL;
    
    if ([localizedNames count] > 0) {
        const char* str = [[localizedNames objectForKey:[[localizedNames allKeys] objectAtIndex:0]] UTF8String];
        int len = StrLen(str) + 1;
        displayName = new char[len];
        MemCopy((void*)displayName, str, len);
    }
    CFRelease(deviceInfo);
    return displayName;
}

#endif // PLATFORM_OSX
