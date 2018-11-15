#include "VideoDevice.h"
using namespace GenericKit;

#if PLATFORM_WINDOWS

#include <Windows.h>

static void AddDisplays(AppKit::VideoDevice* videoDevice);
static BOOL CALLBACK AddDisplaysCallback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
static bool AddDisplay(AppKit::VideoDevice* videoDevice, HMONITOR hMonitor, const MONITORINFOEX *info);
static bool GetDisplayMode(LPCTSTR deviceName, DWORD index, AppKit::DisplayMode * mode);
static void UpdateDisplayMode(LPCTSTR deviceName, DWORD index, AppKit::DisplayMode * mode);

namespace AppKit
{
	struct AddDisplaysData {
		VideoDevice* videoDevice;
		bool wantPrimary;
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

	int VideoDevice::MakeWindow(const char* title, int x, int y, int w, int h, uint32_t flags)
	{
		/* Some platforms can't create zero-sized windows */
		if (w < 1) {
			w = 1;
		}
		if (h < 1) {
			h = 1;
		}

		/* Some platforms blow up if the windows are too large. Raise it later? */
		if ((w > 16384) || (h > 16384)) {
			Log("Window is too large.");
			return -1;
		}




		return 0;
	}
    
    void VideoDevice::Quit()
    {
		QuitModes();
    }
    
    bool VideoDevice::InitModes()
    { 
        do {
			AddDisplays(this);

			if (Displays.size() == 0)
			{
				Log("No displays available");
				break;
			}

            return true;
        } while (false);
        
        return false;
    }

