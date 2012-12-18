
#ifndef _PLAYERCORE_DSFILTERS_H_
#define _PLAYERCORE_DSFILTERS_H_

#define MERIT_DO_USE		(MERIT_DO_NOT_USE + 1)

enum FILTER_DEVICE
{
	FD_UNKNOWN,
	FD_FILTER,
	FD_DMO,
	FD_VFW,
	FD_IMAGE,
};

enum FILTER_TYPE
{
	FT_UNKNOWN,
	FT_SOURCE,
	FT_SPLITTER,
	FT_AUDIO_EFFECT,
	FT_VIDEO_EFFECT,	
	FT_AUDIO_RENDERER,
	FT_VIDEO_RENDERER,
	FT_NULL_RENDERER,
	FT_AUDIO_DECODER,
	FT_VIDEO_DECODER,
	FT_AUDIO_ENCODER,
	FT_VIDEO_ENCODER,
	FT_MUXER,
	FT_WRITER,
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

struct DSFilterInfo
{
	CLSID clsid;
	CString name;
	CString	pathflag;
	CString	path;
	DWORD merit;
	FILTER_DEVICE device;
	CString	devicedata;
	FILTER_TYPE	filtertype;
	CAtlList<CString> protocols;
	CAtlList<CString> extensions;
	CAtlList<CString> depends;
	CAtlList<CString> preloads;
	CAtlList<CheckByteItem> checkbytes;
	CAtlList<MediaTypeItem> mediatypes;

	DSFilterInfo()
	{
		this->name = _T("noname");
		this->device = FD_UNKNOWN;
		this->filtertype = FT_UNKNOWN;
		this->merit = MERIT_DO_NOT_USE;
	}
};

class CDSFilters
{
protected:
	CString m_strCodecsPath;
public:
	//enum {FILTER_SOURCE = 0, FILTER_TRNASFORM, FILTER_TOTAL};
	CAtlList<DSFilterInfo*>	m_source, m_transform/*, m_unknown*/;

public:
	CDSFilters();
	~CDSFilters();

	HRESULT SetCodecsPath(CString& strPath);
	LPCTSTR GetCodecsPath();

	DSFilterInfo* FindFilterByClsid(LPCTSTR pcszClsid);
	DSFilterInfo* FindInfoByName(LPCTSTR pcszName);

	HRESULT CreateCodec(DSFilterInfo* pInfo, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);

	BOOL	CheckTypes(DSFilterInfo* pInfo, const CAtlArray<GUID>& types, bool fExactMatch);

	void	OrderInfoByExtension(const TCHAR* pcszExtension);
	void	BringInfoToTop(CAtlList<DSFilterInfo*>& InfoList, POSITION& pos);

protected:
	//
	HRESULT	LoadCodecsInfo();
	void	UnLoadCodecsInfo();
	HRESULT ReplaceCodecsPath(CAtlList<DSFilterInfo*>& InfoList);

	void	OrderInfoByExtension(CAtlList<DSFilterInfo*>& InfoList, const TCHAR* pcszExtension);

	// Parse FilterInfo Xml
	HRESULT ParseInfoBuffer(TCHAR* pszBuffer);
	HRESULT	SetFilterInfo(DSFilterInfo* pInfo, LPCTSTR pcszKey, LPCTSTR pcszValue);
	HRESULT	SetFilterInfo(DSFilterInfo* pInfo, rapidxml::xml_node<TCHAR>* node);
	BOOL ParseFilterDevice(DSFilterInfo* pInfo, const TCHAR* pcszFilterDevice);
	BOOL ParseFilterType(DSFilterInfo* pInfo, const TCHAR* pcszFilterType);
	BOOL ParseCheckByte(DSFilterInfo* pInfo, const TCHAR* pcszCheckByte);
	
	// 
};


class CDSFilterList
{
    struct filter_t
	{
        int index;
        DSFilterInfo* pInfo;
        int group;
        bool exactmatch, autodelete;
    };
    static int filter_cmp(const void* a, const void* b);
    CAtlList<filter_t> m_filters;
    CAtlList<DSFilterInfo*> m_sortedfilters;

public:
    CDSFilterList();
    virtual ~CDSFilterList();

    bool IsEmpty() { return m_filters.IsEmpty(); }
    void RemoveAll();
    void Insert(DSFilterInfo* pInfo, int group, bool exactmatch = false, bool autodelete = true);

    POSITION GetHeadPosition();
    DSFilterInfo* GetNext(POSITION& pos);
};


#endif