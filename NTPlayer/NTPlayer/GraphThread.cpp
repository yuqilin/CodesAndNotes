#include "stdafx.h"
#include "GraphThread.h"

DWORD CGraphThread::ThreadProc()
{
    bool running = true;
    while (running)
    {
        m_evt.Wait();

        switch (m_msg)
        {
        case GM_OPEN:
            OnOpen();
            m_evt.Reset();
            break;
        case GM_CLOSE:
            OnClose();
            m_evt.Reset();
            break;
        case GM_EXIT:
            OnExit();
            running = false;
            break;
        }
    }

    return 0;
}

void CGraphThread::PostGraphMessage(GraphMessage msg, LPVOID param)
{
    // need lock

    if (msg > GM_INVALID && msg < GM_UNUSED)
    {
        m_param = param;
        m_evt.Set();
    }
}

void CGraphThread::OnOpen()
{
    CAutoPtr<OpenMediaData> pOMD((OpenMediaData*)m_param);
    HRESULT hr = m_player->OpenMediaPrivate(pOMD);
}

void CGraphThread::OnClose()
{
    if (m_player)
    {
        m_player->CloseMediaPrivate();
    }
    if (CAMEvent* e = (CAMEvent*)m_param)
    {
        e->Set();
    }
}

void CGraphThread::OnExit()
{
    if (CAMEvent* e = (CAMEvent*)m_param)
    {
        e->Set();
    }
}