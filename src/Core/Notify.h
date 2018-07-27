#ifndef Notify_h
#define Notify_h

#include "Core/Exports.h"
#include "Core/Referenced.h" // for NotifyHandler

#include <ostream>

namespace core {
    
/** Range of notify levels from kDEBUG_FP through to kFATAL, kALWAYS
 * is reserved for forcing the absorption of all messages.
 * See documentation on core::notify() for further details.
 */
enum NotifySeverity {
    kALWAYS=0,
    kFATAL=1,
    kWARN=2,
    kNOTICE=3,
    kINFO=4,
    kDEBUG_INFO=5,
    kDEBUG_FP=6
};

/** set the notify level. */
extern API_CORE void setNotifyLevel(NotifySeverity severity);

/** get the notify level. */
extern API_CORE NotifySeverity getNotifyLevel();

/** initialize notify level. */
extern API_CORE bool initNotifyLevel();

#ifdef NOTIFY_DISABLED
    inline bool isNotifyEnabled(NotifySeverity) { return false; }
#else
    /** is notification enabled, given the current setNotifyLevel() setting? */
    extern API_CORE bool isNotifyEnabled(NotifySeverity severity);
#endif

/** notify messaging function for providing fatal through to verbose
 * debugging messages.
 *
 * All tell the osg to redirect all debugging and more important messages
 * to the notification stream (useful for debugging) setting kALWAYS will force
 * all messages to be absorbed, which might be appropriate for final
 * applications.  Default NotifyLevel is kNOTICE.  Check the enum
 * #NotifySeverity for full range of possibilities.  To use the notify
 * with your code simply use the notify function as a normal file
 * stream (like std::cout) i.e
 * @code
 * core::notify(core::kDEBUG) << "Hello Bugs!" << std::endl;
 * @endcode
 * @see setNotifyLevel, setNotifyHandler
 */
extern API_CORE std::ostream& notify(const NotifySeverity severity);

inline std::ostream& notify(void) { return notify(core::kINFO); }

#define LOG_NOTIFY(level)   if (core::isNotifyEnabled(level)) core::notify(level)
#define LOG_ALWAYS          LOG_NOTIFY(core::kALWAYS)
#define LOG_FATAL           LOG_NOTIFY(core::kFATAL)
#define LOG_WARN            LOG_NOTIFY(core::kWARN)
#define LOG_NOTICE          LOG_NOTIFY(core::kNOTICE)
#define LOG_INFO            LOG_NOTIFY(core::kINFO)
#define LOG_DEBUG           LOG_NOTIFY(core::kDEBUG_INFO)
#define LOG_DEBUG_FP        LOG_NOTIFY(core::kDEBUG_FP)

/** Handler processing output of notification stream. It acts as a sink to
 * notification messages. It is called when notification stream needs to be
 * synchronized (i.e. after core::notify() << std::endl).
 * StandardNotifyHandler is used by default, it writes notifications to stderr
 * (severity <= kWARN) or stdout (severity > kWARN).
 * Notifications can be redirected to other sinks such as GUI widgets or
 * windows debugger (WinDebugNotifyHandler) with custom handlers.
 * Use setNotifyHandler to set custom handler.
 * Note that osg notification API is not thread safe although notification
 * handler is called from many threads. When incorporating handlers into GUI
 * widgets you must take care of thread safety on your own.
 * @see setNotifyHandler
 */
class API_CORE NotifyHandler : public core::Referenced
{
public:
    virtual void notify(core::NotifySeverity severity, const char *message) = 0;
};

/** Set notification handler, by default StandardNotifyHandler is used.
 * @see NotifyHandler
 */
extern API_CORE void setNotifyHandler(NotifyHandler *handler);

/** Get currrent notification handler. */
extern API_CORE NotifyHandler *getNotifyHandler();

/** Redirects notification stream to stderr (severity <= WARN) or stdout (severity > WARN).
 * The fputs() function is used to write messages to standard files. Note that
 * std::out and std::cerr streams are not used.
 * @see setNotifyHandler
 */
class API_CORE StandardNotifyHandler : public NotifyHandler
{
public:
    void notify(core::NotifySeverity severity, const char *message);
};

#if defined(WIN32) && !defined(__CYGWIN__)

/** Redirects notification stream to windows debugger with use of
 * OuputDebugString functions.
 * @see setNotifyHandler
 */
class API_CORE WinDebugNotifyHandler : public NotifyHandler
{
public:
    void notify(core::NotifySeverity severity, const char *message);
};
    
#endif
    
}

#endif /* Notify_h */
