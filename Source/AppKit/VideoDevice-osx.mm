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
static void ToggleMenuBar(const BOOL show);
static bool GetDisplayMode(CGDisplayModeRef vidmode, CVDisplayLinkRef link, AppKit::DisplayMode *mode);
static const char* GetDisplayName(CGDirectDisplayID displayID);
static int SetDisplayMode(AppKit::VideoDisplay* display, AppKit::DisplayMode* mode);

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

	int VideoDevice::AddVideoDisplay(VideoDisplay* display)
	{
		if (display)
			Displays.push_back(display);

		return Displays.size() - 1;
	}
    
    void VideoDevice::Quit()
    {
        QuitModes();
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
                    VideoDisplay *display = nullptr;
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

					display = new VideoDisplay();
                    if (!display) {
						CGDisplayModeRelease(moderef);
                        continue;
                    }
                    MemSet(display, 0, sizeof(display));

					display->Display = displays[i];

                    CVDisplayLinkCreateWithCGDisplay(displays[i], &link);

                    /* this returns a stddup'ed string */
                    display->Name = (char *)GetDisplayName(displays[i]);
                    if (!GetDisplayMode(moderef, link, &mode)) {
                        CVDisplayLinkRelease(link);
                        CGDisplayModeRelease(moderef);
						delete display;
                        continue;
                    }

                    CVDisplayLinkRelease(link);

                    display->DesktopMode = mode;
                    display->CurrentMode = mode;
					AddVideoDisplay(display);
                }
            }
            delete[] displays;
            
            return true;
        } while (false);
        
        return false;
    }}
    
    void VideoDevice::QuitModes()
    {
        std::vector<VideoDisplay*>::iterator iter = Displays.begin();
        VideoDisplay* display = nullptr;
        
        while (iter != Displays.end())
        {
            display = *iter;
            Displays.erase(iter);
            
            if (display->CurrentMode.ModeRef != display->DesktopMode.ModeRef)
            {
                SetDisplayMode(display, &display->DesktopMode);
            }
            
            CGDisplayModeRelease(display->DesktopMode.ModeRef);
            
            delete display;
            iter = Displays.begin();
        }
    }
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
    
    Log("%s: %s", prefix, error);
}

void ToggleMenuBar(const BOOL show)
{
    /* !!! FIXME: keep an eye on this.
     * ShowMenuBar/HideMenuBar is officially unavailable for 64-bit binaries.
     *  It happens to work, as of 10.7, but we're going to see if
     *  we can just simply do without it on newer OSes...
     */
#if (MAC_OS_X_VERSION_MIN_REQUIRED < 1070) && !defined(__LP64__)
    if (show) {
        ShowMenuBar();
    } else {
        HideMenuBar();
    }
#endif
}

#define SDL_DEFINE_PIXELFORMAT(type, order, layout, bits, bytes) \
((1 << 28) | ((type) << 24) | ((order) << 20) | ((layout) << 16) | \
((bits) << 8) | ((bytes) << 0))

bool GetDisplayMode(CGDisplayModeRef vidmode, CVDisplayLinkRef link, AppKit::DisplayMode *mode)
{
    int width = 0;
    int height = 0;
    int bpp = 0;
    int refreshRate = 0;
    CFStringRef fmt;
    
	mode->ModeRef = vidmode;
    
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
            return false;
    }
    mode->Width = width;
    mode->Height = height;
    mode->RefreshRate = refreshRate;
    return true;
}

const char* GetDisplayName(CGDirectDisplayID displayID)
{
    CFDictionaryRef deviceInfo = IODisplayCreateInfoDictionary(CGDisplayIOServicePort(displayID), kIODisplayOnlyPreferredName);
    NSDictionary *localizedNames = [(NSDictionary *)deviceInfo objectForKey:[NSString stringWithUTF8String:kDisplayProductName]];
    const char* displayName = NULL;
    
    if ([localizedNames count] > 0) {
		displayName = [[localizedNames objectForKey:[[localizedNames allKeys] objectAtIndex:0]] UTF8String];
    }
    CFRelease(deviceInfo);
    return displayName;
}

int SetDisplayMode(AppKit::VideoDisplay* display, AppKit::DisplayMode* mode)
{
    CGDisplayFadeReservationToken fade_token = kCGDisplayFadeReservationInvalidToken;
    CGError result;
    
    /* Fade to black to hide resolution-switching flicker */
    if (CGAcquireDisplayFadeReservation(5, &fade_token) == kCGErrorSuccess) {
        CGDisplayFade(fade_token, 0.3, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0.0, 0.0, 0.0, TRUE);
    }
    
    if (mode->ModeRef == display->DesktopMode.ModeRef) {
        /* Restoring desktop mode */
        CGDisplaySetDisplayMode(display->Display, mode->ModeRef, NULL);
        
        if (CGDisplayIsMain(display->Display)) {
            CGReleaseAllDisplays();
        } else {
            CGDisplayRelease(display->Display);
        }
        
        if (CGDisplayIsMain(display->Display)) {
            ToggleMenuBar(YES);
        }
    } else {
        /* Put up the blanking window (a window above all other windows) */
        if (CGDisplayIsMain(display->Display)) {
            /* If we don't capture all displays, Cocoa tries to rearrange windows... *sigh* */
            result = CGCaptureAllDisplays();
        } else {
            result = CGDisplayCapture(display->Display);
        }
        if (result != kCGErrorSuccess) {
            CG_SetError("CGDisplayCapture()", result);
            goto ERR_NO_CAPTURE;
        }
        
        /* Do the physical switch */
        result = CGDisplaySetDisplayMode(display->Display, mode->ModeRef, NULL);
        if (result != kCGErrorSuccess) {
            CG_SetError("CGDisplaySwitchToMode()", result);
            goto ERR_NO_SWITCH;
        }
        
        /* Hide the menu bar so it doesn't intercept events */
        if (CGDisplayIsMain(display->Display)) {
            ToggleMenuBar(NO);
        }
    }
    
    /* Fade in again (asynchronously) */
    if (fade_token != kCGDisplayFadeReservationInvalidToken) {
        CGDisplayFade(fade_token, 0.5, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal, 0.0, 0.0, 0.0, FALSE);
        CGReleaseDisplayFadeReservation(fade_token);
    }
    
    return 0;
    
    /* Since the blanking window covers *all* windows (even force quit) correct recovery is crucial */
ERR_NO_SWITCH:
    CGDisplayRelease(display->Display);
ERR_NO_CAPTURE:
    if (fade_token != kCGDisplayFadeReservationInvalidToken) {
        CGDisplayFade (fade_token, 0.5, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal, 0.0, 0.0, 0.0, FALSE);
        CGReleaseDisplayFadeReservation(fade_token);
    }
    return -1;
}

#endif // PLATFORM_OSX
