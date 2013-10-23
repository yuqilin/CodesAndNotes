#include "stdafx.h"
#include "CodecsManager.h"

extern HINSTANCE g_hInstance;

struct CodecsCategoryAll
{
    CodecsCategory category;
    const TCHAR* name;
};

struct CodecsTypeAll
{
    CodecsType type;
    const TCHAR* name;
};

static CodecsCategoryAll s_CodecsCategoryAll[] = {
    { kCodecsCategoryDSFilter, _T("dsfilter") },
    { kCodecsCategoryDMO, _T("dmo") },
    { kCodecsCategoryVFW, _T("vfw") },
};

static int s_CodecsCategoryAllCount = _countof(s_CodecsCategoryAll);

static CodecsTypeAll s_CodecsTypeAll[] = {
    { kCodecsTypeSourceFilter,  _T("source") },
    { kCodecsTypeSplitter,      _T("splitter") },
    { kCodecsTypeAudioEffect,   _T("audioeffect") },
    { kCodecsTypeVideoEffect,	_T("videoeffect") },
    { kCodecsTypeAudioRenderer, _T("audiorenderer") },
    { kCodecsTypeVideoRenderer, _T("videorenderer") },
    { kCodecsTypeNullRenderer,  _T("nullrenderer") },
    { kCodecsTypeAudioDecoder,  _T("audiodecoder") },
    { kCodecsTypeVideoDecoder,  _T("videodecoder") },
    { kCodecsTypeAudioEncoder,  _T("audioencoder") },
    { kCodecsTypeVideoEncoder,  _T("videoencoder") },
    { kCodecsTypeMuxer,         _T("muxer") },
    { kCodecsTypeFileWriter,    _T("filewriter") },
};

static int s_CodecsTypeAllCount = _countof(s_CodecsTypeAll);


//////////////////////////////////////////////////////////////////////////
CCodecsManager::CCodecsManager()
{
}

CCodecsManager::~CCodecsManager()
{
    UnLoadCodecsInfo();
}

HRESULT CCodecsManager::SetCodecsPath(LPCTSTR lpszCodecsPath)
{
    // TODO: check path is invalid

    if (!PathIsDirectory(lpszCodecsPath) || !PathFileExists(lpszCodecsPath))
    {
        return E_INVALIDARG;
    }

    CString path(lpszCodecsPath);
    if (!m_strCodecsPath.IsEmpty() &&
        !path.IsEmpty() &&
        m_strCodecsPath != path)
    {
        POSITION pos = m_CodecsInfoList.GetHeadPosition();
        while (pos)
        {
            CodecsInfo* pInfo = m_CodecsInfoList.GetNext(pos);
            if (pInfo)
            {
                pInfo->path.Replace(m_strCodecsPath, path);
            }
        }
        m_strCodecsPath = path;
    }

    return S_OK;
}

HRESULT CCodecsManager::LoadCodecsInfo()
{
    if (m_fCodecsLoaded)
    {
        return S_OK;
    }

    UnLoadCodecsInfo();

    HRESULT hr = E_FAIL;
    TCHAR * pText = LoadResourceText(g_hInstance, _T("TEXT"), IDR_CODECSINFO);
    if(pText != NULL)
    {
        hr = ParseCodecsInfoConfig(pText);
        if (SUCCEEDED(hr))
        {
            m_fCodecsLoaded = true;
        }
        ::FreeResourceText(pText);
    }
    return hr;
}

void CCodecsManager::UnLoadCodecsInfo()
{
    const CodecsInfo* info = NULL;
    POSITION pos = m_CodecsInfoList.GetHeadPosition();
    while (pos)
    {
        info = m_CodecsInfoList.GetNext(pos);
        if (info)
        {
            SAFE_DELETE(info);
        }
    }
    m_CodecsInfoList.RemoveAll();

    m_fCodecsLoaded = false;
}

