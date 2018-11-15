#pragma once

#include "GenericKit/GenericKit.h"
#include "AppKit/VideoDatas.h"
#include <vector>

namespace AppKit
{
    class VideoDevice
    {
	protected:
		std::vector<VideoDisplay*> Displays;
    public:
        ~VideoDevice();
        
        static VideoDevice* Create();
        
        virtual bool Init();

		int AddVideoDisplay(VideoDisplay* display);
        
	public:
		// Window
		int MakeWindow(const char* title, int x, int y, int w, int h, uint32_t flags);
    protected:
        VideoDevice();
        
        virtual void Quit();
        
        bool InitModes();

		void QuitModes();
    };
}
