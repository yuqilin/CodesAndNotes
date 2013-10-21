
#pragma once

#define MERIT64(merit)      (((UINT64)(merit)) << 16)
#define MERIT64_DO_NOT_USE  MERIT64(MERIT_DO_NOT_USE)
#define MERIT64_DO_USE      MERIT64(MERIT_DO_NOT_USE + 1)
#define MERIT64_UNLIKELY    (MERIT64(MERIT_UNLIKELY))
#define MERIT64_NORMAL      (MERIT64(MERIT_NORMAL))
#define MERIT64_PREFERRED   (MERIT64(MERIT_PREFERRED))
#define MERIT64_ABOVE_DSHOW (MERIT64(1) << 32)

enum CodecsCategory
{
    kCodecsCategoryUnknown = 0,
    kCodecsCategoryDSFilter,
    kCodecsCategoryDMO,
    kCodecsCategoryVFW,
};

enum CodecsType
{
    kCodecsTypeUnknown = 0,
    kCodecsTypeSourceFilter,
    kCodecsTypeSplitter,
    kCodecsTypeAudioEffect,
    kCodecsTypeVideoEffect,
    kCodecsTypeAudioRenderer,
    kCodecsTypeVideoRenderer,
    kCodecsTypeAudioDecoder,
    kCodecsTypeNullRenderer,
    kCodecsTypeVideoDecoder,
    kCodecsTypeAudioEncoder,
    kCodecsTypeVideoEncoder,
    kCodecsTypeMuxer,
    kCodecsTypeFileWriter,
};


struct PathFlagItem
{
    CString flag;
    CString path;
};

struct CheckByteItem
{
    CString checkbyte;
    CString subtype;
};

struct MediaTypeItem
{
    GUID majortype;
    GUID subtype;
};

struct CodecsInfo
{
    bool enable;
    DWORD priority;
    CString name;
    CString	pathflag;
    CString	path;
    CodecsCategory category;
    CString	catedata;
    CodecsType type;
    GUID clsid;
    DWORD merit;
    CAtlList<CString> protocols;
    CAtlList<CString> extensions;
    CAtlList<CString> depends;
    CAtlList<CString> preloads;
    CAtlList<CString> checkbytes;
    CAtlList<MediaTypeItem> mediatypes;

    CodecsInfo()
    {
        this->category = kCodecsCategoryUnknown;
        this->type = kCodecsTypeUnknown;
        this->clsid = GUID_NULL;
        //this->merit = MERIT64_DO_NOT_USE;
    }

    ~CodecsInfo()
    {
        protocols.RemoveAll();
        extensions.RemoveAll();
        depends.RemoveAll();
        preloads.RemoveAll();
        checkbytes.RemoveAll();
        mediatypes.RemoveAll();
    }
};


class CFGFilter
{
protected:
    CodecsInfo* m_info;

    UINT64  m_merit;

public:
    CFGFilter() : m_info(NULL) {}

    virtual ~CFGFilter() { m_info = NULL; }

    void SetCodecsInfo(CodecsInfo* info) {
        m_info = info;
    }

    GUID GetCLSID() {
        if (m_info)
            return m_info->clsid;
        return GUID_NULL;
    }

    void SetMerit(UINT64 merit) {
        m_merit = merit;
    }

    UINT64 GetMerit() {
        return m_merit;
    }

    bool CheckTypes(const CAtlArray<GUID>& types, bool fExactMatch);

    virtual HRESULT Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks) = 0;
};

class CFGFilterRegistry : public CFGFilter
{
public:
    CFGFilterRegistry() {}

    HRESULT Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
};

template<class T>
class CFGFilterInternal : public CFGFilter
{
public:
    CFGFilterInternal() {}

    HRESULT Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
    {
        CheckPointer(ppBF, E_POINTER);

        HRESULT hr = S_OK;
        CComPtr<IBaseFilter> pBF = new T(NULL, &hr);
        if (FAILED(hr))
        {
            return hr;
        }

        *ppBF = pBF.Detach();

        return hr;
    }
};

class CFGFilterFile : public CFGFilter
{
protected:
    HINSTANCE m_hInst;

public:
    CFGFilterFile() : m_hInst(NULL) {}

    HRESULT Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
};

class CFGFilterVideoRenderer : public CFGFilter
{
protected:
    HWND m_hWnd;

public:
    CFGFilterVideoRenderer() : m_hWnd(NULL) {}

    void SetVideoWindow(HWND hWnd) {
        m_hWnd = hWnd;
    }

    HRESULT Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
};

class CFGFilterList
{
    struct filter_t {
        int index;
        CFGFilter* pFGF;
        int group;
        bool exactmatch, autodelete;
    };
    static int filter_cmp(const void* a, const void* b);
    CAtlList<filter_t> m_filters;
    CAtlList<CFGFilter*> m_sortedfilters;

public:
    CFGFilterList();
    virtual ~CFGFilterList();

    bool IsEmpty() { return m_filters.IsEmpty(); }
    void RemoveAll();
    void Insert(CFGFilter* pFGF, int group, bool exactmatch = false, bool autodelete = true);

    POSITION GetHeadPosition();
    CFGFilter* GetNext(POSITION& pos);
};
