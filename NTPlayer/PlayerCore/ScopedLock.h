#ifndef _PLAYERCORE_SCOPEDLOCK_H_
#define _PLAYERCORE_SCOPEDLOCK_H_

template <class M>
class ScopedLock
    /// A class that simplifies thread synchronization
    /// with a mutex.
    /// The constructor accepts a Mutex (and optionally
    /// a timeout value in milliseconds) and locks it.
    /// The destructor unlocks the mutex.
{
public:
    explicit ScopedLock(M& mutex): _mutex(mutex)
    {
        _mutex.lock();
    }

    ScopedLock(M& mutex, long milliseconds): _mutex(mutex)
    {
        _mutex.lock(milliseconds);
    }

    ~ScopedLock()
    {
        _mutex.unlock();
    }

private:
    M& _mutex;

    ScopedLock();
    ScopedLock(const ScopedLock&);
    ScopedLock& operator = (const ScopedLock&);
};


#endif