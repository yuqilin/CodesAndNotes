#ifndef _NTPLAYER_GRAPHTHREAD_H_
#define _NTPLAYER_GRAPHTHREAD_H_

class CGraphThread : public CAMThread
{
public:
    CGraphThread(CPlayerCore* player);
    ~CGraphThread();

    DWORD ThreadProc();

    void PostGraphMessage(GraphMessage msg, LPVOID param);

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

    CPlayerCore* m_player;

};

#endif