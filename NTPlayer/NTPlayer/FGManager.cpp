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
#include <atlpath.h>
#include <mpconfig.h>
#include "FGManager.h"
#include "DSUtil.h"
#include "FileVersionInfo.h"
#include "WinAPIUtils.h"
//#include "../filters/Filters.h"
#include "AllocatorCommon7.h"
#include "AllocatorCommon.h"
#include "SyncAllocatorPresenter.h"
#include "madVRAllocatorPresenter.h"
//#include "DeinterlacerFilter.h"
//#include "../DeCSS/VobFile.h"
#include <InitGuid.h>
#include <dmodshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <evr.h>
#include <evr9.h>
//#include <ksproxy.h>
#include "moreuuids.h"

//
// CFGManager
//

HRESULT CFGManager::LoadCodecsInfo()
{
    // load codecsinfo.xml


    // create CFGFilter
    POSITION pos = m_Codecs.GetHeadPosition();
    while (pos)
    {
        CFGFilter* pFGF = NULL;
        CodecsInfo* pInfo = m_Codecs.GetNext(pos);
        if (pInfo)
        {
            if (pInfo->type == kCodecsTypeVideoRenderer)
            {
                pFGF = new CFGFilterVideoRenderer(pInfo);
            }
            else if (pInfo->pathflag == "reg")
            {
                pFGF = new CFGFilterRegistry(pInfo);
            }
            else if (pInfo->pathflag == "file")
            {
                pFGF = new CFGFilterFile(pInfo);
            }

//             if (pInfo->type == kCodecsTypeSourceFilter)
//             {
//                 m_source.AddTail(pFGF);
//             }
//             else
//             {
//                 m_transform.AddTail(pFGF);
//             }
        }
    }

    return S_OK;
}

CodecsInfo* CFGManager::FindCodecsInfo(const CString& clsid)
{
    CodecsInfo* pInfo = NULL;

    return pInfo;
}

CFGManager::CFGManager(LPCTSTR pName, LPUNKNOWN pUnk)
: CUnknown(pName, pUnk)
, m_dwRegister(0)
{
    m_pUnkInner.CoCreateInstance(CLSID_FilterGraph, GetOwner());
    m_pFM.CoCreateInstance(CLSID_FilterMapper2);
}

CFGManager::~CFGManager()
{
    CAutoLock cAutoLock(this);
    while (!m_source.IsEmpty()) {
        delete m_source.RemoveHead();
    }
    while (!m_transform.IsEmpty()) {
        delete m_transform.RemoveHead();
    }
    while (!m_override.IsEmpty()) {
        delete m_override.RemoveHead();
    }
    m_pUnks.RemoveAll();
    m_pUnkInner.Release();
}

STDMETHODIMP CFGManager::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(IFilterGraph)
        QI(IGraphBuilder)
        QI(IFilterGraph2)
        QI(IGraphBuilder2)
        QI(IGraphBuilderDeadEnd)
        m_pUnkInner && (riid != IID_IUnknown && SUCCEEDED(m_pUnkInner->QueryInterface(riid, ppv))) ? S_OK :
        __super::NonDelegatingQueryInterface(riid, ppv);
}

//

void CFGManager::CStreamPath::Append(IBaseFilter* pBF, IPin* pPin)
{
    path_t p;
    p.clsid = GetCLSID(pBF);
    p.filter = GetFilterName(pBF);
    p.pin = GetPinName(pPin);
    AddTail(p);
}

bool CFGManager::CStreamPath::Compare(const CStreamPath& path)
{
    POSITION pos1 = GetHeadPosition();
    POSITION pos2 = path.GetHeadPosition();

    while (pos1 && pos2) {
        const path_t& p1 = GetNext(pos1);
        const path_t& p2 = path.GetNext(pos2);

        if (p1.filter != p2.filter) {
            return true;
        } else if (p1.pin != p2.pin) {
            return false;
        }
    }

    return true;
}

//

bool CFGManager::CheckBytes(HANDLE hFile, CString chkbytes)
{
    CAtlList<CString> sl;
    Explode(chkbytes, sl, ',');

    if (sl.GetCount() < 4) {
        return false;
    }

    ASSERT(!(sl.GetCount() & 3));

    LARGE_INTEGER size = {0, 0};
    GetFileSizeEx(hFile, &size);

    while (sl.GetCount() >= 4) {
        CString offsetstr = sl.RemoveHead();
        CString cbstr = sl.RemoveHead();
        CString maskstr = sl.RemoveHead();
        CString valstr = sl.RemoveHead();

        long cb = _ttol(cbstr);

        if (offsetstr.IsEmpty() || cbstr.IsEmpty()
            || valstr.IsEmpty() || (valstr.GetLength() & 1)
            || cb * 2 != valstr.GetLength()) {
                return false;
        }

        LARGE_INTEGER offset;
        offset.QuadPart = _ttoi64(offsetstr);
        if (offset.QuadPart < 0) {
            offset.QuadPart = size.QuadPart - offset.QuadPart;
        }
        SetFilePointerEx(hFile, offset, &offset, FILE_BEGIN);

        // LAME
        while (maskstr.GetLength() < valstr.GetLength()) {
            maskstr += 'F';
        }

        CAtlArray<BYTE> mask, val;
        CStringToBin(maskstr, mask);
        CStringToBin(valstr, val);

        for (size_t i = 0; i < val.GetCount(); i++) {
            BYTE b;
            DWORD r;
            if (!ReadFile(hFile, &b, 1, &r, NULL) || (b & mask[i]) != val[i]) {
                return false;
            }
        }
    }

    return sl.IsEmpty();
}

//CFGFilter* LookupFilterRegistry(const GUID& guid, CAtlList<CFGFilter*>& list, UINT64 fallback_merit = MERIT64_DO_USE)
//{
//    POSITION pos = list.GetHeadPosition();
//    CFGFilter* pFilter = NULL;
//    while (pos) {
//        CFGFilter* pFGF = list.GetNext(pos);
//        if (pFGF->GetCLSID() == guid) {
//            pFilter = pFGF;
//            break;
//        }
//    }
//    if (pFilter) {
//        return new CFGFilterRegistry(guid, pFilter->GetMerit());
//    } else {
//        return new CFGFilterRegistry(guid, fallback_merit);
//    }
//}

