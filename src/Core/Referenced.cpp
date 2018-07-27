#include "Referenced.h"
#include "Core/Notify.h"
#include "Core/Observer.h"

#include <typeinfo>
#include <memory>
#include <set>

namespace core {
    
// specialized smart pointer, used to get round auto_ptr<>'s lack of the destructor resetting itself to 0.
template<typename T>
struct ResetPointer
{
    ResetPointer():
    _ptr(0) {}
    
    ResetPointer(T* ptr):
    _ptr(ptr) {}
    
    ~ResetPointer()
    {
        delete _ptr;
        _ptr = 0;
    }
    
    inline ResetPointer& operator = (T* ptr)
    {
        if (_ptr==ptr) return *this;
        delete _ptr;
        _ptr = ptr;
        return *this;
    }
    
    void reset(T* ptr)
    {
        if (_ptr==ptr) return;
        delete _ptr;
        _ptr = ptr;
    }
    
    inline T& operator*()  { return *_ptr; }
    
    inline const T& operator*() const { return *_ptr; }
    
    inline T* operator->() { return _ptr; }
    
    inline const T* operator->() const   { return _ptr; }
    
    T* get() { return _ptr; }
    
    const T* get() const { return _ptr; }
    
    T* _ptr;
};

typedef ResetPointer<DeleteHandler> DeleteHandlerPointer;
typedef ResetPointer<OpenThreads::Mutex> GlobalMutexPointer;

OpenThreads::Mutex* Referenced::getGlobalReferencedMutex()
{
    static GlobalMutexPointer s_ReferencedGlobalMutext = new OpenThreads::Mutex;
    return s_ReferencedGlobalMutext.get();
}
    
// helper class for forcing the global mutex to be constructed when the library is loaded.
struct InitGlobalMutexes
{
    InitGlobalMutexes()
    {
        Referenced::getGlobalReferencedMutex();
    }
};
static InitGlobalMutexes s_initGlobalMutexes;
    
void Referenced::setDeleteHandler(DeleteHandler* handler)
{
    //s_deleteHandler.reset(handler);
}

DeleteHandler* Referenced::getDeleteHandler()
{
    return NULL;//s_deleteHandler.get();
}

Referenced::Referenced():
#if defined(_REFERENCED_USE_ATOMIC_OPERATIONS)
    _observerSet(0),
    _refCount(0)
#else
    _refMutex(0),
    _refCount(0),
    _observerSet(0)
#endif
{
#if !defined(_REFERENCED_USE_ATOMIC_OPERATIONS)
        _refMutex = new OpenThreads::Mutex;
#endif
}

Referenced::Referenced(bool threadSafeRefUnref):
#if defined(_REFERENCED_USE_ATOMIC_OPERATIONS)
    _observerSet(0),
    _refCount(0)
#else
    _refMutex(0),
    _refCount(0),
    _observerSet(0)
#endif
{
#if !defined(_REFERENCED_USE_ATOMIC_OPERATIONS)
    if (threadSafeRefUnref)
        _refMutex = new OpenThreads::Mutex;
#endif
}

Referenced::Referenced(const Referenced&):
#if defined(_REFERENCED_USE_ATOMIC_OPERATIONS)
    _observerSet(0),
    _refCount(0)
#else
    _refMutex(0),
    _refCount(0),
    _observerSet(0)
#endif
{
#if !defined(_REFERENCED_USE_ATOMIC_OPERATIONS)
        _refMutex = new OpenThreads::Mutex;
#endif
}

Referenced::~Referenced()
{
    if (_refCount>0)
    {
        LOG_WARN<<"Warning: deleting still referenced object "<<this<<" of type '"<<typeid(this).name()<<"'"<<std::endl;
        LOG_WARN<<"         the final reference count was "<<_refCount<<", memory corruption possible."<<std::endl;
    }
    
    // signal observers that we are being deleted.
    signalObserversAndDelete(true, false);
    
    // delete the ObserverSet
#if defined(_REFERENCED_USE_ATOMIC_OPERATIONS)
    if (_observerSet.get()) static_cast<ObserverSet*>(_observerSet.get())->unref();
#else
    if (_observerSet) static_cast<ObserverSet*>(_observerSet)->unref();
#endif
    
#if !defined(_REFERENCED_USE_ATOMIC_OPERATIONS)
    if (_refMutex) delete _refMutex;
#endif
}

ObserverSet* Referenced::getOrCreateObserverSet() const
{
#if defined(_REFERENCED_USE_ATOMIC_OPERATIONS)
    ObserverSet* observerSet = static_cast<ObserverSet*>(_observerSet.get());
    while (0 == observerSet)
    {
        ObserverSet* newObserverSet = new ObserverSet(this);
        newObserverSet->ref();
        
        if (!_observerSet.assign(newObserverSet, 0))
        {
            newObserverSet->unref();
        }
        
        observerSet = static_cast<ObserverSet*>(_observerSet.get());
    }
    return observerSet;
#else
    if (_refMutex)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*_refMutex);
        if (!_observerSet)
        {
            _observerSet = new ObserverSet(this);
            static_cast<ObserverSet*>(_observerSet)->ref();
        }
        return static_cast<ObserverSet*>(_observerSet);
    }
    else
    {
        if (!_observerSet)
        {
            _observerSet = new ObserverSet(this);
            static_cast<ObserverSet*>(_observerSet)->ref();
        }
        return static_cast<ObserverSet*>(_observerSet);
    }
#endif
}

void Referenced::addObserver(Observer* observer) const
{
    getOrCreateObserverSet()->addObserver(observer);
}

void Referenced::removeObserver(Observer* observer) const
{
    getOrCreateObserverSet()->removeObserver(observer);
}

void Referenced::signalObserversAndDelete(bool signalDelete, bool doDelete) const
{
#if defined(_REFERENCED_USE_ATOMIC_OPERATIONS)
    ObserverSet* observerSet = static_cast<ObserverSet*>(_observerSet.get());
#else
    ObserverSet* observerSet = static_cast<ObserverSet*>(_observerSet);
#endif
    
    if (observerSet && signalDelete)
    {
        observerSet->signalObjectDeleted(const_cast<Referenced*>(this));
    }
    
    if (doDelete)
    {
        if (_refCount!=0)
            LOG_NOTICE<<"Warning Referenced::signalObserversAndDelete(,,) doing delete with _refCount="<<_refCount<<std::endl;
        
        if (getDeleteHandler()) deleteUsingDeleteHandler();
        else delete this;
    }
}

void Referenced::setThreadSafeRefUnref(bool threadSafe)
{
#if !defined(_REFERENCED_USE_ATOMIC_OPERATIONS)
    if (threadSafe)
    {
        if (!_refMutex)
        {
            // we want thread safe ref()/unref() so assign a mutex
            _refMutex = new OpenThreads::Mutex;
        }
    }
    else
    {
        if (_refMutex)
        {
            // we don't want thread safe ref()/unref() so remove any assigned mutex
            OpenThreads::Mutex* tmpMutexPtr = _refMutex;
            _refMutex = 0;
            delete tmpMutexPtr;
        }
    }
#endif
}

int Referenced::unref_nodelete() const
{
#if defined(_REFERENCED_USE_ATOMIC_OPERATIONS)
    return --_refCount;
#else
    if (_refMutex)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*_refMutex);
        return --_refCount;
    }
    else
    {
        return --_refCount;
    }
#endif
}

void Referenced::deleteUsingDeleteHandler() const
{
//    getDeleteHandler()->requestDelete(this);
}
    
}
