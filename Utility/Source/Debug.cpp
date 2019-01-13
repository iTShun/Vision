#include "Debug.h"
#include "FileSystem.h"

#if COMPILER_MSVC
#   include <windows.h>
#   include <iostream>

void logToIDEConsole(const Utility::String& message)
{
    OutputDebugString(message.c_str());
    OutputDebugString("\n");
    
    // Also default output in case we're running without debugger attached
    std::cout << message << std::endl;
}
#else
void logToIDEConsole(const Utility::String& message)
{
    std::cout << message << std::endl;
}
#endif

namespace Utility
{
    Log::~Log()
    {
        clear();
    }
    
    void Log::logMsg(const String& message, UINT32 channel)
    {
        RecursiveLock lock(mMutex);
        
        mUnreadEntries.push(LogEntry(message, channel));
    }
    
    void Log::clear()
    {
        RecursiveLock lock(mMutex);
        
        mEntries.clear();
        
        while (!mUnreadEntries.empty())
            mUnreadEntries.pop();
        
        mHash++;
    }
    
    void Log::clear(UINT32 channel)
    {
        RecursiveLock lock(mMutex);
        
        Vector<LogEntry> newEntries;
        for(auto& entry : mEntries)
        {
            if (entry.getChannel() == channel)
                continue;
            
            newEntries.push_back(entry);
        }
        
        mEntries = newEntries;
        
        Queue<LogEntry> newUnreadEntries;
        while (!mUnreadEntries.empty())
        {
            LogEntry entry = mUnreadEntries.front();
            mUnreadEntries.pop();
            
            if (entry.getChannel() == channel)
                continue;
            
            newUnreadEntries.push(entry);
        }
        
        mUnreadEntries = newUnreadEntries;
        mHash++;
    }
    
    bool Log::getUnreadEntry(LogEntry& entry)
    {
        RecursiveLock lock(mMutex);
        
        if (mUnreadEntries.empty())
            return false;
        
        entry = mUnreadEntries.front();
        mUnreadEntries.pop();
        mEntries.push_back(entry);
        mHash++;
        
        return true;
    }
    
    bool Log::getLastEntry(LogEntry& entry)
    {
        if (mEntries.size() == 0)
            return false;
        
        entry = mEntries.back();
        return true;
    }
    
    Vector<LogEntry> Log::getEntries() const
    {
        RecursiveLock lock(mMutex);
        
        return mEntries;
    }
    
    Vector<LogEntry> Log::getAllEntries() const
    {
        Vector<LogEntry> entries;
        {
            RecursiveLock lock(mMutex);
            
            for (auto& entry : mEntries)
                entries.push_back(entry);
            
            Queue<LogEntry> unreadEntries = mUnreadEntries;
            while (!unreadEntries.empty())
            {
                entries.push_back(unreadEntries.front());
                unreadEntries.pop();
            }
        }
        return entries;
    }
    
    void Debug::logDebug(const String& msg)
    {
        mLog.logMsg(msg, (UINT32)DebugChannel::Debug);
        logToIDEConsole(msg);
    }
    
    void Debug::logWarning(const String& msg)
    {
        mLog.logMsg(msg, (UINT32)DebugChannel::Warning);
        logToIDEConsole(msg);
    }
    
    void Debug::logError(const String& msg)
    {
        mLog.logMsg(msg, (UINT32)DebugChannel::Error);
        logToIDEConsole(msg);
    }
    
    void Debug::log(const String& msg, UINT32 channel)
    {
        mLog.logMsg(msg, channel);
        logToIDEConsole(msg);
    }
    
    void Debug::_triggerCallbacks()
    {
        LogEntry entry;
        while (mLog.getUnreadEntry(entry))
        {
            onLogEntryAdded(entry);
        }
        
        UINT64 hash = mLog.getHash();
        if(mLogHash != hash)
        {
            onLogModified();
            mLogHash = hash;
        }
    }
    
    void Debug::saveLog(const Path& path) const
    {
        static const char* style =
            R"(html {
            font-family: sans-serif;
        }
    
        table
        {
            border-collapse: collapse;
            border-spacing: 0;
            empty-cells: show;
            border: 1px solid #cbcbcb;
            width:100%;
            table-layout:fixed;
        }
    
        table caption
        {
            color: #000;
            font: italic 85%/1 arial, sans-serif;
            padding: 1em 0;
            text-align: center;
        }
    
        table td,
        table th
        {
            border-left: 1px solid #cbcbcb;/*  inner column border */
            border-width: 0 0 0 1px;
            font-size: inherit;
            margin: 0;
            overflow: visible; /*to make ths where the title is really long work*/
            padding: 0.5em 1em; /* cell padding */
        }
    
        table td:first-child,
        table th:first-child
        {
            border-left-width: 0;
        }
    
        table thead
        {
            background-color: #e0e0e0;
            color: #000;
            text-align: left;
            vertical-align: bottom;
        }
    
