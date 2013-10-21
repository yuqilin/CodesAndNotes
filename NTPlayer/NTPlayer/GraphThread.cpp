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

void CGraphThread::OnOpen(/*void* pParam*/)
{
    Reply(S_OK);

    HRESULT hr = E_FAIL;
    CAutoPtr<OpenMediaData> pOMD((OpenMediaData*)m_pParam);
    
    
    if (pOMD && m_pPlayer)
        hr = m_pPlayer->OpenMediaPrivate(pOMD);

    if (SUCCEEDED(hr))
    {
        m_pPlayer->OnOpenResult(hr);
    }
}

void CGraphThread::OnClose(/*void* pParam*/)
{
    Reply(S_OK);
//     CAMEvent* evt = (CAMEvent*)pParam;
//     if (evt)
//     {
//         evt->Set();
//     }
}

void CGraphThread::OnExit(/*void* pParam*/)
{
    Reply(S_OK);
//     CAMEvent* evt = (CAMEvent*)pParam;
//     if (evt)
//     {
//         evt->Set();
//     }
}

HRESULT CGraphThread::PostGraphMessage(GraphMessage msg, void* param)
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_csLock);

    m_pParam = param;

    CallWorker(msg);

    return hr;
}