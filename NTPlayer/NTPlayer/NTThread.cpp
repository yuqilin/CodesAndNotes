
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "NTThread.h"

void*   CNTThread::sMainThreadData = NULL;

DWORD   CNTThread::sThreadStorageIndex = 0;

char    CNTThread::sUser[128] = "";
char    CNTThread::sGroup[128] = "";

void CNTThread::Initialize()
{
    sThreadStorageIndex = ::TlsAlloc();
}

CNTThread::CNTThread()
:   fStopRequested(false),
    fJoined(false),
    fThreadData(NULL)
{
}

CNTThread::~CNTThread()
{
    this->StopAndWaitForThread();
}

void CNTThread::Start()
{
    unsigned int theId = 0; // We don't care about the identifier
    fThreadID = (HANDLE)_beginthreadex( NULL,   // Inherit security
                                        0,      // Inherit stack size
                                        _Entry, // Entry function
                                        (void*)this,    // Entry arg
                                        0,      // Begin executing immediately
                                        &theId ); 
}

void CNTThread::StopAndWaitForThread()
{
    fStopRequested = true;
    if (!fJoined)
        Join();
}

void CNTThread::Join()
{
    // What we're trying to do is allow the thread we want to delete to complete
    // running. So we wait for it to stop.
    //assert(!fJoined);
    fJoined = true;
    DWORD theErr = ::WaitForSingleObject(fThreadID, INFINITE);
    //assert(theErr == WAIT_OBJECT_0);
}

void CNTThread::ThreadYield()
{
    // on platforms who's threading is not pre-emptive yield 
    // to another thread
}

void CNTThread::Sleep(DWORD inMsec)
{
    ::Sleep(inMsec);
}

unsigned int WINAPI CNTThread::_Entry(LPVOID inThread)
{
    CNTThread* theThread = (CNTThread*)inThread;
    BOOL theErr = ::TlsSetValue(sThreadStorageIndex, theThread);
    //assert(theErr == TRUE);

    theThread->SwitchPersonality();
    //
    // Run the thread
    theThread->Entry();
    return NULL;
}

BOOL CNTThread::SwitchPersonality()
{
   return true;
}

CNTThread*   CNTThread::GetCurrent()
{
    return (CNTThread *)::TlsGetValue(sThreadStorageIndex);
}

int CNTThread::GetErrno()
{
    int winErr = ::GetLastError();

    return winErr;
}