void CCodecsManager::PrintCodecsInfo(const CodecsInfoList& InfoList, CString& strToPrint)
{
    strToPrint.Empty();

    int count = 0;
    POSITION pos = InfoList.GetHeadPosition();
    while (pos)
    {
        const CodecsInfo* info = InfoList.GetNext(pos);
        if (info != NULL)
        {
            CString temp;
            temp.Format(
                _T("CodecsInfo[%d]:\r\n")
                _T("clsid=%s\r\n")
                _T("name=%s\r\n")
                _T("pathflag=%s\r\n")
                _T("path=%s\r\n")
                _T("merit=0x%08x\r\n")
                _T("category=%s\r\n")
                _T("catedata=%s\r\n")
                _T("type=%s\r\n"),
                count,
                info->clsid,
                info->name,
                info->pathflag,
                info->path,
                info->merit,
                CodecsCategoryToText(info->category),
                info->catedata,
                CodecsTypeToText(info->type)
                );

            strToPrint += temp;

            strToPrint += _T("protocols=");
            POSITION pos0 = info->protocols.GetHeadPosition();
            while (pos0)
            {
                const CString& protocol = info->protocols.GetNext(pos0);
                strToPrint += protocol;
                strToPrint += _T(";");
            }
            strToPrint += _T("\r\n");
            
            strToPrint += _T("extensions=");
            pos0 = info->extensions.GetHeadPosition();
            while (pos0)
            {
                const CString& extension = info->extensions.GetNext(pos0);
                strToPrint += extension;
                strToPrint += _T(";");
            }
            strToPrint += _T("\r\n");

            strToPrint += _T("depends=");
            pos0 = info->depends.GetHeadPosition();
            while (pos0)
            {
                const CString& depend = info->depends.GetNext(pos0);
                strToPrint += depend;
                strToPrint += _T(";");
            }
            strToPrint += _T("\r\n");

            strToPrint += _T("preloads=");
            pos0 = info->depends.GetHeadPosition();
            while (pos0)
            {
                const CString& preload = info->depends.GetNext(pos0);
                strToPrint += preload;
                strToPrint += _T(";");
            }
            strToPrint += _T("\r\n");

            pos0 = info->checkbytes.GetHeadPosition();
            while (pos0)
            {
                strToPrint += _T("checkbytes=");
                const CString& chkbytes = info->checkbytes.GetNext(pos0);
                strToPrint += chkbytes;
                strToPrint += _T("\r\n");
            }

            pos0 = info->mediatypes.GetHeadPosition();
            while (pos0)
            {
                strToPrint += _T("mediatypes=");
                const MediaTypeItem& item = info->mediatypes.GetNext(pos0);
                strToPrint += _T("majortype=");
                strToPrint += CStringFromGUID(item.majortype);
                strToPrint += _T(";subtype=");
                strToPrint += CStringFromGUID(item.subtype);
                strToPrint += _T("\r\n");
            }

            strToPrint += _T("\r\n");
        }

        ++count;
    }
}