	void VideoDevice::QuitModes()
	{
		std::vector<VideoDisplay*>::iterator iter = Displays.begin();
		VideoDisplay* ret = nullptr;

		while (iter != Displays.end())
		{
			ret = *iter;
			Displays.erase(iter);
			delete ret;
			iter = Displays.begin();
		}
	}
}

void AddDisplays(AppKit::VideoDevice* videoDevice)
{
	AppKit::AddDisplaysData callback_data;
	callback_data.videoDevice = videoDevice;

	callback_data.wantPrimary = true;
	EnumDisplayMonitors(NULL, NULL, AddDisplaysCallback, (LPARAM)&callback_data);

	callback_data.wantPrimary = false;
	EnumDisplayMonitors(NULL, NULL, AddDisplaysCallback, (LPARAM)&callback_data);
}

BOOL CALLBACK
AddDisplaysCallback(HMONITOR hMonitor,
	HDC      hdcMonitor,
	LPRECT   lprcMonitor,
	LPARAM   dwData)
{
	AppKit::AddDisplaysData *data = (AppKit::AddDisplaysData*)dwData;
	MONITORINFOEX info;

	MemSet(&info, 0, sizeof(info));
	info.cbSize = sizeof(info);

	if (GetMonitorInfo(hMonitor, (LPMONITORINFO)&info) != 0) {
		const bool is_primary = ((info.dwFlags & MONITORINFOF_PRIMARY) == MONITORINFOF_PRIMARY);

		if (is_primary == data->wantPrimary) {
			AddDisplay(data->videoDevice, hMonitor, &info);
		}
	}

	// continue enumeration
	return TRUE;
}

bool AddDisplay(AppKit::VideoDevice* videoDevice, HMONITOR hMonitor, const MONITORINFOEX *info)
{
	using namespace AppKit;

	VideoDisplay *display = nullptr;
	DisplayMode mode;
	DISPLAY_DEVICE device;

#ifdef DEBUG_MODES
	Log("Display: %s", info->szDevice);
#endif

	if (!GetDisplayMode(info->szDevice, ENUM_CURRENT_SETTINGS, &mode)) {
		return false;
	}

	display = new VideoDisplay();
	if (!display) {
		return false;
	}
	MemSet(display, 0, sizeof(display));

	MemCopy(display->DeviceName, info->szDevice,
		sizeof(display->DeviceName));
	display->MonitorHandle = hMonitor;

	device.cb = sizeof(device);
	if (EnumDisplayDevices(info->szDevice, 0, &device, 0)) {
		display->Name = device.DeviceString;
	}
	display->DesktopMode = mode;
	display->CurrentMode = mode;
	videoDevice->AddVideoDisplay(display);

	return true;
}

bool GetDisplayMode(LPCTSTR deviceName, DWORD index, AppKit::DisplayMode * mode)
{
	using namespace AppKit;

	DEVMODE devmode;

	devmode.dmSize = sizeof(devmode);
	devmode.dmDriverExtra = 0;
	if (!EnumDisplaySettings(deviceName, index, &devmode)) {
		return false;
	}

	mode->DeviceMode = devmode;

	mode->Format = 0;// SDL_PIXELFORMAT_UNKNOWN;
	mode->Width = mode->DeviceMode.dmPelsWidth;
	mode->Height = mode->DeviceMode.dmPelsHeight;
	mode->RefreshRate = mode->DeviceMode.dmDisplayFrequency;

	/* Fill in the mode information */
	UpdateDisplayMode(deviceName, index, mode);

	return true;
}

void UpdateDisplayMode(LPCTSTR deviceName, DWORD index, AppKit::DisplayMode * mode)
{
	using namespace AppKit;

	DEVMODE devmode = mode->DeviceMode;
	HDC hdc;

	devmode.dmFields =
		(DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY |
			DM_DISPLAYFLAGS);

	if (index == ENUM_CURRENT_SETTINGS
		&& (hdc = CreateDC(deviceName, NULL, NULL, NULL)) != NULL) {
		char bmi_data[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)];
		LPBITMAPINFO bmi;
		HBITMAP hbm;
		int logical_width = GetDeviceCaps(hdc, HORZRES);
		int logical_height = GetDeviceCaps(hdc, VERTRES);

		mode->Width = logical_width;
		mode->Height = logical_height;

		MemSet(&bmi_data, 0, sizeof(bmi_data));
		bmi = (LPBITMAPINFO)bmi_data;
		bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

		hbm = CreateCompatibleBitmap(hdc, 1, 1);
		GetDIBits(hdc, hbm, 0, 1, NULL, bmi, DIB_RGB_COLORS);
		GetDIBits(hdc, hbm, 0, 1, NULL, bmi, DIB_RGB_COLORS);
		DeleteObject(hbm);
		DeleteDC(hdc);
		if (bmi->bmiHeader.biCompression == BI_BITFIELDS) {
			switch (*(uint32_t *)bmi->bmiColors) {
			case 0x00FF0000:
				mode->Format = 0;// SDL_PIXELFORMAT_RGB888;
				break;
			case 0x000000FF:
				mode->Format = 0;//SDL_PIXELFORMAT_BGR888;
				break;
			case 0xF800:
				mode->Format = 0;//SDL_PIXELFORMAT_RGB565;
				break;
			case 0x7C00:
				mode->Format = 0;//SDL_PIXELFORMAT_RGB555;
				break;
			}
		}
		else if (bmi->bmiHeader.biBitCount == 8) {
			mode->Format = 0;//SDL_PIXELFORMAT_INDEX8;
		}
		else if (bmi->bmiHeader.biBitCount == 4) {
			mode->Format = 0;//SDL_PIXELFORMAT_INDEX4LSB;
		}
	}
	else if (mode->Format == 0//SDL_PIXELFORMAT_UNKNOWN
		) {
		/* FIXME: Can we tell what this will be? */
		if ((devmode.dmFields & DM_BITSPERPEL) == DM_BITSPERPEL) {
			switch (devmode.dmBitsPerPel) {
			case 32:
				mode->Format = 0;//SDL_PIXELFORMAT_RGB888;
				break;
			case 24:
				mode->Format = 0;//SDL_PIXELFORMAT_RGB24;
				break;
			case 16:
				mode->Format = 0;//SDL_PIXELFORMAT_RGB565;
				break;
			case 15:
				mode->Format = 0;//SDL_PIXELFORMAT_RGB555;
				break;
			case 8:
				mode->Format = 0;//SDL_PIXELFORMAT_INDEX8;
				break;
			case 4:
				mode->Format = 0;//SDL_PIXELFORMAT_INDEX4LSB;
				break;
			}
		}
	}
}


#endif // PLATFORM_WINDOWS
