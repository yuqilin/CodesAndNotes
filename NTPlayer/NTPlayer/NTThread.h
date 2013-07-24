
#ifndef __NTPLAYER_NTTHREAD_H__
#define __NTPLAYER_NTTHREAD_H__

class CNTThread
{

public:
    //
    // Call before calling any other CNTThread function
    static void                 Initialize();
                
    CNTThread();
    virtual                     ~CNTThread();
    
    //
    // Derived classes must implement their own entry function
    virtual     void            Entry() = 0;
                void            Start();
                
                static void     ThreadYield();
                static void     Sleep(DWORD inMsec);
                
                void            Join();
                void            SendStopRequest() { fStopRequested = true; }
                BOOL            IsStopRequested() { return fStopRequested; }
                void            StopAndWaitForThread();

                void*           GetThreadData()         { return fThreadData; }
                void            SetThreadData(void* inThreadData) { fThreadData = inThreadData; }
                                
                static void*    GetMainThreadData()     { return sMainThreadData; }
                static void     SetMainThreadData(void* inData) { sMainThreadData = inData; }
                static void     SetUser(char *user) {::strncpy(sUser,user, sizeof(sUser) -1); sUser[sizeof(sUser) -1]=0;} 
                static void     SetGroup(char *group) {::strncpy(sGroup,group, sizeof(sGroup) -1); sGroup[sizeof(sGroup) -1]=0;} 
                static void     SetPersonality(char *user, char* group) { SetUser(user); SetGroup(group); };
                BOOL            SwitchPersonality();

    static int                  GetErrno();
    static DWORD                GetCurrentThreadID() { return ::GetCurrentThreadId(); }

    static      CNTThread*      GetCurrent();
    
private:
    static DWORD                sThreadStorageIndex;

    static char                 sUser[128];
    static char                 sGroup[128];


    BOOL                        fStopRequested;
    BOOL                        fJoined;

    HANDLE                      fThreadID;
    void*                       fThreadData;

    static void*                sMainThreadData;
    static unsigned int WINAPI  _Entry(LPVOID inThread);

};

class CNTThreadDataSetter
{
    public:
        CNTThreadDataSetter(void* inInitialValue, void* inFinalValue) : fFinalValue(inFinalValue)
            { CNTThread::GetCurrent()->SetThreadData(inInitialValue); }

        ~CNTThreadDataSetter() { CNTThread::GetCurrent()->SetThreadData(fFinalValue); }

    private:
        void*   fFinalValue;
};


#endif

