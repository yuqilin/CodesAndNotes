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

    HRESULT CreateCodecsObject(CodecsInfo* info, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);

protected:
    HRESULT ParseCodecsInfoConfig(const char* config);
    HRESULT SetCodecsPriority();

    HRESULT CreateRegCodecs(CodecsInfo* info, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
    HRESULT CreateVideoRenderer(CodecsInfo* info, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
    HRESULT CreateFileCodecs(CodecsInfo* info, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
    HRESULT CreateInnerCodecs(CodecsInfo* info, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);

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
    HWND m_hVideoWindow;
};

