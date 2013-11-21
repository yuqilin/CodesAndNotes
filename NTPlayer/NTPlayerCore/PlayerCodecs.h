#pragma once

#include "CodecsInfo.h"

class PlayerCodecs
{
public:
    PlayerCodecs();
    ~PlayerCodecs();

    void            SetCodecsPath(const char* path);
    HRESULT         LoadCodecs();
    void            FreeCodecs();

    CodecsInfoList& GetSources() {
        return m_source;
    }
    CodecsInfoList& GetTransforms() {
        return m_transform;
    }
//     CodecsInfoList& GetCodecsInfoList() {
//         return codecs_list_;
//     }
    void SortCodecsInfoList();

    CodecsInfo* FindCodecsInfo(const CString& clsid, CodecsType type);

    HRESULT CreateCodecsObject(CodecsInfo* info, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks, void* pParam = NULL);

protected:
    HRESULT ParseCodecsInfoConfig(const char* config);
    HRESULT SetCodecsPriority();

    HRESULT CreateRegCodecs(CodecsInfo* info, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks, void* pParam);
    HRESULT CreateVideoRenderer(CodecsInfo* info, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks, void* pParam);
    HRESULT CreateFileCodecs(CodecsInfo* info, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks, void* pParam);
    HRESULT CreateInnerCodecs(CodecsInfo* info, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks, void* pParam);

private:
    HRESULT SetCodecsInfo(CodecsInfo* info, const char* key, const char* value);
    HRESULT SetCodecsInfo(CodecsInfo* info, void* subnode);


    void    ConfigFFDShow(void* pffdshowbase, const TCHAR * pcszGUID);

protected:
    CString m_codecspath;
    //CodecsInfoList codecs_list_;
    CodecsInfoList m_source;
    CodecsInfoList m_transform;
    bool m_loaded;
    //HWND m_hVideoWindow;
};

//////////////////////////////////////////////////////////////////////////
template<class T>
class PlayerInnerFilter
{
public:
   static HRESULT Create(IBaseFilter** ppBF)
   {
       CheckPointer(ppBF, E_POINTER);

       HRESULT hr = S_OK;
       T* pBF = new T(&hr);
       if (pBF == NULL)
           return E_OUTOFMEMORY;
       if (FAILED(hr))
           return hr;
       pBF->AddRef();
       *ppBF = pBF;
       return hr;
   }
};