HRESULT CCodecsManager::ParseCodecsInfoConfig(LPCTSTR lpszCodecsInfoConfig)
{
    HRESULT hr = S_OK;

    rapidxml::xml_document<TCHAR> doc;

    try
    {
        doc.parse<0>((TCHAR*)lpszCodecsInfoConfig);
    }
    catch(rapidxml::parse_error& error)
    {
        CString strError(error.what());
        //g_utility.Log(_T("RapidXml got parse error:%s"), strError);
        return FALSE;
    }

    rapidxml::xml_node<TCHAR>* node = doc.first_node();
    if (node)
        node = node->first_node(_T("codecs"));
    for(; node!=NULL; node=node->next_sibling())
    {
        if (_tcsicmp(node->name(), _T("codecs")) != 0)
        {
            continue;
        }

        CodecsInfo* pInfo  = new CodecsInfo();
        if (pInfo == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        rapidxml::xml_attribute<TCHAR>* attr = node->first_attribute();
        for(; attr!=NULL; attr=attr->next_attribute())
        {
            SetCodecsInfo(pInfo, attr->name(), attr->value());
        }

        rapidxml::xml_node<TCHAR>* subnode = node->first_node();
        for (; subnode!=NULL; subnode=subnode->next_sibling())
        {
            SetCodecsInfo(pInfo, subnode);
        }

        m_CodecsInfoList.AddTail(pInfo);
    }
    return hr;
}

HRESULT	CCodecsManager::SetCodecsInfo(CodecsInfo* pInfo, LPCTSTR pcszKey, LPCTSTR pcszValue)
{
    CheckPointer(pcszKey, E_POINTER);
    CheckPointer(pcszValue, E_POINTER);

    HRESULT hr = S_OK;

    if(_tcsicmp(pcszKey, _T("enable")) == 0)
    {
        pInfo->enable = (bool)_tcstoul(pcszValue, NULL, 10);
    }
    else if (_tcsicmp(pcszKey, _T("priority")) == 0)
    {
        pInfo->priority = _tcstoul(pcszValue, NULL, 10);
    }
    else if(_tcsicmp(pcszKey, _T("name")) == 0)
    {
        pInfo->name = pcszValue;
    }
    else if(_tcsicmp(pcszKey, _T("pathflag")) == 0)
    {
        pInfo->pathflag = pcszValue;
    }
    else if(_tcsicmp(pcszKey, _T("path")) == 0)
    {
        pInfo->path = pcszValue;
    }
    else if(_tcsicmp(pcszKey, _T("clsid")) == 0)
    {
        pInfo->clsid = GUIDFromCString(pcszValue);
    }
    else if(_tcsicmp(pcszKey, _T("category")) == 0)
    {
        this->ParseCodecsCategory(pInfo, pcszValue);
    }
    else if(_tcsicmp(pcszKey, _T("type")) == 0)
    {
        this->ParseCodecsType(pInfo, pcszValue);
    }
    else if(_tcsicmp(pcszKey, _T("catedata")) == 0)
    {
        pInfo->catedata = pcszValue;
    }
    else if(_tcsicmp(pcszKey, _T("merit")) == 0)
    {
        pInfo->merit = _tcstoul(pcszValue, NULL, 16);
    }
    else if(_tcsicmp(pcszKey, _T("checkbyte")) == 0)
    {
        //this->ParseCheckByte(pInfo, pcszValue);
        CString checkbyte(pcszValue);
        if (checkbyte.GetLength() > 0)
            pInfo->checkbytes.AddTail(checkbyte);
    }
    else
    {
        //g_utility.Log(_T("Unknown filter info item:%s"), pcszKey);
        assert(0);
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT	CCodecsManager::SetCodecsInfo(CodecsInfo* pInfo, rapidxml::xml_node<TCHAR>* node)
{
    CheckPointer(node, E_POINTER);

    HRESULT hr = S_OK;

    const TCHAR* pcszKey = node->name();

    if (_tcsicmp(pcszKey, _T("protocol")) == 0)
    {
        rapidxml::xml_attribute<TCHAR>* attr=node->first_attribute(_T("value"));
        if (attr)
        {
            pInfo->protocols.AddTail(attr->value());
        }
    }
    else if (_tcsicmp(pcszKey, _T("extname")) == 0)
    {
        rapidxml::xml_attribute<TCHAR>* attr=node->first_attribute(_T("value"));
        if (attr)
        {
            pInfo->extensions.AddTail(attr->value());
        }
    }
    else if (_tcsicmp(pcszKey, _T("checkbyte")) == 0)
    {
        rapidxml::xml_attribute<TCHAR>* attr=node->first_attribute(_T("value"));
        if (attr)
        {
            ParseCheckByte(pInfo, attr->value());
        }
    }
    else if (_tcsicmp(pcszKey, _T("depend")) == 0)
    {
        rapidxml::xml_attribute<TCHAR>* attr=node->first_attribute(_T("dll"));
        if (attr)
        {
            CString dependpath;
            dependpath.Format(_T("%s\\%s"), m_strCodecsPath, attr->value());
            pInfo->depends.AddTail(dependpath);
        }
    }
    else if (_tcsicmp(pcszKey, _T("mediatype")) == 0)
    {
        rapidxml::xml_attribute<TCHAR>* major=node->first_attribute(_T("major"));
        rapidxml::xml_attribute<TCHAR>* sub=node->first_attribute(_T("sub"));
        if (major!=NULL && sub!=NULL)
        {
            CString majortype = major->value();
            CString subtype = sub->value();

            MediaTypeItem mediatype;
            mediatype.majortype = GUIDFromCString(majortype);
            mediatype.subtype = GUIDFromCString(subtype);
            pInfo->mediatypes.AddTail(mediatype);
        }
    }
    else if (_tcsicmp(pcszKey, _T("preload")) == 0)
    {
        rapidxml::xml_attribute<TCHAR>* attr=node->first_attribute(_T("dll"));
        if (attr)
        {
            CString preloadpath;
            preloadpath.Format(_T("%s\\%s"), m_strCodecsPath, attr->value());
            pInfo->preloads.AddTail(preloadpath);
        }
    }
    else
    {
        //g_utility.Log(_T("Unknown filter info item:%s"), pcszKey);
        assert(0);
        hr = E_INVALIDARG;
    }

    return hr;
}

BOOL CCodecsManager::ParseCodecsCategory(CodecsInfo* pInfo, const TCHAR* pcszCategory)
{
    if (pInfo != NULL)
    {
        pInfo->category = CodecsCategoryFromText(pcszCategory);
    }
    return TRUE;
}

BOOL CCodecsManager::ParseCodecsType(CodecsInfo* pInfo, const TCHAR* pcszType)
{
    if (pInfo != NULL)
    {
        pInfo->type = CodecsTypeFromText(pcszType);
    }
    return TRUE;
}

BOOL CCodecsManager::ParseCheckByte(CodecsInfo* pInfo, const TCHAR* pcszValue)
{
    CheckPointer(pcszValue, FALSE);

    //CheckByteItem item;
    CString checkbytes(pcszValue);		
//     int i = checkbytes.Find(_T("|"));
//     if(i > 0)
//     {
//         item.checkbyte = checkbytes.Mid(0, i);
//         item.subtype = checkbytes.Mid(i+1);
//     }
//     else
//     {
//         item.checkbyte = checkbytes;
//     }
//     pInfo->checkbytes.AddTail(item);
    pInfo->checkbytes.AddTail(checkbytes);
    return TRUE;
}

LPCTSTR CCodecsManager::CodecsCategoryToText(CodecsCategory cate)
{
    for (int i=0; i<s_CodecsCategoryAllCount; ++i)
    {
        if (s_CodecsCategoryAll[i].category == cate)
        {
            return s_CodecsCategoryAll[i].name;
        }
    }
    return _T("UNKNOWN");
}

LPCTSTR CCodecsManager::CodecsTypeToText(CodecsType type)
{
    for (int i=0; i<s_CodecsTypeAllCount; ++i)
    {
        if (s_CodecsTypeAll[i].type == type)
        {
            return s_CodecsTypeAll[i].name;
        }
    }
    return _T("UNKNOWN");
}

CodecsCategory CCodecsManager::CodecsCategoryFromText(LPCTSTR cate)
{
    for (int i=0; i<s_CodecsCategoryAllCount; ++i)
    {
        if (_tcsicmp(s_CodecsCategoryAll[i].name, cate) == 0)
        {
            return s_CodecsCategoryAll[i].category;
        }
    }
    return kCodecsCategoryUnknown;
}

CodecsType CCodecsManager::CodecsTypeFromText(LPCTSTR type)
{
    for (int i=0; i<s_CodecsTypeAllCount; ++i)
    {
        if (_tcsicmp(s_CodecsTypeAll[i].name, type) == 0)
        {
            return s_CodecsTypeAll[i].type;
        }
    }
    return kCodecsTypeUnknown;
}

//CFGFilter* FGFilterFromCodecsInfo(CodecsInfo* pInfo)
//{
//    CFGFilter* pFGF = NULL;
//    if (pInfo->pathflag == "reg")
//    {
//        pFGF = new CFGFilterRegistry(pInfo);
//    }
//    else if (pInfo->pathflag == "file")
//    {
//        if (pFGF)
//            pFGF = new CFGFilterFile(pInfo);
//    }
//
//    return pFGF;
//}