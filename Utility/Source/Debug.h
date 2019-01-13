#pragma once

#include "Config.h"
#include "Path.h"
#include "Event.h"
#include "Thread.h"

namespace Utility
{
    /** @addtogroup Debug
     *  @{
     */
    
    /** A single log entry, containing a message and a channel the message was recorded on. */
    class UTILITY_EXPORT LogEntry
    {
    public:
        LogEntry() = default;
        LogEntry(String msg, UINT32 channel)
        :mMsg(std::move(msg)), mChannel(channel)
        { }
        
        /** Channel the message was recorded on. */
        UINT32 getChannel() const { return mChannel; }
        
        /** Text of the message. */
        const String& getMessage() const { return mMsg; }
        
    private:
        String mMsg;
        UINT32 mChannel;
    };
    
    /**
     * Used for logging messages. Can categorize messages according to channels, save the log to a file
     * and send out callbacks when a new message is added.
     *
     * @note    Thread safe.
     */
    class UTILITY_EXPORT Log
    {
    public:
        Log() = default;
        ~Log();
        
        /**
         * Logs a new message.
         *
         * @param[in]    message    The message describing the log entry.
         * @param[in]    channel Channel in which to store the log entry.
         */
        void logMsg(const String& message, UINT32 channel);
        
        /** Removes all log entries. */
        void clear();
        
        /** Removes all log entries in a specific channel. */
        void clear(UINT32 channel);
        
        /** Returns all existing log entries. */
        Vector<LogEntry> getEntries() const;
        
        /**
         * Returns the latest unread entry from the log queue, and removes the entry from the unread entries list.
         *
         * @param[out]    entry    Entry that was retrieved, or undefined if no entries exist.
         * @return                True if an unread entry was retrieved, false otherwise.
         */
        bool getUnreadEntry(LogEntry& entry);
        
        /**
         * Returns the last available log entry.
         *
         * @param[out]    entry    Entry that was retrieved, or undefined if no entries exist.
         * @return                True if an entry was retrieved, false otherwise.
         */
        bool getLastEntry(LogEntry& entry);
        
        /**
         * Returns a hash value that is modified whenever entries in the log change. This can be used for
         * checking for changes by external systems.
         */
        UINT64 getHash() const { return mHash; }
        
    private:
        friend class Debug;
        
        /** Returns all log entries, including those marked as unread. */
        Vector<LogEntry> getAllEntries() const;
        
        Vector<LogEntry> mEntries;
        Queue<LogEntry> mUnreadEntries;
        UINT64 mHash = 0;
        mutable RecursiveMutex mMutex;
    };
    
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
    class UTILITY_EXPORT Debug
    {
    public:
        Debug() = default;
        
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
    
    /** A simpler way of accessing the Debug module. */
    UTILITY_EXPORT Debug& gDebug();
    
    /** @} */
}

#ifndef LOGDBG
/** Shortcut for logging a message in the debug channel. */
#   define LOGDBG(x) Utility::gDebug().logDebug((x) + Utility::String("\n\t\t in ") + FUNCTION + " [" + __FILE__ + ":" + Utility::toString(__LINE__) + "]\n");
#endif

#ifndef LOGWRN
/** Shortcut for logging a message in the warning channel. */
#   define LOGWRN(x) Utility::gDebug().logWarning((x) + Utility::String("\n\t\t in ") + FUNCTION + " [" + __FILE__ + ":" + Utility::toString(__LINE__) + "]\n");
#endif

#ifndef LOGERR
/** Shortcut for logging a message in the error channel. */
#   define LOGERR(x) Utility::gDebug().logError((x) + Utility::String("\n\t\t in ") + FUNCTION + " [" + __FILE__ + ":" + Utility::toString(__LINE__) + "]\n");
#endif

