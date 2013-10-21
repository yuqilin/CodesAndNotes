#ifndef _NTPLAYER_GRAPHTHREAD_H_
#define _NTPLAYER_GRAPHTHREAD_H_

class CGraphThread : public CAMThread
{
public:
    CGraphThread(CPlayerCore* player);
    ~CGraphThread();


    enum {GM_OPEN, GM_CLOSE, GM_EXIT};

    DWORD ThreadProc();

    HRESULT PostGraphMessage(GraphMessage msg, void* param);


    HRESULT OnOpen(void* omd);
    HRESULT OnClose(void* evt);
    HRESULT OnExit(void* evt);




protected:
    void OnOpen();
    void OnClose();
    void OnExit();

private:
    typedef enum GraphMessage
    {
        GM_INVALID,
        GM_EXIT,
        GM_OPEN,
        GM_CLOSE,

        GM_UNUSED,
    };

    GraphMessage m_msg;
    void* m_param;


    CAMEvent m_evt;

    CPlayerCore* m_pPlayer;

    void* m_pParam;

    CCritSec m_csLock;

    //CAutoPtr<OpenMediaData> m_pOMD;

};

#endif