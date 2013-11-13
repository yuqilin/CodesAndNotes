#include "stdafx.h"
#include "MutexImpl.h"

static const DWORD kDefaultSpinCount = 4000;

MutexImpl::MutexImpl()
{
    // the fct has a boolean return value under WInnNt/2000/XP but not on Win98
    // the return only checks if the input address of &_cs was valid, so it is safe to omit it
    InitializeCriticalSectionAndSpinCount(&cs_, kDefaultSpinCount);
}


MutexImpl::~MutexImpl()
{
    DeleteCriticalSection(&cs_);
}


// bool MutexImpl::tryLockImpl(long milliseconds)
// {
//     const int sleepMillis = 5;
//     Timestamp now;
//     Timestamp::TimeDiff diff(Timestamp::TimeDiff(milliseconds)*1000);
//     do
//     {
//         try
//         {
//             if (TryEnterCriticalSection(&_cs) == TRUE)
//                 return true;
//         }
//         catch (...)
//         {
//             //throw SystemException("cannot lock mutex");
//         }
//         Sleep(sleepMillis);
//     }
//     while (!now.isElapsed(diff));
//     return false;
// }
