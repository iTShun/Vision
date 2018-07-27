#include "Notify.h"
#include "Core/ref_ptr.h"

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <iostream>

#include <ctype.h>

namespace core {

class NullStreamBuffer : public std::streambuf
{
private:
    std::streamsize xsputn(const std::streambuf::char_type * /*str*/, std::streamsize n)
    {
        return n;
    }
};

struct NullStream : public std::ostream
{
public:
    NullStream():
        std::ostream(new NullStreamBuffer)
    { _buffer = dynamic_cast<NullStreamBuffer *>(rdbuf()); }

    ~NullStream()
    {
        rdbuf(0);
        delete _buffer;
    }

protected:
    NullStreamBuffer* _buffer;
};

/** Stream buffer calling notify handler when buffer is synchronized (usually on std::endl).
 * Stream stores last notification severity to pass it to handler call.
 */
struct NotifyStreamBuffer : public std::stringbuf
{
    NotifyStreamBuffer() : _severity(core::kNOTICE)
    {
    }

    void setNotifyHandler(core::NotifyHandler *handler) { _handler = handler; }
    core::NotifyHandler *getNotifyHandler() const { return _handler.get(); }

    /** Sets severity for next call of notify handler */
    void setCurrentSeverity(core::NotifySeverity severity)
    {
        if (_severity != severity)
        {
            sync();
            _severity = severity;
        }
    }

    core::NotifySeverity getCurrentSeverity() const { return _severity; }

private:

    int sync()
    {
        sputc(0); // string termination
        if (_handler.valid())
            _handler->notify(_severity, pbase());
        pubseekpos(0, std::ios_base::out); // or str(std::string())
        return 0;
    }

    core::ref_ptr<core::NotifyHandler> _handler;
    core::NotifySeverity _severity;
};

struct NotifyStream : public std::ostream
{
public:
    NotifyStream():
        std::ostream(new NotifyStreamBuffer)
    { _buffer = dynamic_cast<NotifyStreamBuffer *>(rdbuf()); }

    void setCurrentSeverity(core::NotifySeverity severity)
    {
        _buffer->setCurrentSeverity(severity);
    }

    core::NotifySeverity getCurrentSeverity() const
    {
        return _buffer->getCurrentSeverity();
    }

    ~NotifyStream()
    {
        rdbuf(0);
        delete _buffer;
    }

protected:
    NotifyStreamBuffer* _buffer;
};

}

struct NotifySingleton
{
    NotifySingleton()
    {
        // _notifyLevel
        // =============

        _notifyLevel = core::kNOTICE; // Default value

        // Setup standard notify handler
        core::NotifyStreamBuffer *buffer = dynamic_cast<core::NotifyStreamBuffer *>(_notifyStream.rdbuf());
        if (buffer && !buffer->getNotifyHandler())
            buffer->setNotifyHandler(new core::StandardNotifyHandler);
    }

    core::NotifySeverity _notifyLevel;
    core::NullStream     _nullStream;
    core::NotifyStream   _notifyStream;
};

static NotifySingleton& getNotifySingleton()
{
    static NotifySingleton s_NotifySingleton;
    return s_NotifySingleton;
}

bool core::initNotifyLevel()
{
    getNotifySingleton();
    return true;
}

// Use a proxy to force the initialization of the NotifySingleton during static initialization
INIT_SINGLETON_PROXY(NotifySingletonProxy, core::initNotifyLevel())

void core::setNotifyLevel(core::NotifySeverity severity)
{
    getNotifySingleton()._notifyLevel = severity;
}

core::NotifySeverity core::getNotifyLevel()
{
    return getNotifySingleton()._notifyLevel;
}

void core::setNotifyHandler(core::NotifyHandler *handler)
{
    core::NotifyStreamBuffer *buffer = static_cast<core::NotifyStreamBuffer*>(getNotifySingleton()._notifyStream.rdbuf());
    if (buffer) buffer->setNotifyHandler(handler);
}

core::NotifyHandler* core::getNotifyHandler()
{
    core::NotifyStreamBuffer *buffer = static_cast<core::NotifyStreamBuffer *>(getNotifySingleton()._notifyStream.rdbuf());
    return buffer ? buffer->getNotifyHandler() : 0;
}


#ifndef NOTIFY_DISABLED
bool core::isNotifyEnabled( core::NotifySeverity severity )
{
    return severity<=getNotifySingleton()._notifyLevel;
}
#endif

std::ostream& core::notify(const core::NotifySeverity severity)
{
    if (core::isNotifyEnabled(severity))
    {
        getNotifySingleton()._notifyStream.setCurrentSeverity(severity);
        return getNotifySingleton()._notifyStream;
    }
    return getNotifySingleton()._nullStream;
}

void core::StandardNotifyHandler::notify(core::NotifySeverity severity, const char *message)
{
    if (severity <= core::kWARN)
        fputs(message, stderr);
    else
        fputs(message, stdout);
}

#if defined(WIN32) && !defined(__CYGWIN__)

#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#endif
#   include <windows.h>

void core::WinDebugNotifyHandler::notify(core::NotifySeverity severity, const char *message)
{
    OutputDebugStringA(message);
}

#endif

