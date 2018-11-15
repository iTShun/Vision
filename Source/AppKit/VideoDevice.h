#pragma once

#include "GenericKit/GenericKit.h"

namespace AppKit
{
    class VideoDevice;
    struct DisplayData;
    struct DisplayModeData;
    
    struct DisplayMode
    {
        int32_t Format;                 /**< pixel format */
        int Width;                      /**< width, in screen coordinates */
        int Height;                     /**< height, in screen coordinates */
        int RefreshRate;                /**< refresh rate (or zero for unspecified) */
        DisplayModeData *DriverData;    /**< driver-specific data, initialize to 0 */
    };
    
    struct VideoDisplay
    {
        char *Name;
        int MaxDisplaymodes;
        int NumDisplayModes;
        DisplayMode *DisplayModes;
        DisplayMode DesktopMode;
        DisplayMode CurrentMode;
        
//        SDL_Window *fullscreen_window;
        
        VideoDevice *Device;
        
        DisplayData *DriverData;
    };
    
    class VideoDevice
    {
    public:
        ~VideoDevice();
        
        static VideoDevice* Create();
        
        virtual bool Init();
        
    protected:
        VideoDevice();
        
        virtual void Quit();
        
        bool InitModes();
    };
}
