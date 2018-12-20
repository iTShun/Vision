#include "Platform/LSPlatformUtil.h"

#if PLATFORM_WINDOWS

#include <windows.h>

namespace ls
{
    GPUInfo PlatformUtil::sGPUInfo;
    
    void PlatformUtil::terminate(bool force)
    {
        if (!force)
            PostQuitMessage(0);
        else
            TerminateProcess(GetCurrentProcess(), 0);
    }
    
    SystemInfo PlatformUtil::getSystemInfo()
    {
        SystemInfo output;
        
        INT32 CPUInfo[4] = { -1 };
        
        // Get CPU manufacturer
        __cpuid(CPUInfo, 0);
        output.cpuManufacturer = String(12, ' ');
        memcpy((char*)output.cpuManufacturer.data(), &CPUInfo[1], 4);
        memcpy((char*)output.cpuManufacturer.data() + 4, &CPUInfo[3], 4);
        memcpy((char*)output.cpuManufacturer.data() + 8, &CPUInfo[2], 4);
        
        // Get CPU brand string
        char brandString[48];
        
        //// Get the information associated with each extended ID.
        __cpuid(CPUInfo, 0x80000000);
        UINT32 numExtensionIds = CPUInfo[0];
        for (UINT32 i = 0x80000000; i <= numExtensionIds; ++i)
        {
            __cpuid(CPUInfo, i);
            
            if (i == 0x80000002)
                memcpy(brandString, CPUInfo, sizeof(CPUInfo));
            else if (i == 0x80000003)
                memcpy(brandString + 16, CPUInfo, sizeof(CPUInfo));
            else if (i == 0x80000004)
                memcpy(brandString + 32, CPUInfo, sizeof(CPUInfo));
        }
        
        output.cpuModel = brandString;
        
        // Get number of CPU cores
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        output.cpuNumCores = (UINT32)sysInfo.dwNumberOfProcessors;
        
        // Get CPU clock speed
        HKEY hKey;
        
        long status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ,
                                   &hKey);
        
        if (status == ERROR_SUCCESS)
        {
            DWORD mhz;
            DWORD bufferSize = 4;
            RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE) &mhz, &bufferSize);
            
            output.cpuClockSpeedMhz = (UINT32)mhz;
        }
        else
            output.cpuClockSpeedMhz = 0;
        
        // Get amount of system memory
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        GlobalMemoryStatusEx(&statex);
        
        output.memoryAmountMb = (UINT32)(statex.ullTotalPhys / (1024 * 1024));
        
        if (BS_ARCH_TYPE == BS_ARCHITECTURE_x86_64)
            output.osIs64Bit = true;
        else
        {
            HANDLE process = GetCurrentProcess();
            BOOL is64Bit = false;
            IsWow64Process(process, (PBOOL)&is64Bit);
            
            output.osIs64Bit = is64Bit > 0;
        }
        
        // Get OS version
        output.osName = "Windows " + toString((UINT32)GetRealOSVersion().dwMajorVersion);
        
        // Get GPU info
        output.gpuInfo = sGPUInfo;
        
        return output;
    }
}

#endif // PLATFORM_WINDOWS
