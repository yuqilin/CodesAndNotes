#ifndef __NTPLAYER_CODECS_MANAGER_H__
#define __NTPLAYER_CODECS_MANAGER_H__

#include <rapidxml/rapidxml.hpp>

typedef CAtlList<CodecsInfo*>       CodecsInfoList;

class CCodecsInfoList
{
    struct codecs_t {
        int index;
        CodecsInfo* info;
        int group;
        bool exactmatch, autodelete;
    };

    static int codecs_cmp(const void* a, const void* b);
    CAtlList<codecs_t> m_codecs;
    CAtlList<CodecsInfo*> m_sortedcodecs;

public:
    CCodecsInfoList();
    virtual ~CCodecsInfoList();

    bool IsEmpty() { return m_codecs.IsEmpty(); }
    void RemoveAll();
    void Insert(CodecsInfo* pInfo, int group, bool exactmatch = false, bool autodelete = true);

    POSITION GetHeadPosition();
    CodecsInfo* GetNext(POSITION& pos);
};

class CCodecsManager
{
    friend class CNTPlayer;


    bool m_fCodecsLoaded;
public:

    CCodecsManager();
    ~CCodecsManager();

    HRESULT LoadCodecsFromPath(LPCTSTR lpszCodecsPath, bool fReload);

    HRESULT LoadCodecsInfo();
    void    UnLoadCodecsInfo();

    const CodecsInfoList& GetCodecsInfoList() {
        return m_CodecsInfoList;
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

    CodecsInfoList m_CodecsInfoList;
};

#endif