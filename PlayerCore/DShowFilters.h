
#ifndef _DSHOW_FILTERS_H_
#define _DSHOW_FILTERS_H_

#include "rapidxml/rapidxml.hpp"

#define MAX_GUID_LENGTH				40
#define MAX_EXTNAME_LENGTH			32
#define MAX_FILTERNAME_LENGTH		64

#define MAX_PATHFLAG_LENGTH			64
#define MAX_PATHFLAG_COUNT			32
#define MAX_DSFILTER_INFO_COUNT		256
#define MAX_SOURCEFILTER_COUNT		64
#define MAX_TRANSFORMFILTER_COUNT	128
#define MAX_PLAYER_FILTER_COUNT		8
#define MAX_SECTION_COUNT			6
#define MAX_CHECKBYTE_COUNT			64
#define MAX_PROTOCOL_LENGTH			32
#define MAX_PROTOCOL_COUNT			5
#define MAX_NETSOURCE_COUNT			16
#define MAX_MEDIATYPE_COUNT			256
#define MAX_EXTNAME_COUNT			16
#define MAX_DEPENDENCE_COUNT		32
#define MAX_DEPENDENCE_PATH			8
#define MAX_PRELOADDLL_COUNT		32

enum EFilterDevice
{
	kFilterDeviceUnknown,
	kFilterDeviceFilter,
	kFilterDeviceDmo,
	kFilterDeviceVfw,
	kFilterDeviceImage,
};

enum EFilterType
{
	kFilterTypeUnknown,
	kFilterTypeSource,
	kFilterTypeSplitter,
	kFilterTypeAudioEffect,
	kFilterTypeVideoEffect,
	kFilterTypeAudioRenderer,
	kFilterTypeVideoRenderer,
	kFilterTypeNullRenderer,
	kFilterTypeAudioDecoder,
	kFilterTypeVideoDecoder,
	kFilterTypeAudioEncoder,
	kFilterTypeVideoEncoder,
	kFilterTypeMuxer,
	kFilterTypeWriter,
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
	CString name;
	CString pathflag;
	CString path;
	CString clsid;
	EFilterDevice device;
	CString devicedata;
	EFilterType filtertype;
	DWORD merit;
	CAtlList<CString> protocols;
	CAtlList<CString> extensions;
	CAtlList<CString> dependences;
	CAtlList<CString> preloaddlls;
	CAtlList<CheckByteItem> checkbytes;
	CAtlList<MediaTypeItem> mediatypes;
	CAtlList<MediaTypeItem> excludetypes;
	BOOL extlimit;
	BOOL disabled;
};

class CDShowFilters
{
public:
	CDShowFilters();
	~CDShowFilters();

	HRESULT				Initialize(void);
	void				UnInitialize(void);

	// Properties
	LPCTSTR				GetCodecsPath(void);
	BOOL				SetCodecsPath(const TCHAR* pcszPath);
	int					GetCount(void);

	DShowFilterInfo*	FindInfoByClsid(const TCHAR* pcszClsId);
	DShowFilterInfo*	FindInfoByName(const TCHAR* pcszName);

	BOOL				IsSupportsMediaTypes(DShowFilterInfo* pInfo, AM_MEDIA_TYPE** pmts, int nMediaTypeCount);

	BOOL				GetDisableList(TCHAR* pClsIdList, int nListSize);
	BOOL				SetDisableList(LPCTSTR pClsIdList);

	// Codecs helper
	BOOL				IsCodecExists(LPCTSTR pcszClsId);
	BOOL				IsCodecExists(DShowFilterInfo* pInfo);
	BOOL				DownloadCodec(DShowFilterInfo* pInfo);

	// Create instance
	HRESULT				CreateCodec(DShowFilterInfo* pInfo, REFIID riid, LPUNKNOWN pUnkOuter, LPVOID* ppv);
	HRESULT				CreateCodec(LPCTSTR pcszClsId, REFIID riid, LPUNKNOWN pUnkOuter, LPVOID* ppv);
	HRESULT				CreateFilter(DShowFilterInfo* pInfo, IBaseFilter** ppBaseFilter);
	HRESULT				CreateFilter(LPCTSTR pcszClsId, IBaseFilter** ppBaseFilter);
	BOOL				InstallVfwDriver(DShowFilterInfo* pInfo);

