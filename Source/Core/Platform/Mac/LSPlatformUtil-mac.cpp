#include "Platform/LSPlatformUtil.h"

#if PLATFORM_OSX

#include <sys/sysctl.h>

namespace ls
{
    GPUInfo PlatformUtil::sGPUInfo;
    
    void PlatformUtil::terminate(bool force)
    {
        // TODOPORT - Support clean exit by sending the main window a quit message
        exit(0);
    }
    
    SystemInfo PlatformUtil::getSystemInfo()
    {
        char buffer[256];
        
        SystemInfo sysInfo;
        
        size_t bufferlen = sizeof(buffer);
        if(sysctlbyname("machdep.cpu.vendor", buffer, &bufferlen, nullptr, 0) == 0)
            sysInfo.cpuManufacturer = buffer;
        
        bufferlen = sizeof(buffer);
        if(sysctlbyname("machdep.cpu.brand_string", buffer, &bufferlen, nullptr, 0) == 0)
            sysInfo.cpuModel = buffer;
        
        bufferlen = sizeof(buffer);
        if(sysctlbyname("kern.osrelease", buffer, &bufferlen, nullptr, 0) == 0)
            sysInfo.osName = "macOS " + String(buffer);
        
        bufferlen = sizeof(buffer);
        if(sysctlbyname("kern.version", buffer, &bufferlen, nullptr, 0) == 0)
            sysInfo.osIs64Bit = strstr(buffer, "X86_64") != nullptr;
        else
            sysInfo.osIs64Bit = false;
        
        bufferlen = sizeof(buffer);
        if(sysctlbyname("hw.cpufrequency", buffer, &bufferlen, nullptr, 0) == 0)
        {
            UINT32 speedHz = *(UINT32*)buffer;
            sysInfo.cpuClockSpeedMhz = speedHz / (1000 * 1000);
        }
        else
            sysInfo.cpuClockSpeedMhz = 0;
        
        bufferlen = sizeof(buffer);
        if(sysctlbyname("hw.physicalcpu", buffer, &bufferlen, nullptr, 0) == 0)
            sysInfo.cpuNumCores = *(UINT32*)buffer;
        else
            sysInfo.cpuNumCores = 0;
        
        bufferlen = sizeof(buffer);
        if(sysctlbyname("hw.memsize", buffer, &bufferlen, nullptr, 0) == 0)
        {
            UINT64 memAmountBytes = *(UINT64*)buffer;
            sysInfo.memoryAmountMb = (UINT32)(memAmountBytes / (1024 * 1024));
        }
        else
            sysInfo.memoryAmountMb = 0;
        
        sysInfo.gpuInfo = sGPUInfo;
        return sysInfo;
    }
}

#endif // PLATFORM_WINDOWS

