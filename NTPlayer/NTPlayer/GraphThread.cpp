#include "stdafx.h"
#include "GraphThread.h"

DWORD CGraphThread::ThreadProc()
{
    for (;;)
    {
        DWORD msg = GetRequest();

        switch (msg)
        {
        case GM_OPEN:
            OnOpen();
            break;

        case GM_CLOSE:
            OnClose();
            break;

        case GM_EXIT:
            OnExit();
            return 0;
        }
    }
    return 0;
}

void CGraphThread::OpenMedia(CAutoPtr<OpenMediaData> pOMD)
{
    m_pOMD = pOMD;

    CallWorker(GM_OPEN);
}

void CGraphThread::CloseMedia()
{
    m_pOMD.Free();

    CallWorker(GM_CLOSE);
}

void CGraphThread::Exit()
{
    m_pOMD.Free();

    CallWorker(GM_EXIT);
}


void CGraphThread::OnOpen(/*void* pParam*/)
{
    Reply(S_OK);

    HRESULT hr = E_FAIL;
    //CAutoPtr<OpenMediaData> pOMD((OpenMediaData*)m_pParam);
        
    if (m_pPlayer)
        hr = m_pPlayer->OpenMediaPrivate(m_pOMD);

    if (SUCCEEDED(hr))
    {
        m_pPlayer->OnOpenResult(hr);
    }
}

void CGraphThread::OnClose(/*void* pParam*/)
{
    if (m_pPlayer)
    {
        m_pPlayer->CloseMediaPrivate();
    }
    Reply(S_OK);
}

void CGraphThread::OnExit(/*void* pParam*/)
{
    Reply(S_OK);
}

HRESULT CGraphThread::PostGraphMessage(GraphMessage msg, void* param)
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_csLock);

    m_pParam = param;

    CallWorker(msg);

    return hr;
}