#ifndef _PLAYERCORE_MUTEXIMPL_H_
#define _PLAYERCORE_MUTEXIMPL_H_

#include <windows.h>

class MutexImpl
{
protected:
    MutexImpl();
    ~MutexImpl();
    void lockImpl();
    bool tryLockImpl();
    //bool tryLockImpl(long milliseconds);
    void unlockImpl();

private:
    CRITICAL_SECTION cs_;
};


typedef MutexImpl FastMutexImpl;


//
// inlines
//
inline void MutexImpl::lockImpl()
{
    try
    {
        EnterCriticalSection(&cs_);
    }
    catch (...)
    {
        //throw SystemException("cannot lock mutex");
    }
}


inline bool MutexImpl::tryLockImpl()
{
    try
    {
        return TryEnterCriticalSection(&cs_) != 0;
    }
    catch (...)
    {
        //throw SystemException("cannot lock mutex");
    }
}


inline void MutexImpl::unlockImpl()
{
    LeaveCriticalSection(&cs_);
}

#endif