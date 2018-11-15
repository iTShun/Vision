#pragma once

#include "GenericKit/GenericKit.h"

#if COMPILER_MSVC
#   pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#endif

namespace AppKit
{
    class VideoDevice;
    
    class Application
    {
    protected:
        VideoDevice* Device;
        bool ExitApp;
    public:
        Application();

        virtual ~Application();

        int32_t Run(int argc = NULL, const char* const* argv = NULL);

    protected:
        bool RegisterApp();
        
        void DestroyVideoDevice();
        
    };
}

/* SDL2 流程
 SDL_Init(SDL_INIT_VIDEO)
 {
    SDL_VideoInit(NULL)
    {
        OS_CreateDevice()
        {
            OS_RegisterApp();
 
            创建 SDL_VideoDevice 对象
 
            创建系统 SDL_VideoData 对象，并赋值给 SDL_VideoDevice->driverdata
 
            赋值系统对应方法函数
 
            返回 SDL_VideoDevice 对象
        }
 
        为全局 SDL_VideoDevice(_this) 对象赋值
 
        OS_VideoInit(_this)
        {
            平台相关初始化
        }
    }
 }
 
 SDL_CreateWindow("Test",
                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                 640, 480, 0)
 {
    判断当前可使用的图形API
 
    图形API相关初始化
 
    创建 SDL_Window 对象
 
    SDL_Window 对象相关初始化
 }
 
 SDL_PollEvent(&event)
 {
    SDL_PumpEvents()
 
    SDL_PeepEvents()
 }
 
 SDL_Quit()
 {
    SDL_VideoQuit()
 }
 */
