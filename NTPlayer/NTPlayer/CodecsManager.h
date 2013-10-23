#ifndef __NTPLAYER_CODECS_MANAGER_H__
#define __NTPLAYER_CODECS_MANAGER_H__

#include <rapidxml/rapidxml.hpp>
#include "FGFilter.h"

typedef CAtlList<CodecsInfo*>       CodecsInfoList;

class CCodecsManager
{
    friend class CNTPlayer;


    bool m_fCodecsLoaded;
public:

    CCodecsManager();
    ~CCodecsManager();

    //HRESULT LoadCodecsFromPath(LPCTSTR lpszCodecsPath, bool fReload);
    HRESULT SetCodecsPath(LPCTSTR lpszCodecsPath);

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