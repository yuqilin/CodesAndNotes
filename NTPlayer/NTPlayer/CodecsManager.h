#ifndef __NTPLAYER_CODECS_MANAGER_H__
#define __NTPLAYER_CODECS_MANAGER_H__

#include <rapidxml/rapidxml.hpp>

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
    kCodecsTypeNullRenderer,
    kCodecsTypeAudioDecoder,
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
    CString majortype;
    CString subtype;
};

struct CodecsInfo
{
    CString clsid;
    CString name;
    CString	pathflag;
    CString	path;
    DWORD merit;
    CodecsCategory category;
    CString	catedata;
    CodecsType type;
    CAtlList<CString> protocols;
    CAtlList<CString> extensions;
    CAtlList<CString> depends;
    CAtlList<CString> preloads;
    CAtlList<CheckByteItem> checkbytes;
    CAtlList<MediaTypeItem> mediatypes;

    CodecsInfo()
    {
        this->name = _T("noname");
        this->category = kCodecsCategoryUnknown;
        this->type = kCodecsTypeUnknown;
        this->merit = MERIT_DO_NOT_USE;
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

typedef CAtlList<CodecsInfo*>       CodecsInfoList;

class CCodecsManager
{
public:
    CCodecsManager();
    ~CCodecsManager();

    HRESULT SetCodecsPath(LPCTSTR lpszCodecsPath);

    HRESULT LoadCodecsInfo();
    void    UnLoadCodecsInfo();

    const char* GetCodecsInfoString() {
        return m_strCodecsInfoString;
    }


protected:
    HRESULT ParseCodecsInfoConfig(LPCTSTR lpszCodecsInfoConfig);
    HRESULT	SetCodecsInfo(CodecsInfo* pInfo, LPCTSTR pcszKey, LPCTSTR pcszValue);
    HRESULT	SetCodecsInfo(CodecsInfo* pInfo, rapidxml::xml_node<TCHAR>* node);
    BOOL    ParseCodecsCategory(CodecsInfo* pInfo, const TCHAR* pcszCategory);
    BOOL    ParseCodecsType(CodecsInfo* pInfo, const TCHAR* pcszType);
    BOOL    ParseCheckByte(CodecsInfo* pInfo, const TCHAR* pcszValue);

    void    PrintCodecsInfo(const CodecsInfoList& InfoList, CString& strToPrint);


private:
    LPCTSTR CodecsCategoryToText(CodecsCategory cate);
    LPCTSTR CodecsTypeToText(CodecsType type);
    CodecsCategory CodecsCategoryFromText(LPCTSTR cate);
    CodecsType CodecsTypeFromText(LPCTSTR type);


private:
    CString m_strCodecsPath;

    CodecsInfoList m_listCodecsInfo;

    CStringA m_strCodecsInfoString;


};

#endif