HRESULT CFGManager::EnumSourceFilters(LPCWSTR lpcwstrFileName, CFGFilterList& fl)
{
    // TODO: use overrides

    CheckPointer(lpcwstrFileName, E_POINTER);

    fl.RemoveAll();

    CStringW fn = CStringW(lpcwstrFileName).TrimLeft();
    CStringW protocol = fn.Left(fn.Find(':') + 1).TrimRight(':').MakeLower();
    CStringW ext = CPathW(fn).GetExtension().MakeLower();

    HANDLE hFile = INVALID_HANDLE_VALUE;

    if (protocol.GetLength() <= 1 || protocol == L"file")
    {
        hFile = CreateFile(CString(fn), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            return VFW_E_NOT_FOUND;
        }
    }

//     TCHAR buff[256];
//     ULONG len;

    if (hFile == INVALID_HANDLE_VALUE)
    {
        // internal / protocol

        POSITION pos = m_source.GetHeadPosition();
        while (pos)
        {
            CFGFilter* pFGF = m_source.GetNext(pos);
            if (pFGF && pFGF->m_info && 
                pFGF->m_info->protocols.Find(CString(protocol)))
            {
                fl.Insert(pFGF, 0, false, false);
            }
        }
    }
    else
    {
        // internal / check bytes

        POSITION pos = m_source.GetHeadPosition();
        while (pos)
        {
            CFGFilter* pFGF = m_source.GetNext(pos);

            if (pFGF && pFGF->m_info)
            {
                POSITION pos2 = pFGF->m_info->checkbytes.GetHeadPosition();
                while (pos2)
                {
                    if (CheckBytes(hFile, pFGF->m_info->checkbytes.GetNext(pos2)))
                    {
                        fl.Insert(pFGF, 1, false, false);
                        break;
                    }
                }
            }
        }
    }

    if (!ext.IsEmpty())
    {
        // internal / file extension

        POSITION pos = m_source.GetHeadPosition();
        while (pos)
        {
            CFGFilter* pFGF = m_source.GetNext(pos);

            if (pFGF && pFGF->m_info)
            {
                if (pFGF->m_info->extensions.Find(CString(ext)))
                {
                    fl.Insert(pFGF, 2, false, false);
                }
            }
        }
    }

    {
        // internal / the rest

        POSITION pos = m_source.GetHeadPosition();
        while (pos)
        {
            CFGFilter* pFGF = m_source.GetNext(pos);

            if (pFGF && pFGF->m_info)
            {
                if (pFGF->m_info->protocols.IsEmpty() &&
                    pFGF->m_info->checkbytes.IsEmpty() &&
                    pFGF->m_info->extensions.IsEmpty())
                {
                    fl.Insert(pFGF, 3, false, false);
                }
            }
        }
    }

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }

//     CFGFilter* pFGF = LookupFilterRegistry(CLSID_AsyncReader, m_override);
//     pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_NULL);
//     fl.Insert(pFGF, 9);

    return S_OK;
}

