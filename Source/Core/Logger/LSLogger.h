#pragma once

#include "String/LSString.h"
#include "Utility/LSEvent.h"
#include "Logger/LSLog.h"

namespace ls
{
    /** @addtogroup Logger
     *  @{
     */
    
    /** Available types of channels that debug messages can be logged to. */
    enum class DebugChannel
    {
        Debug, Warning, Error, CompilerWarning, CompilerError
    };
    
    /**
     * Utility class providing various debug functionality.
     *
     * @note    Thread safe.
     */
    class Logger
    {
    public:
        Logger() = default;
        
        /** Adds a log entry in the "Debug" channel. */
        void logDebug(const String& msg);
        
        /** Adds a log entry in the "Warning" channel. */
        void logWarning(const String& msg);
        
        /** Adds a log entry in the "Error" channel. */
        void logError(const String& msg);
        
        /** Adds a log entry in the specified channel. You may specify custom channels as needed. */
        void log(const String& msg, UINT32 channel);
        
        /** Retrieves the Log used by the Debug instance. */
        Log& getLog() { return mLog; }
        
        /** Converts raw pixels into a BMP image and saves it as a file */
//        void writeAsBMP(UINT8* rawPixels, UINT32 bytesPerPixel, UINT32 width, UINT32 height, const Path& filePath, bool overwrite = true) const; Todo
        
        /**
         * Saves a log about the current state of the application to the specified location.
         *
         * @param    path    Absolute path to the log filename.
         */
        void saveLog(const Path& path) const;
        
        /**
         * Triggered when a new entry in the log is added.
         *
         * @note    Sim thread only.
         */
        Event<void(const LogEntry&)> onLogEntryAdded;
        
        /**
         * Triggered whenever one or multiple log entries were added or removed. Triggers only once per frame.
         *
         * @note    Sim thread only.
         */
        Event<void()> onLogModified;
        
    public: // ***** INTERNAL ******
        /** @name Internal
         *  @{
         */
        
        /**
         * Triggers callbacks that notify external code that a log entry was added.
         *
         * @note    Sim thread only.
         */
        void _triggerCallbacks();
        
        /** @} */
    private:
        UINT64 mLogHash = 0;
        Log mLog;
    };
    
    /** A simpler way of accessing the Logger module. */
    Logger& gLogger();
    
    /** Shortcut for logging a message in the debug channel. */
#define LOGDBG(x) ls::gLogger().logDebug((x) + ls::String("\n\t\t in ") + FUNCTION + " [" + __FILE__ + ":" + ls::toString(__LINE__) + "]\n");
    
    /** Shortcut for logging a message in the warning channel. */
#define LOGWRN(x) ls::gLogger().logWarning((x) + ls::String("\n\t\t in ") + FUNCTION + " [" + __FILE__ + ":" + ls::toString(__LINE__) + "]\n");
    
    /** Shortcut for logging a message in the error channel. */
#define LOGERR(x) ls::gLogger().logError((x) + ls::String("\n\t\t in ") + FUNCTION + " [" + __FILE__ + ":" + ls::toString(__LINE__) + "]\n");
    
    /** @} */
}