        table td
        {
            background-color: transparent;
            word-wrap:break-word;
            vertical-align: top;
            color: #7D7D7D;
        }
    
        .debug-row td {
            background-color: #FFFFFF;
        }
    
        .debug-alt-row td {
            background-color: #f2f2f2;
        }
    
        .warn-row td {
            background-color: #ffc016;
            color: #5F5F5F;
        }
    
        .warn-alt-row td {
            background-color: #fdcb41;
            color: #5F5F5F;
        }
    
        .error-row td {
            background-color: #9f1621;
            color: #9F9F9F;
        }
    
        .error-alt-row td {
            background-color: #ae1621;
            color: #9F9F9F;
        }
        )";
    
        static const char* htmlPreStyleHeader =
            R"(<!DOCTYPE html>
            <html lang="en">
            <head>
            <style type="text/css">
            )";
    
        static const char* htmlPostStyleHeader =
            R"(</style>
            <title>LS Engine Log</title>
            </head>
            <body>
            )";
    
        static const char* htmlEntriesTableHeader =
            R"(<table border="1" cellpadding="1" cellspacing="1">
            <thead>
            <tr>
            <th scope="col" style="width:60px">Type</th>
            <th scope="col">Description</th>
            </tr>
            </thead>
            <tbody>
            )";
    
        static const char* htmlFooter =
            R"(   </tbody>
            </table>
            </body>
            </html>)";
    
        StringStream stream;
        stream << htmlPreStyleHeader;
        stream << style;
        stream << htmlPostStyleHeader;
        stream << "<h1>Debug Logger</h1>\n";
    
        stream << "<h2>System information</h2>\n";
    
        // Write header information
//        stream << "<p>LS version: " << LS_VERSION_MAJOR << "." << LS_VERSION_MINOR << "</p>\n";
//    Todo
//        SystemInfo systemInfo = PlatformUtil::getSystemInfo();
//        stream << "<p>OS version: " << systemInfo.osName << " " << (systemInfo.osIs64Bit ? "64-bit" : "32-bit") << "</p>\n";
//        stream << "<p>CPU vendor: " << systemInfo.cpuManufacturer << "</p>\n";
//        stream << "<p>CPU name: " << systemInfo.cpuModel << "</p>\n";
//        stream << "<p>CPU clock speed: " << systemInfo.cpuClockSpeedMhz << "Mhz</p>\n";
//        stream << "<p>CPU core count: " << systemInfo.cpuNumCores << "</p>\n";
//
//        if(systemInfo.gpuInfo.numGPUs == 1)
//        stream << "<p>GPU: " << systemInfo.gpuInfo.names[0] << "</p>\n";
//        else
//        {
//            for(UINT32 i = 0; i < systemInfo.gpuInfo.numGPUs; i++)
//                stream << "<p>GPU #" << i << ": " << systemInfo.gpuInfo.names[i] << "</p>\n";
//        }
    
        // Write log entries
        stream << "<h2>Log entries</h2>\n";
        stream << htmlEntriesTableHeader;
    
        bool alternate = false;
        Vector<LogEntry> entries = mLog.getAllEntries();
        for (auto& entry : entries)
        {
            String channelName;
            if (entry.getChannel() == (UINT32)DebugChannel::Error || entry.getChannel() == (UINT32)DebugChannel::CompilerError)
            {
                if (!alternate)
                    stream << R"(        <tr class="error-row">)" << std::endl;
                else
                    stream << R"(        <tr class="error-alt-row">)" << std::endl;
                
                stream << R"(            <td>Error</td>)" << std::endl;
            }
            else if (entry.getChannel() == (UINT32)DebugChannel::Warning || entry.getChannel() == (UINT32)DebugChannel::CompilerWarning)
            {
                if (!alternate)
                    stream << R"(        <tr class="warn-row">)" << std::endl;
                else
                    stream << R"(        <tr class="warn-alt-row">)" << std::endl;
                
                stream << R"(            <td>Warning</td>)" << std::endl;
            }
            else
            {
                if (!alternate)
                    stream << R"(        <tr class="debug-row">)" << std::endl;
                else
                    stream << R"(        <tr class="debug-alt-row">)" << std::endl;
                
                stream << R"(            <td>Debug</td>)" << std::endl;
            }
            
            String parsedMessage = StringUtil::replaceAll(entry.getMessage(), "\n", "<br>\n");
            
            stream << R"(            <td>)" << parsedMessage << "</td>" << std::endl;
            stream << R"(        </tr>)" << std::endl;
            
            alternate = !alternate;
        }
    
        stream << htmlFooter;
    
        SPtr<DataStream> fileStream = FileSystem::createAndOpenFile(path);
        fileStream->writeString(stream.str());
    }
    
    Debug& gDebug()
    {
        static Debug debug;
        return debug;
    }
}