HRESULT CFGManager::AddSourceFilter(CFGFilter* pFGF, LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppBF)
{
    TRACE(_T("FGM: AddSourceFilter trying '%s'\n"), CStringFromGUID(pFGF->GetCLSID()));

    CheckPointer(lpcwstrFileName, E_POINTER);
    CheckPointer(ppBF, E_POINTER);

    ASSERT(*ppBF == NULL);

    HRESULT hr;

    CComPtr<IBaseFilter> pBF;
    CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
    if (FAILED(hr = pFGF->Create(&pBF, pUnks))) {
        return hr;
    }

    CComQIPtr<IFileSourceFilter> pFSF = pBF;
    if (!pFSF) {
        return E_NOINTERFACE;
    }

    if (FAILED(hr = AddFilter(pBF, lpcwstrFilterName))) {
        return hr;
    }

    const AM_MEDIA_TYPE* pmt = NULL;

    CMediaType mt;
    const CAtlList<GUID>& types = pFGF->GetTypes();
    if (types.GetCount() == 2 && (types.GetHead() != GUID_NULL || types.GetTail() != GUID_NULL))
    {
        mt.majortype = types.GetHead();
        mt.subtype = types.GetTail();
        pmt = &mt;
    }

    // sometimes looping with AviSynth
    if (FAILED(hr = pFSF->Load(lpcwstrFileName, pmt))) {
        RemoveFilter(pBF);
        return hr;
    }

    // doh :P
    BeginEnumMediaTypes(GetFirstPin(pBF, PINDIR_OUTPUT), pEMT, pmt) {
        static const GUID guid1 =
        { 0x640999A0, 0xA946, 0x11D0, { 0xA5, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
        static const GUID guid2 =
        { 0x640999A1, 0xA946, 0x11D0, { 0xA5, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
        static const GUID guid3 =
        { 0xD51BD5AE, 0x7548, 0x11CF, { 0xA5, 0x20, 0x00, 0x80, 0xC7, 0x7E, 0xF5, 0x8A } };

        if (pmt->subtype == guid1 || pmt->subtype == guid2 || pmt->subtype == guid3) {
            RemoveFilter(pBF);
            pFGF = new CFGFilterRegistry(CLSID_NetShowSource);
            hr = AddSourceFilter(pFGF, lpcwstrFileName, lpcwstrFilterName, ppBF);
            delete pFGF;
            return hr;
        }
    }
    EndEnumMediaTypes(pmt);

    *ppBF = pBF.Detach();

    m_pUnks.AddTailList(&pUnks);

    return S_OK;
}

// IFilterGraph

STDMETHODIMP CFGManager::AddFilter(IBaseFilter* pFilter, LPCWSTR pName)
{
    if (!m_pUnkInner) {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(this);

    HRESULT hr;

    if (FAILED(hr = CComQIPtr<IFilterGraph2>(m_pUnkInner)->AddFilter(pFilter, pName))) {
        return hr;
    }

    // TODO
    hr = pFilter->JoinFilterGraph(NULL, NULL);
    hr = pFilter->JoinFilterGraph(this, pName);

    return hr;
}

STDMETHODIMP CFGManager::RemoveFilter(IBaseFilter* pFilter)
{
    if (!m_pUnkInner) {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(this);

    return CComQIPtr<IFilterGraph2>(m_pUnkInner)->RemoveFilter(pFilter);
}

STDMETHODIMP CFGManager::EnumFilters(IEnumFilters** ppEnum)
{
    if (!m_pUnkInner) {
        return E_UNEXPECTED;
    }

    // Not locking here fixes a deadlock involving ReClock
    //CAutoLock cAutoLock(this);

    return CComQIPtr<IFilterGraph2>(m_pUnkInner)->EnumFilters(ppEnum);
}

STDMETHODIMP CFGManager::FindFilterByName(LPCWSTR pName, IBaseFilter** ppFilter)
{
    if (!m_pUnkInner) {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(this);

    return CComQIPtr<IFilterGraph2>(m_pUnkInner)->FindFilterByName(pName, ppFilter);
}

STDMETHODIMP CFGManager::ConnectDirect(IPin* pPinOut, IPin* pPinIn, const AM_MEDIA_TYPE* pmt)
{
    if (!m_pUnkInner) {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(this);

    CComPtr<IBaseFilter> pBF = GetFilterFromPin(pPinIn);
    CLSID clsid = GetCLSID(pBF);

    // TODO: GetUpStreamFilter goes up on the first input pin only
    for (CComPtr<IBaseFilter> pBFUS = GetFilterFromPin(pPinOut); pBFUS; pBFUS = GetUpStreamFilter(pBFUS)) {
        if (pBFUS == pBF) {
            return VFW_E_CIRCULAR_GRAPH;
        }
        if (clsid != CLSID_Proxy && GetCLSID(pBFUS) == clsid) {
            return VFW_E_CANNOT_CONNECT;
        }
    }

    return CComQIPtr<IFilterGraph2>(m_pUnkInner)->ConnectDirect(pPinOut, pPinIn, pmt);
}

STDMETHODIMP CFGManager::Reconnect(IPin* ppin)
{
    if (!m_pUnkInner) {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(this);

    return CComQIPtr<IFilterGraph2>(m_pUnkInner)->Reconnect(ppin);
}

STDMETHODIMP CFGManager::Disconnect(IPin* ppin)
{
    if (!m_pUnkInner) {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(this);

    return CComQIPtr<IFilterGraph2>(m_pUnkInner)->Disconnect(ppin);
}

STDMETHODIMP CFGManager::SetDefaultSyncSource()
{
    if (!m_pUnkInner) {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(this);

    return CComQIPtr<IFilterGraph2>(m_pUnkInner)->SetDefaultSyncSource();
}

// IGraphBuilder

STDMETHODIMP CFGManager::Connect(IPin* pPinOut, IPin* pPinIn)
{
    return Connect(pPinOut, pPinIn, true);
}

HRESULT CFGManager::Connect(IPin* pPinOut, IPin* pPinIn, bool bContinueRender)
{
    CAutoLock cAutoLock(this);

    CheckPointer(pPinOut, E_POINTER);

    HRESULT hr;

    if (S_OK != IsPinDirection(pPinOut, PINDIR_OUTPUT)
        || pPinIn && S_OK != IsPinDirection(pPinIn, PINDIR_INPUT)) {
            return VFW_E_INVALID_DIRECTION;
    }

    if (S_OK == IsPinConnected(pPinOut)
        || pPinIn && S_OK == IsPinConnected(pPinIn)) {
            return VFW_E_ALREADY_CONNECTED;
    }

    bool fDeadEnd = true;

    if (pPinIn)
    {
        // 1. Try a direct connection between the filters, with no intermediate filters
        if (SUCCEEDED(hr = ConnectDirect(pPinOut, pPinIn, NULL)))
        {
            return hr;
        }
    }

    // 1. Try filters in the graph
    {
        CInterfaceList<IBaseFilter> pBFs;

        BeginEnumFilters(this, pEF, pBF)
        {
            if (pPinIn && GetFilterFromPin(pPinIn) == pBF
                || GetFilterFromPin(pPinOut) == pBF)
            {
                continue;
            }

            // HACK: ffdshow - audio capture filter
            if (GetCLSID(pPinOut) == GUIDFromCString(_T("{04FE9017-F873-410E-871E-AB91661A4EF7}"))
                && GetCLSID(pBF) == GUIDFromCString(_T("{E30629D2-27E5-11CE-875D-00608CB78066}")))
            {
                continue;
            }

            pBFs.AddTail(pBF);
        }
        EndEnumFilters;

        POSITION pos = pBFs.GetHeadPosition();
        while (pos)
        {
            IBaseFilter* pBF = pBFs.GetNext(pos);

            if (SUCCEEDED(hr = ConnectFilterDirect(pPinOut, pBF, NULL)))
            {
                if (!IsStreamEnd(pBF))
                {
                    fDeadEnd = false;
                }

                if (SUCCEEDED(hr = ConnectFilter(pBF, pPinIn)))
                {
                    return hr;
                }
            }

            EXECUTE_ASSERT(Disconnect(pPinOut));
        }
    }

    // 2. Look up filters in the <codecs>
    {
        CFGFilterList fl;

        CAtlArray<GUID> types;
        ExtractMediaTypes(pPinOut, types);

        POSITION pos = m_transform.GetHeadPosition();
        while (pos)
        {
            CFGFilter* pFGF = m_transform.GetNext(pos);
            if (pFGF->GetMerit() < MERIT64_DO_USE || pFGF->CheckTypes(types, false))
            {
                fl.Insert(pFGF, 0, pFGF->CheckTypes(types, true), false);
            }
        }

        pos = m_override.GetHeadPosition();
        while (pos)
        {
            CFGFilter* pFGF = m_override.GetNext(pos);
            if (pFGF->GetMerit() < MERIT64_DO_USE || pFGF->CheckTypes(types, false))
            {
                fl.Insert(pFGF, 0, pFGF->CheckTypes(types, true), false);
            }
        }

        // do not check registry
        //         CComPtr<IEnumMoniker> pEM;
        //         if (!types.IsEmpty()
        //                 && SUCCEEDED(m_pFM->EnumMatchingFilters(
        //                                  &pEM, 0, FALSE, MERIT_DO_NOT_USE + 1,
        //                                  TRUE, (DWORD)types.GetCount() / 2, types.GetData(), NULL, NULL, FALSE,
        //                                  !!pPinIn, 0, NULL, NULL, NULL)))
        //         {
        //             for (CComPtr<IMoniker> pMoniker; S_OK == pEM->Next(1, &pMoniker, NULL); pMoniker = NULL)
        //             {
        //                 CFGFilterRegistry* pFGF = new CFGFilterRegistry(pMoniker);
        //                 fl.Insert(pFGF, 0, pFGF->CheckTypes(types, true));
        //             }
        //         }

        // let's check whether the madVR allocator presenter is in our list
        // it should be if madVR is selected as the video renderer
        CFGFilter* pMadVRAllocatorPresenter = NULL;
        pos = fl.GetHeadPosition();
        while (pos)
        {
            CFGFilter* pFGF = fl.GetNext(pos);
            if (pFGF->GetCLSID() == CLSID_madVRAllocatorPresenter)
            {
                // found it!
                pMadVRAllocatorPresenter = pFGF;
                break;
            }
        }

        pos = fl.GetHeadPosition();
        while (pos)
        {
            CFGFilter* pFGF = fl.GetNext(pos);

            // Checks if madVR is already in the graph to avoid two instances at the same time
            CComPtr<IBaseFilter> pBFmadVR;
            FindFilterByName(_T("madVR Renderer"), &pBFmadVR);
            if (pBFmadVR && (pFGF->GetName() == _T("madVR Renderer")))
            {
                continue;
            }

            if (pMadVRAllocatorPresenter && (pFGF->GetCLSID() == CLSID_madVR))
            {
                // the pure madVR filter was selected (without the allocator presenter)
                // subtitles, OSD etc don't work correctly without the allocator presenter
                // so we prefer the allocator presenter over the pure filter
                pFGF = pMadVRAllocatorPresenter;
            }

            TRACE(_T("FGM: Connecting '%s'\n"), pFGF->GetName());

            CComPtr<IBaseFilter> pBF;
            CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
            if (FAILED(pFGF->Create(&pBF, pUnks)))
            {
                TRACE(_T("     --> Filter creation failed\n"));
                continue;
            }

            if (FAILED(hr = AddFilter(pBF, pFGF->GetName())))
            {
                TRACE(_T("     --> Adding the filter failed\n"));
                pUnks.RemoveAll();
                pBF.Release();
                continue;
            }

            hr = ConnectFilterDirect(pPinOut, pBF, NULL);
            /*
            if (FAILED(hr))
            {
            if (types.GetCount() >= 2 && types[0] == MEDIATYPE_Stream && types[1] != GUID_NULL)
            {
            CMediaType mt;

            mt.majortype = types[0];
            mt.subtype = types[1];
            mt.formattype = FORMAT_None;
            if (FAILED(hr)) hr = ConnectFilterDirect(pPinOut, pBF, &mt);

            mt.formattype = GUID_NULL;
            if (FAILED(hr)) hr = ConnectFilterDirect(pPinOut, pBF, &mt);
            }
            }
            */
            if (SUCCEEDED(hr))
            {
                if (!IsStreamEnd(pBF))
                {
                    fDeadEnd = false;
                }

                if (bContinueRender)
                {
                    hr = ConnectFilter(pBF, pPinIn);
                }

                if (SUCCEEDED(hr))
                {
                    m_pUnks.AddTailList(&pUnks);

                    // maybe the application should do this...

                    POSITION pos = pUnks.GetHeadPosition();
                    while (pos)
                    {
                        if (CComQIPtr<IMixerPinConfig, &IID_IMixerPinConfig> pMPC = pUnks.GetNext(pos))
                        {
                            pMPC->SetAspectRatioMode(AM_ARMODE_STRETCHED);
                        }
                    }

                    if (CComQIPtr<IVMRAspectRatioControl> pARC = pBF)
                    {
                        pARC->SetAspectRatioMode(VMR_ARMODE_NONE);
                    }

                    if (CComQIPtr<IVMRAspectRatioControl9> pARC = pBF)
                    {
                        pARC->SetAspectRatioMode(VMR_ARMODE_NONE);
                    }

                    if (CComQIPtr<IVMRMixerControl9> pMC = pBF)
                    {
                        m_pUnks.AddTail(pMC);
                    }

                    if (CComQIPtr<IVMRMixerBitmap9> pMB = pBF)
                    {
                        m_pUnks.AddTail(pMB);
                    }

                    if (CComQIPtr<IMFGetService, &__uuidof(IMFGetService)> pMFGS = pBF)
                    {
                        CComPtr<IMFVideoDisplayControl> pMFVDC;
                        CComPtr<IMFVideoMixerBitmap>    pMFMB;
                        CComPtr<IMFVideoProcessor>      pMFVP;

                        if (SUCCEEDED(pMFGS->GetService(MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, (void**)&pMFVDC)))
                        {
                            m_pUnks.AddTail(pMFVDC);
                        }

                        if (SUCCEEDED(pMFGS->GetService(MR_VIDEO_MIXER_SERVICE, IID_IMFVideoMixerBitmap, (void**)&pMFMB)))
                        {
                            m_pUnks.AddTail(pMFMB);
                        }

                        if (SUCCEEDED(pMFGS->GetService(MR_VIDEO_MIXER_SERVICE, IID_IMFVideoProcessor, (void**)&pMFVP)))
                        {
                            m_pUnks.AddTail(pMFVP);
                        }

                        //CComPtr<IMFWorkQueueServices> pMFWQS;
                        //pMFGS->GetService (MF_WORKQUEUE_SERVICES, IID_IMFWorkQueueServices, (void**)&pMFWQS);
                        //pMFWQS->BeginRegisterPlatformWorkQueueWithMMCSS(

                    }

                    return hr;
                }
            }

            EXECUTE_ASSERT(SUCCEEDED(RemoveFilter(pBF)));
            TRACE(_T("     --> Failed to connect\n"));
            pUnks.RemoveAll();
            pBF.Release();
        }
    }

    if (fDeadEnd)
    {
        CAutoPtr<CStreamDeadEnd> psde(new CStreamDeadEnd());
        psde->AddTailList(&m_streampath);
        int skip = 0;
        BeginEnumMediaTypes(pPinOut, pEM, pmt)
        {
            if (pmt->majortype == MEDIATYPE_Stream && pmt->subtype == MEDIASUBTYPE_NULL)
            {
                skip++;
            }
            psde->mts.AddTail(CMediaType(*pmt));
        }
        EndEnumMediaTypes(pmt);
        if (skip < (int)psde->mts.GetCount())
        {
            m_deadends.Add(psde);
        }
    }

    return pPinIn ? VFW_E_CANNOT_CONNECT : VFW_E_CANNOT_RENDER;
}

STDMETHODIMP CFGManager::Render(IPin* pPinOut)
{
    CAutoLock cAutoLock(this);

    return RenderEx(pPinOut, 0, NULL);
}

STDMETHODIMP CFGManager::RenderFile(LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrPlayList)
{
    TRACE(_T("--> CFGManager::RenderFile on thread: %d\n"), GetCurrentThreadId());
    CAutoLock cAutoLock(this);

    m_streampath.RemoveAll();
    m_deadends.RemoveAll();

    HRESULT hr;
    HRESULT hrRFS = S_OK;

    /*CComPtr<IBaseFilter> pBF;
    if (FAILED(hr = AddSourceFilter(lpcwstrFile, lpcwstrFile, &pBF)))
    return hr;

    return ConnectFilter(pBF, NULL);*/

    CFGFilterList fl;
    if (FAILED(hr = EnumSourceFilters(lpcwstrFileName, fl)))
    {
        return hr;
    }

    CAutoPtrArray<CStreamDeadEnd> deadends;

    hr = VFW_E_CANNOT_RENDER;

    POSITION pos = fl.GetHeadPosition();
    while (pos)
    {
        CComPtr<IBaseFilter> pBF;
        CFGFilter* pFG = fl.GetNext(pos);

        if (pFG->m_info && SUCCEEDED(hr = AddSourceFilter(pFG, lpcwstrFileName, pFG->m_info->name), &pBF))
        {
            m_streampath.RemoveAll();
            m_deadends.RemoveAll();

            if (SUCCEEDED(hr = ConnectFilter(pBF, NULL)))
            {
                return hr;
            }

            NukeDownstream(pBF);
            RemoveFilter(pBF);

            deadends.Append(m_deadends);
        }
    }

    m_deadends.Copy(deadends);

    // If RFS was part of the graph, return its error code instead of the last error code.
    // TODO: Improve filter error reporting to graph manager.
    return hrRFS != S_OK ? hrRFS : hr;
}

STDMETHODIMP CFGManager::AddSourceFilter(LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppFilter)
{
    CAutoLock cAutoLock(this);

    HRESULT hr;
    CFGFilterList fl;

    if (FAILED(hr = EnumSourceFilters(lpcwstrFileName, fl)))
    {
        return hr;
    }

    POSITION pos = fl.GetHeadPosition();
    while (pos)
    {
        if (SUCCEEDED(hr = AddSourceFilter(fl.GetNext(pos), lpcwstrFileName, lpcwstrFilterName, ppFilter)))
        {
            return hr;
        }
    }

    return VFW_E_CANNOT_LOAD_SOURCE_FILTER;
}

STDMETHODIMP CFGManager::SetLogFile(DWORD_PTR hFile)
{
    if (!m_pUnkInner)
    {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(this);

    return CComQIPtr<IFilterGraph2>(m_pUnkInner)->SetLogFile(hFile);
}

STDMETHODIMP CFGManager::Abort()
{
    if (!m_pUnkInner)
    {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(this);

    return CComQIPtr<IFilterGraph2>(m_pUnkInner)->Abort();
}

STDMETHODIMP CFGManager::ShouldOperationContinue()
{
    if (!m_pUnkInner)
    {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(this);

    return CComQIPtr<IFilterGraph2>(m_pUnkInner)->ShouldOperationContinue();
}

// IFilterGraph2

STDMETHODIMP CFGManager::AddSourceFilterForMoniker(IMoniker* pMoniker, IBindCtx* pCtx, LPCWSTR lpcwstrFilterName, IBaseFilter** ppFilter)
{
    if (!m_pUnkInner)
    {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(this);

    return CComQIPtr<IFilterGraph2>(m_pUnkInner)->AddSourceFilterForMoniker(pMoniker, pCtx, lpcwstrFilterName, ppFilter);
}

STDMETHODIMP CFGManager::ReconnectEx(IPin* ppin, const AM_MEDIA_TYPE* pmt)
{
    if (!m_pUnkInner)
    {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(this);

    return CComQIPtr<IFilterGraph2>(m_pUnkInner)->ReconnectEx(ppin, pmt);
}

STDMETHODIMP CFGManager::RenderEx(IPin* pPinOut, DWORD dwFlags, DWORD* pvContext)
{
    CAutoLock cAutoLock(this);

    m_streampath.RemoveAll();
    m_deadends.RemoveAll();

    if (!pPinOut || dwFlags > AM_RENDEREX_RENDERTOEXISTINGRENDERERS || pvContext)
    {
        return E_INVALIDARG;
    }

    if (dwFlags & AM_RENDEREX_RENDERTOEXISTINGRENDERERS)
    {
        CInterfaceList<IBaseFilter> pBFs;

        BeginEnumFilters(this, pEF, pBF)
        {
            if (CComQIPtr<IAMFilterMiscFlags> pAMMF = pBF)
            {
                if (pAMMF->GetMiscFlags() & AM_FILTER_MISC_FLAGS_IS_RENDERER)
                {
                    pBFs.AddTail(pBF);
                }
            }
            else
            {
                BeginEnumPins(pBF, pEP, pPin)
                {
                    CComPtr<IPin> pPinIn;
                    DWORD size = 1;
                    if (SUCCEEDED(pPin->QueryInternalConnections(&pPinIn, &size)) && size == 0)
                    {
                        pBFs.AddTail(pBF);
                        break;
                    }
                }
                EndEnumPins;
            }
        }
        EndEnumFilters;

        HRESULT hr;

        while (!pBFs.IsEmpty())
        {
            if (SUCCEEDED(hr = ConnectFilter(pPinOut, pBFs.RemoveHead())))
            {
                return hr;
            }
        }

        return VFW_E_CANNOT_RENDER;
    }

    return Connect(pPinOut, (IPin*)NULL);
}

// IGraphBuilder2

STDMETHODIMP CFGManager::IsPinDirection(IPin* pPin, PIN_DIRECTION dir1)
{
    CAutoLock cAutoLock(this);

    CheckPointer(pPin, E_POINTER);

    PIN_DIRECTION dir2;
    if (FAILED(pPin->QueryDirection(&dir2)))
    {
        return E_FAIL;
    }

    return dir1 == dir2 ? S_OK : S_FALSE;
}

STDMETHODIMP CFGManager::IsPinConnected(IPin* pPin)
{
    CAutoLock cAutoLock(this);

    CheckPointer(pPin, E_POINTER);

    CComPtr<IPin> pPinTo;
    return SUCCEEDED(pPin->ConnectedTo(&pPinTo)) && pPinTo ? S_OK : S_FALSE;
}

STDMETHODIMP CFGManager::ConnectFilter(IBaseFilter* pBF, IPin* pPinIn)
{
    CAutoLock cAutoLock(this);

    CheckPointer(pBF, E_POINTER);

    if (pPinIn && S_OK != IsPinDirection(pPinIn, PINDIR_INPUT)) {
        return VFW_E_INVALID_DIRECTION;
    }

    int nTotal = 0, nRendered = 0;

    const CPlayerSettings& s = AfxGetAppSettings();

    BeginEnumPins(pBF, pEP, pPin) {
        if (S_OK == IsPinDirection(pPin, PINDIR_OUTPUT)
            && S_OK != IsPinConnected(pPin)
            && !((s.iDSVideoRendererType != VIDRNDT_DS_EVR_CUSTOM && s.iDSVideoRendererType != VIDRNDT_DS_EVR && s.iDSVideoRendererType != VIDRNDT_DS_SYNC) && GetPinName(pPin)[0] == '~')) {

                CLSID clsid;
                pBF->GetClassID(&clsid);
                // Disable DVD subtitle mixing in EVR (CP) and Sync Renderer for Microsoft DTV-DVD Video Decoder, it corrupts DVD playback.
                if (clsid == CLSID_CMPEG2VidDecoderDS) {
                    if (s.iDSVideoRendererType == VIDRNDT_DS_EVR_CUSTOM || s.iDSVideoRendererType == VIDRNDT_DS_SYNC) {
                        if (GetPinName(pPin)[0] == '~') {
                            continue;
                        }
                    }
                }
                // No multiple pin for Internal MPEG2 Software Decoder, Nvidia PureVideo Decoder, Sonic Cinemaster VideoDecoder
                else if (clsid == CLSID_CMpeg2DecFilter
                    || clsid == CLSID_NvidiaVideoDecoder
                    || clsid == CLSID_SonicCinemasterVideoDecoder) {
                        if (GetPinName(pPin)[0] == '~') {
                            continue;
                        }
                        //TODO: enable multiple pins for the renderer, if the video decoder supports DXVA
                }

                m_streampath.Append(pBF, pPin);

                HRESULT hr = Connect(pPin, pPinIn);

                if (SUCCEEDED(hr)) {
                    for (ptrdiff_t i = m_deadends.GetCount() - 1; i >= 0; i--) {
                        if (m_deadends[i]->Compare(m_streampath)) {
                            m_deadends.RemoveAt(i);
                        }
                    }
                    nRendered++;
                }

                nTotal++;

                m_streampath.RemoveTail();

                if (SUCCEEDED(hr) && pPinIn) {
                    return S_OK;
                }
        }
    }
    EndEnumPins;

    return
        nRendered == nTotal ? (nRendered > 0 ? S_OK : S_FALSE) :
        nRendered > 0 ? VFW_S_PARTIAL_RENDER :
        VFW_E_CANNOT_RENDER;
}

STDMETHODIMP CFGManager::ConnectFilter(IPin* pPinOut, IBaseFilter* pBF)
{
    CAutoLock cAutoLock(this);

    CheckPointer(pPinOut, E_POINTER);
    CheckPointer(pBF, E_POINTER);

    if (S_OK != IsPinDirection(pPinOut, PINDIR_OUTPUT)) {
        return VFW_E_INVALID_DIRECTION;
    }

    const CPlayerSettings& s = CPlayerCore::m_settings;

    BeginEnumPins(pBF, pEP, pPin) {
        if (S_OK == IsPinDirection(pPin, PINDIR_INPUT)
            && S_OK != IsPinConnected(pPin)
            && !((s.iDSVideoRendererType != VIDRNDT_DS_EVR_CUSTOM && s.iDSVideoRendererType != VIDRNDT_DS_EVR && s.iDSVideoRendererType != VIDRNDT_DS_SYNC) && GetPinName(pPin)[0] == '~')) {
                HRESULT hr = Connect(pPinOut, pPin);
                if (SUCCEEDED(hr)) {
                    return hr;
                }
        }
    }
    EndEnumPins;

    return VFW_E_CANNOT_CONNECT;
}

STDMETHODIMP CFGManager::ConnectFilterDirect(IPin* pPinOut, IBaseFilter* pBF, const AM_MEDIA_TYPE* pmt)
{
    CAutoLock cAutoLock(this);

    CheckPointer(pPinOut, E_POINTER);
    CheckPointer(pBF, E_POINTER);

    if (S_OK != IsPinDirection(pPinOut, PINDIR_OUTPUT)) {
        return VFW_E_INVALID_DIRECTION;
    }

    const CPlayerSettings& s = CPlayerCore::m_settings;

    BeginEnumPins(pBF, pEP, pPin) {
        if (S_OK == IsPinDirection(pPin, PINDIR_INPUT)
            && S_OK != IsPinConnected(pPin)
            && !((s.iDSVideoRendererType != VIDRNDT_DS_EVR_CUSTOM && s.iDSVideoRendererType != VIDRNDT_DS_EVR && s.iDSVideoRendererType != VIDRNDT_DS_SYNC) && GetPinName(pPin)[0] == '~')) {
                HRESULT hr = ConnectDirect(pPinOut, pPin, pmt);
                if (SUCCEEDED(hr)) {
                    return hr;
                }
        }
    }
    EndEnumPins;

    return VFW_E_CANNOT_CONNECT;
}

STDMETHODIMP CFGManager::NukeDownstream(IUnknown* pUnk)
{
    CAutoLock cAutoLock(this);

    if (CComQIPtr<IBaseFilter> pBF = pUnk) {
        BeginEnumPins(pBF, pEP, pPin) {
            NukeDownstream(pPin);
        }
        EndEnumPins;
    } else if (CComQIPtr<IPin> pPin = pUnk) {
        CComPtr<IPin> pPinTo;
        if (S_OK == IsPinDirection(pPin, PINDIR_OUTPUT)
            && SUCCEEDED(pPin->ConnectedTo(&pPinTo)) && pPinTo) {
                if (CComPtr<IBaseFilter> pBF = GetFilterFromPin(pPinTo)) {
                    NukeDownstream(pBF);
                    Disconnect(pPinTo);
                    Disconnect(pPin);
                    RemoveFilter(pBF);
                }
        }
    } else {
        return E_INVALIDARG;
    }

    return S_OK;
}

STDMETHODIMP CFGManager::FindInterface(REFIID iid, void** ppv, BOOL bRemove)
{
    CAutoLock cAutoLock(this);

    CheckPointer(ppv, E_POINTER);

    for (POSITION pos = m_pUnks.GetHeadPosition(); pos; m_pUnks.GetNext(pos)) {
        if (SUCCEEDED(m_pUnks.GetAt(pos)->QueryInterface(iid, ppv))) {
            if (bRemove) {
                m_pUnks.RemoveAt(pos);
            }
            return S_OK;
        }
    }

    return E_NOINTERFACE;
}

STDMETHODIMP CFGManager::AddToROT()
{
    CAutoLock cAutoLock(this);

    HRESULT hr;

    if (m_dwRegister) {
        return S_FALSE;
    }

    CComPtr<IRunningObjectTable> pROT;
    CComPtr<IMoniker> pMoniker;
    WCHAR wsz[256];
    swprintf_s(wsz, _countof(wsz), L"FilterGraph %08p pid %08x (MPC)", this, GetCurrentProcessId());
    if (SUCCEEDED(hr = GetRunningObjectTable(0, &pROT))
        && SUCCEEDED(hr = CreateItemMoniker(L"!", wsz, &pMoniker))) {
            hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, (IGraphBuilder2*)this, pMoniker, &m_dwRegister);
    }

    return hr;
}

STDMETHODIMP CFGManager::RemoveFromROT()
{
    CAutoLock cAutoLock(this);

    HRESULT hr;

    if (!m_dwRegister) {
        return S_FALSE;
    }

    CComPtr<IRunningObjectTable> pROT;
    if (SUCCEEDED(hr = GetRunningObjectTable(0, &pROT))
        && SUCCEEDED(hr = pROT->Revoke(m_dwRegister))) {
            m_dwRegister = 0;
    }

    return hr;
}

// IGraphBuilderDeadEnd

STDMETHODIMP_(size_t) CFGManager::GetCount()
{
    CAutoLock cAutoLock(this);

    return m_deadends.GetCount();
}

STDMETHODIMP CFGManager::GetDeadEnd(int iIndex, CAtlList<CStringW>& path, CAtlList<CMediaType>& mts)
{
    CAutoLock cAutoLock(this);

    if (iIndex < 0 || iIndex >= (int)m_deadends.GetCount()) {
        return E_FAIL;
    }

    path.RemoveAll();
    mts.RemoveAll();

    POSITION pos = m_deadends[iIndex]->GetHeadPosition();
    while (pos) {
        const path_t& p = m_deadends[iIndex]->GetNext(pos);

        CStringW str;
        str.Format(L"%s::%s", p.filter, p.pin);
        path.AddTail(str);
    }

    mts.AddTailList(&m_deadends[iIndex]->mts);

    return S_OK;
}

//
//  CFGManagerCustom
//

CFGManagerCustom::CFGManagerCustom(LPCTSTR pName, LPUNKNOWN pUnk)
: CFGManager(pName, pUnk)
{
}

STDMETHODIMP CFGManagerCustom::AddFilter(IBaseFilter* pBF, LPCWSTR pName)
{
    CAutoLock cAutoLock(this);

    HRESULT hr;

    if (FAILED(hr = __super::AddFilter(pBF, pName))) {
        return hr;
    }

    CPlayerSettings& s = CPlayerCore::m_settings();

    if (GetCLSID(pBF) == CLSID_DMOWrapperFilter) {
        if (CComQIPtr<IPropertyBag> pPB = pBF) {
            CComVariant var(true);
            pPB->Write(CComBSTR(L"_HIRESOUTPUT"), &var);
        }
    }

    // set AudioSwitcher
    if (CComQIPtr<IAudioSwitcherFilter> pASF = pBF) {
        pASF->EnableDownSamplingTo441(s.fDownSampleTo441);
        pASF->SetSpeakerConfig(s.fCustomChannelMapping, s.pSpeakerToChannelMap);
        pASF->SetAudioTimeShift(s.fAudioTimeShift ? 10000i64 * s.iAudioTimeShift : 0);
        pASF->SetNormalizeBoost2(s.fAudioNormalize, s.nAudioMaxNormFactor, s.fAudioNormalizeRecover, s.nAudioBoost);
    }

    return hr;
}

//
//  CFGManagerPlayer
//

CFGManagerPlayer::CFGManagerPlayer(LPCTSTR pName, LPUNKNOWN pUnk, HWND hWnd)
: CFGManagerCustom(pName, pUnk)
, m_hWnd(hWnd)
, m_vrmerit(MERIT64(MERIT_PREFERRED))
, m_armerit(MERIT64(MERIT_PREFERRED))
{
    TRACE(_T("--> CFGManagerPlayer::CFGManagerPlayer on thread: %d\n"), GetCurrentThreadId());

    LoadCodecsInfo();   // add each into source or transform

    // preload vsfilter and audioswitcher into graph

    // add video renderer into transform
    // priority:
    // 1. selected
    // 2. manual priority
    // 3. os version (default priority)


    // add audio renderer into transform
    // priority:
    // 1. selected
    // 2. manual priority

    // 简单一些
    // vsfilter, audioswitcher默认预加载到graph
    // renderer的优先级默认为 m_vrmerit + priority
    // 其他类型默认 MERIT_DO_USE + priority
    // 选中类型renderer m_vrmerit +　0x100

    // 标准一些
    // render 优先级默认 MERIT_PREFERRED + priority
    // 其他filter 优先级默认 MERIT_DO_USE + priority
    // 遍历codecs列表求得m_vrmerit
    // 选择时: m_vrmerit + 0x100
    // vsfilter audioswitcher优先级 m_vrmerit + 0x200

    // 初始记录 优先选择的render
    // Open时,调整其优先级

    // AudioRender
    // 初始时,枚举系统音频设备,添加m_transform
    // 默认Default Audio Renderer, MERIT_PREFERRED + priority
    // 选择时, m_armerit + 0x100

    POSITION pos = m_transform.GetHeadPosition();
    while (pos)
    {
        const CFGFilter* pFGF = m_transform.GetNext(pos);
        if (pFGF && pFGF->m_info)
        {
            if (pFGF->m_info->IsVideoType())
            {
                m_vrmerit = max(m_vrmerit, pFGF->GetMerit());
            }
            else if (pFGF->m_info->IsAudioType())
            {
                m_armerit = max(m_armerit, pFGF->GetMerit());
            }
        }
    }

    m_vrmerit += 0x100;
    m_armerit += 0x100;

    GUID selVideoRenderer = GUID_NULL;
    GUID selAudioRenderer = GUID_NULL;

    switch (CPlayerCore::m_settings.iDSVideoRendererType)
    {
    case VIDRNDT_DS_OLDRENDERER:
        selVideoRenderer = CLSID_VideoRenderer;
        break;
    case VIDRNDT_DS_OVERLAYMIXER:
        selVideoRenderer = CLSID_OverlayMixer;
        break;
    case VIDRNDT_DS_VMR7WINDOWED:
        selVideoRenderer = CLSID_VideoMixingRenderer;
        break;
    case VIDRNDT_DS_VMR9WINDOWED:
        selVideoRenderer = CLSID_VideoMixingRenderer9;
        break;
    case VIDRNDT_DS_VMR7RENDERLESS:
        selVideoRenderer = CLSID_VMR7AllocatorPresenter;
        break;
    case VIDRNDT_DS_VMR9RENDERLESS:
        selVideoRenderer = CLSID_VMR9AllocatorPresenter;
        break;
    case VIDRNDT_DS_EVR:
        selVideoRenderer = CLSID_EnhancedVideoRenderer;
        break;
    case VIDRNDT_DS_EVR_CUSTOM:
        selVideoRenderer = CLSID_EVRAllocatorPresenter;
        break;
    case VIDRNDT_DS_DXR:
        selVideoRenderer = CLSID_DXRAllocatorPresenter;
        break;
    case VIDRNDT_DS_MADVR:
        selVideoRenderer = CLSID_madVRAllocatorPresenter;
        break;
    case VIDRNDT_DS_SYNC:
        selVideoRenderer = CLSID_SyncAllocatorPresenter;
        break;
    case VIDRNDT_DS_NULL_COMP:
        break;
    case VIDRNDT_DS_NULL_UNCOMP:
        break;
    }

    CString SelAudioRenderer = s.SelectedAudioRenderer();
    if (SelAudioRenderer == AUDRNDT_NULL_COMP) {
        pFGF = new CFGFilterInternal<CNullAudioRenderer>(AUDRNDT_NULL_COMP, MERIT64_ABOVE_DSHOW + 2);
        pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_NULL);
        m_transform.AddTail(pFGF);
    } else if (SelAudioRenderer == AUDRNDT_NULL_UNCOMP) {
        pFGF = new CFGFilterInternal<CNullUAudioRenderer>(AUDRNDT_NULL_UNCOMP, MERIT64_ABOVE_DSHOW + 2);
        pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_NULL);
        m_transform.AddTail(pFGF);
    } else if (SelAudioRenderer == AUDRNDT_MPC) {
        pFGF = new CFGFilterInternal<CMpcAudioRenderer>(AUDRNDT_MPC, MERIT64_ABOVE_DSHOW + 2);
        pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_NULL);
        m_transform.AddTail(pFGF);
    } else if (!SelAudioRenderer.IsEmpty()) {
        pFGF = new CFGFilterRegistry(SelAudioRenderer, m_armerit);
        pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_NULL);
        m_transform.AddTail(pFGF);
    }

    CFGFilter* pFGF = FindFromFilterList(selVideoRenderer, m_transform);
    pFGF->SetMerit(m_vrmerit);

    pFGF = FindFromFilterList(selAudioRenderer, m_transform);
    pFGF->SetMerit(m_armerit);

    pFGF = FindFromFilterList(CLSID_VSFilter, m_transform);
    pFGF->SetMerit(m_vrmerit + 0x100);

    pFGF = FindFromFilterList(CLSID_AudioSwitcher, m_transform);
    pFGF->SetMerit(m_armerit + 0x100);
}

STDMETHODIMP CFGManagerPlayer::ConnectDirect(IPin* pPinOut, IPin* pPinIn, const AM_MEDIA_TYPE* pmt)
{
    CAutoLock cAutoLock(this);

    if (GetCLSID(pPinOut) == CLSID_MPEG2Demultiplexer) {
        CComQIPtr<IMediaSeeking> pMS = pPinOut;
        REFERENCE_TIME rtDur = 0;
        if (!pMS || FAILED(pMS->GetDuration(&rtDur)) || rtDur <= 0) {
            return E_FAIL;
        }
    }

    return __super::ConnectDirect(pPinOut, pPinIn, pmt);
}

//
// CFGManagerDVD
//

CFGManagerDVD::CFGManagerDVD(LPCTSTR pName, LPUNKNOWN pUnk, HWND hWnd)
: CFGManagerPlayer(pName, pUnk, hWnd)
{
    //const CPlayerSettings& s = CPlayerCore::m_settings;

    //// have to avoid the old video renderer
    //if (s.iDSVideoRendererType == VIDRNDT_DS_OLDRENDERER) {
    //    m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, CLSID_OverlayMixer, L"Overlay Mixer", m_vrmerit - 1));
    //}

    //// elecard's decoder isn't suited for dvd playback (atm)
    //m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{F50B3F13-19C4-11CF-AA9A-02608C9BABA2}")), MERIT64_DO_NOT_USE));
}

//class CResetDVD : public CDVDSession
//{
//public:
//    CResetDVD(LPCTSTR path) {
//        if (Open(path)) {
//            if (BeginSession()) {
//                Authenticate(); /*GetDiscKey();*/
//                EndSession();
//            }
//            Close();
//        }
//    }
//};

STDMETHODIMP CFGManagerDVD::RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList)
{
    CAutoLock cAutoLock(this);

    HRESULT hr;

    CComPtr<IBaseFilter> pBF;
    if (FAILED(hr = AddSourceFilter(lpcwstrFile, lpcwstrFile, &pBF))) {
        return hr;
    }

    return ConnectFilter(pBF, NULL);
}

STDMETHODIMP CFGManagerDVD::AddSourceFilter(LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppFilter)
{
    CAutoLock cAutoLock(this);

    CheckPointer(lpcwstrFileName, E_POINTER);
    CheckPointer(ppFilter, E_POINTER);

    HRESULT hr;

    CStringW fn = CStringW(lpcwstrFileName).TrimLeft();

    GUID clsid = CLSID_DVDNavigator;

    CComPtr<IBaseFilter> pBF;
    if (FAILED(hr = pBF.CoCreateInstance(clsid))
        || FAILED(hr = AddFilter(pBF, L"DVD Navigator"))) {
            return VFW_E_CANNOT_LOAD_SOURCE_FILTER;
    }

    CComQIPtr<IDvdControl2> pDVDC;
    CComQIPtr<IDvdInfo2> pDVDI;

    if (!((pDVDC = pBF) && (pDVDI = pBF))) {
        return E_NOINTERFACE;
    }

    WCHAR buff[MAX_PATH];
    ULONG len;
    if ((!fn.IsEmpty()
        && FAILED(hr = pDVDC->SetDVDDirectory(fn))
        && FAILED(hr = pDVDC->SetDVDDirectory(fn + L"VIDEO_TS"))
        && FAILED(hr = pDVDC->SetDVDDirectory(fn + L"\\VIDEO_TS")))
        || FAILED(hr = pDVDI->GetDVDDirectory(buff, _countof(buff), &len)) || len == 0) {
            return E_INVALIDARG;
    }

    pDVDC->SetOption(DVD_ResetOnStop, FALSE);
    pDVDC->SetOption(DVD_HMSF_TimeCodeEvents, TRUE);

    if (clsid == CLSID_DVDNavigator) {
        CResetDVD(CString(buff));
    }

    *ppFilter = pBF.Detach();

    return S_OK;
}

//
// CFGManagerCapture
//

CFGManagerCapture::CFGManagerCapture(LPCTSTR pName, LPUNKNOWN pUnk, HWND hWnd)
: CFGManagerPlayer(pName, pUnk, hWnd)
{
    //CFGFilter* pFGF = new CFGFilterInternal<CDeinterlacerFilter>(L"Deinterlacer", m_vrmerit + 0x100);
    //pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_NULL);
    //m_transform.AddTail(pFGF);

    //// Blacklist Morgan's Stream Switcher
    //m_transform.AddTail(new CFGFilterRegistry(CLSID_MorganStreamSwitcher, MERIT64_DO_NOT_USE));
}

//
// CFGManagerMuxer
//

CFGManagerMuxer::CFGManagerMuxer(LPCTSTR pName, LPUNKNOWN pUnk)
: CFGManagerCustom(pName, pUnk)
{
    //m_source.AddTail(new CFGFilterInternal<CSubtitleSourceASS>());
}

//
// CFGAggregator
//

CFGAggregator::CFGAggregator(const CLSID& clsid, LPCTSTR pName, LPUNKNOWN pUnk, HRESULT& hr)
: CUnknown(pName, pUnk)
{
    hr = m_pUnkInner.CoCreateInstance(clsid, GetOwner());
}

CFGAggregator::~CFGAggregator()
{
    m_pUnkInner.Release();
}

STDMETHODIMP CFGAggregator::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        m_pUnkInner && (riid != IID_IUnknown && SUCCEEDED(m_pUnkInner->QueryInterface(riid, ppv))) ? S_OK :
        __super::NonDelegatingQueryInterface(riid, ppv);
}