	HRESULT				CreateInstanceFromFile(const TCHAR* pcszPath,
											REFCLSID rclsid,
											REFIID riid,
											IUnknown* pUnkOuter,
											LPVOID* ppv);

	// Attach & detach PlayCore
	void				AttachPlayerCore(CZPlayerCore* pCore);
	void				DetachPlayerCore(CZPlayerCore* pCore);

	
	// Sort Info
	void				SortInfoByExtension(LPCTSTR pcszExtension);
	BOOL				BringInfoToTop(DShowFilterInfo* pInfo);

	CAtlList<DShowFilterInfo*>& GetInfoList() { return m_InfoList; }

protected:
	// API Hooks
	static HRESULT WINAPI	MyCoCreateInstance(REFCLSID rclsid,
		LPUNKNOWN pUnkOuter,
		DWORD dwClsContext,
		REFIID riid,
		LPVOID * ppv);
	static BOOL WINAPI		MyShell_NotifyIconA(DWORD dwMessage, PNOTIFYICONDATAA lpData);
	static BOOL WINAPI		MyShell_NotifyIconW(DWORD dwMessage, PNOTIFYICONDATAW lpData);

	// Generic functions
	void					SetMyCurrentDirectory(void);
	void					RestoreOldCurrentDirectory(void);

	// Path flag functions
	void					ClearPathFlags(void);
	BOOL					SetPathFlags(void);
	BOOL					AddPathFlag(LPCTSTR pcszFlag, LPCTSTR pcszPath);
	BOOL					ExpandPathString(LPCTSTR pcszPathFlag, TCHAR* pszPath);

	// Codec info functions
	void					ClearInfo(void);
	BOOL					LoadInfo(void);
	BOOL					ParseInfoBuffer(LPCTSTR pcszInfoBuffer);
	DShowFilterInfo*		NewInfo(void);
	BOOL					SetInfo(DShowFilterInfo* pInfo, LPCTSTR pcszKey, LPCTSTR pcszValue);
	BOOL					SetInfo(DShowFilterInfo* pInfo, rapidxml::xml_node<TCHAR>* node);
	BOOL					ParseFilterDevice(DShowFilterInfo* pInfo, LPCTSTR pcszFilterDevice);
	BOOL					ParseFilterType(DShowFilterInfo* pInfo, LPCTSTR pcszFilterType);
	BOOL					ParseCheckByte(DShowFilterInfo* pInfo, LPCTSTR pcszCheckBytes);
	BYTE*					BuildHexStringBytes(const TCHAR* pcszHexString);
	void					OrderInfoByMerit(void);

	// Create objects
	HRESULT					NewCodec(DShowFilterInfo* pInfo, REFIID riid, LPUNKNOWN pUnkOuter, LPVOID* ppv);
	HRESULT					CreateInternalFilter(REFCLSID rclsid, IBaseFilter** ppFilter);
	BOOL					CreateFFDShowInstance(IClassFactory* pCF,
												REFCLSID rclsid,
												REFIID riid,
												LPVOID* ppv,
												HRESULT* phr);
	void					ConfigFFDShow(IffDShowBase* pffdshow, LPCTSTR pcszGUID);
	BOOL					LoadPreloadDlls(DShowFilterInfo* pInfo);
	HRESULT					ConfigTypeData(LPVOID pv, DShowFilterInfo* pInfo);

	// Vfw functions
	BOOL					InitializeAllVfwDrivers(void);
	BOOL					UninstallAllVfwDrivers(void);

	// Parse type-data functions
	BOOL					ParseDeviceData_DMO(LPCTSTR pcszData, CLSID* pDmoClsId);
	BOOL					ParseDeviceData_VFW(LPCTSTR pcszData, DWORD* pfccType, DWORD* pfccHandler);

protected:
	// Generic variables
	BOOL						m_bInitialized;
	CString						m_strCodecsPath;
	CString						m_strOldCurrentDirectory;
	CSubclassHelper				m_shPlayCores;
	CRITICAL_SECTION			m_csPlayCores;
	CAtlList<PathFlagItem>		m_PathFlags;
	CAtlList<SFilterInfo*>		m_InfoList;
};


#endif