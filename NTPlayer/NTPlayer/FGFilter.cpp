/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <evr.h>
#include <mpconfig.h>
#include "FGFilter.h"
#include "DSUtil.h"
#include "AllocatorCommon7.h"
#include "AllocatorCommon.h"
#include "SyncAllocatorPresenter.h"
#include "moreuuids.h"


//
// CFGFilter
//
bool CFGFilter::CheckTypes(const CAtlArray<GUID>& types, bool fExactMatch)
{
    if (m_info == NULL)
        return false;

    const CAtlList<MediaTypeItem>& mediatypes = m_info->mediatypes;
    POSITION pos = mediatypes.GetHeadPosition();
    while (pos)
    {
        const MediaTypeItem& item = mediatypes.GetNext(pos);
        const GUID& majortype = item.majortype;
        const GUID& subtype = item.subtype;

        for (int i = 0, len = types.GetCount() & ~1; i < len; i += 2)
        {
            if (fExactMatch)
            {
                if (majortype == types[i] && majortype != GUID_NULL
                        && subtype == types[i + 1] && subtype != GUID_NULL)
                {
                    return true;
                }
            }
            else
            {
                if ((majortype == GUID_NULL || types[i] == GUID_NULL || majortype == types[i])
                        && (subtype == GUID_NULL || types[i + 1] == GUID_NULL || subtype == types[i + 1]))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

//
// CFGFilterRegistry
//

HRESULT CFGFilterRegistry::Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    CheckPointer(ppBF, E_POINTER);

    HRESULT hr = E_FAIL;

    if (m_info == NULL)
        return hr;

    if (m_info->clsid != GUID_NULL)
    {
        CComQIPtr<IBaseFilter> pBF;

        if (FAILED(pBF.CoCreateInstance(m_info->clsid)))
        {
            return E_FAIL;
        }

        *ppBF = pBF.Detach();

        hr = S_OK;
    }

    return hr;
};


//
// CFGFilterFile
//
HRESULT CFGFilterFile::Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    CheckPointer(ppBF, E_POINTER);

    if (m_info == NULL)
        return E_FAIL;

    return LoadExternalFilter(m_info->path, m_info->clsid, ppBF);
}

//
// CFGFilterVideoRenderer
//
HRESULT CFGFilterVideoRenderer::Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    //TRACE(_T("--> CFGFilterVideoRenderer::Create on thread: %d\n"), GetCurrentThreadId());
    CheckPointer(ppBF, E_POINTER);

    if (m_info == NULL)
        return E_FAIL;

    HRESULT hr = S_OK;

    CComPtr<ISubPicAllocatorPresenter> pCAP;

    const GUID& clsid = m_info->clsid;

    if (clsid == CLSID_VMR7AllocatorPresenter   ||
        clsid == CLSID_VMR9AllocatorPresenter   ||
        clsid == CLSID_DXRAllocatorPresenter    ||
        clsid == CLSID_madVRAllocatorPresenter  ||
        clsid == CLSID_EVRAllocatorPresenter    ||
        clsid == CLSID_SyncAllocatorPresenter)
    {
        //bool bFullscreen = (AfxGetApp()->m_pMainWnd != NULL) && (((CMainFrame*)AfxGetApp()->m_pMainWnd)->IsD3DFullScreenMode());
        bool bFullscreen = false;
        if (SUCCEEDED(CreateAP7(clsid, m_hWnd, &pCAP))  ||
            SUCCEEDED(CreateAP9(clsid, m_hWnd, bFullscreen, &pCAP)) ||
            SUCCEEDED(CreateEVR(clsid, m_hWnd, bFullscreen, &pCAP)) ||
            SUCCEEDED(CreateSyncRenderer(clsid, m_hWnd, bFullscreen, &pCAP)))
        {
            CComPtr<IUnknown> pRenderer;
            if (SUCCEEDED(hr = pCAP->CreateRenderer(&pRenderer)))
            {
                *ppBF = CComQIPtr<IBaseFilter>(pRenderer).Detach();
                pUnks.AddTail(pCAP);
                if (CComQIPtr<ISubPicAllocatorPresenter2> pCAP2 = pCAP)
                {
                    pUnks.AddTail(pCAP2);
                }
                // madVR supports calling IVideoWindow::put_Owner before the pins are connected
                if (clsid == CLSID_madVRAllocatorPresenter)
                {
                    if (CComQIPtr<IVideoWindow> pVW = pCAP)
                    {
                        pVW->put_Owner((OAHWND)m_hWnd);
                    }
                }
            }
        }
    }
    else
    {
        CComPtr<IBaseFilter> pBF;
        if (SUCCEEDED(pBF.CoCreateInstance(clsid)))
        {
            if (clsid == CLSID_EnhancedVideoRenderer)
            {
                CComQIPtr<IEVRFilterConfig> pConfig = pBF;
                pConfig->SetNumberOfStreams(3);
            }

            BeginEnumPins(pBF, pEP, pPin)
            {
                if (CComQIPtr<IMixerPinConfig, &IID_IMixerPinConfig> pMPC = pPin)
                {
                    pUnks.AddTail(pMPC);
                    break;
                }
            }
            EndEnumPins;

            *ppBF = pBF.Detach();
        }
		else
		{
			LPOLESTR pStr = NULL;
			if (S_OK == StringFromCLSID(clsid, &pStr) && pStr)
			{
				//ShowDebug(_T("CreateFilter failed, %s"), (LPTSTR)pStr);
				CoTaskMemFree(pStr);
			}
		}
    }

    if (!*ppBF)
    {
        hr = E_FAIL;
    }

    return hr;
}

//
// CFGFilterList
//

CFGFilterList::CFGFilterList()
{
}

CFGFilterList::~CFGFilterList()
{
    RemoveAll();
}

void CFGFilterList::RemoveAll()
{
    while (!m_filters.IsEmpty())
    {
        const filter_t& f = m_filters.RemoveHead();
        if (f.autodelete)
        {
            delete f.pFGF;
        }
    }

    m_sortedfilters.RemoveAll();
}

void CFGFilterList::Insert(CFGFilter* pFGF, int group, bool exactmatch, bool autodelete)
{
    bool bInsert = true;

//     TRACE(_T("FGM: Inserting %d %d %016I64x '%s' --> "), group, exactmatch, pFGF->GetMerit(),
//           pFGF->GetName().IsEmpty() ? CStringFromGUID(pFGF->GetCLSID()) : CString(pFGF->GetName()));

    CFGFilterRegistry* pFGFR = dynamic_cast<CFGFilterRegistry*>(pFGF);

    POSITION pos = m_filters.GetHeadPosition();
    while (pos)
    {
        filter_t& f = m_filters.GetNext(pos);

        if (pFGF == f.pFGF)
        {
            TRACE(_T("Rejected (exact duplicate)\n"));
            bInsert = false;
            break;
        }

        if (group != f.group)
        {
            continue;
        }

        if (pFGF->GetCLSID() != GUID_NULL && pFGF->GetCLSID() == f.pFGF->GetCLSID()
                && f.pFGF->GetMerit() == MERIT64_DO_NOT_USE)
        {
            TRACE(_T("Rejected (same filter with merit DO_NOT_USE already in the list)\n"));
            bInsert = false;
            break;
        }
    }

    if (bInsert)
    {
        TRACE(_T("Success\n"));

        filter_t f = {(int)m_filters.GetCount(), pFGF, group, exactmatch, autodelete};
        m_filters.AddTail(f);

        m_sortedfilters.RemoveAll();
    }
    else if (autodelete)
    {
        delete pFGF;
    }
}

POSITION CFGFilterList::GetHeadPosition()
{
    if (m_sortedfilters.IsEmpty())
    {
        CAtlArray<filter_t> sort;
        sort.SetCount(m_filters.GetCount());
        POSITION pos = m_filters.GetHeadPosition();
        for (int i = 0; pos; i++)
        {
            sort[i] = m_filters.GetNext(pos);
        }
        qsort(&sort[0], sort.GetCount(), sizeof(sort[0]), filter_cmp);
        for (size_t i = 0; i < sort.GetCount(); i++)
        {
            if (sort[i].pFGF->GetMerit() >= MERIT64_DO_USE)
            {
                m_sortedfilters.AddTail(sort[i].pFGF);
            }
        }
    }

#ifdef _DEBUG
    TRACE(_T("FGM: Sorting filters\n"));

    POSITION pos = m_sortedfilters.GetHeadPosition();
    while (pos)
    {
        CFGFilter* pFGF = m_sortedfilters.GetNext(pos);
        TRACE(_T("FGM: - %016I64x '%s'\n"), pFGF->GetMerit(), pFGF->GetName().IsEmpty() ? CStringFromGUID(pFGF->GetCLSID()) : CString(pFGF->GetName()));
    }
#endif

    return m_sortedfilters.GetHeadPosition();
}

CFGFilter* CFGFilterList::GetNext(POSITION& pos)
{
    return m_sortedfilters.GetNext(pos);
}

int CFGFilterList::filter_cmp(const void* a, const void* b)
{
    filter_t* fa = (filter_t*)a;
    filter_t* fb = (filter_t*)b;

    if (fa->group < fb->group) {
        return -1;
    }
    if (fa->group > fb->group) {
        return +1;
    }

    if (fa->pFGF->GetMerit() > fb->pFGF->GetMerit()) {
        return -1;
    }
    if (fa->pFGF->GetMerit() < fb->pFGF->GetMerit()) {
        return +1;
    }

//     if (fa->pFGF->GetCLSID() == fb->pFGF->GetCLSID()) {
//         CFGFilterFile* fgfa = dynamic_cast<CFGFilterFile*>(fa->pFGF);
//         CFGFilterFile* fgfb = dynamic_cast<CFGFilterFile*>(fb->pFGF);
// 
//         if (fgfa && !fgfb) {
//             return -1;
//         }
//         if (!fgfa && fgfb) {
//             return +1;
//         }
//     }

    if (fa->exactmatch && !fb->exactmatch) {
        return -1;
    }
    if (!fa->exactmatch && fb->exactmatch) {
        return +1;
    }

    if (fa->index < fb->index) {
        return -1;
    }
    if (fa->index > fb->index) {
        return +1;
    }

    return 0;
}
