#include "stdafx.h"
#include "rapidxml/rapidxml.hpp"
#include "PlayerCodecs.h"
#include "PlayerCore.h"
#include "../filters/renderer/VideoRenderers/AllocatorCommon7.h"
#include "../filters/renderer/VideoRenderers/AllocatorCommon.h"
#include "ffdshow.h"
#include "PlayerAsyncReader.h"
#include "BaseVideoRenderer.h"

//////////////////////////////////////////////////////////////////////////
struct CodecsCategoryAll
{
    CodecsCategory category;
    const char* name;
};

struct CodecsTypeAll
{
    CodecsType type;
    const char* name;
};

struct CodecsPathFlagAll
{
    CodecsPathFlag flag;
    const char* name;
};

//////////////////////////////////////////////////////////////////////////
static CodecsCategoryAll s_CodecsCategoryAll[] = {
    { kCodecsCategoryDSFilter, "dsfilter" },
    { kCodecsCategoryDMO, "dmo" },
    { kCodecsCategoryVFW, "vfw" },
};

static CodecsTypeAll s_CodecsTypeAll[] = {
    { kCodecsTypeSourceFilter,  "src" },
    { kCodecsTypeSplitter,      "splt" },
    { kCodecsTypeAudioEffect,   "aeffect" },
    { kCodecsTypeVideoEffect,	"veffect" },
    { kCodecsTypeAudioRenderer, "arender" },
    { kCodecsTypeVideoRenderer, "vrender" },
    { kCodecsTypeNullRenderer,  "nullrender" },
    { kCodecsTypeAudioDecoder,  "adec" },
    { kCodecsTypeVideoDecoder,  "vdec" },
//     { kCodecsTypeAudioEncoder,  "aenc" },
//     { kCodecsTypeVideoEncoder,  "venc" },
//     { kCodecsTypeMuxer,         "muxer" },
//     { kCodecsTypeFileWriter,    "filewriter" },
};

static CodecsPathFlagAll s_CodecsPathFlagAll[] = {
    { kCodecsPathFlagReg,       "reg" },
    { kCodecsPathFlagFile,      "file" },
    { kCodecsPathFlagInner,     "inner" },
};

// static int s_CodecsCategoryAllCount = _countof(s_CodecsCategoryAll);
// static int s_CodecsTypeAllCount = _countof(s_CodecsTypeAll);
// static int s_CodecsPathFlagAllCount = _countof(s_CodecsPathFlagAll);


//////////////////////////////////////////////////////////////////////////
static CodecsCategory CodecsCategoryFromText(const char* cate);
static CodecsType CodecsTypeFromText(const char* type);
static CodecsPathFlag CodecsPathFlagFromText(const char* flag);

//////////////////////////////////////////////////////////////////////////
PlayerCodecs::PlayerCodecs()
: m_bLoaded(false)
{

}

PlayerCodecs::~PlayerCodecs()
{

}

void PlayerCodecs::SetCodecsPath(const char* path)
{
    std::wstring wpath = mbs2wcs(CP_UTF8, path);
    if (PathIsDirectory(wpath.c_str()) && PathFileExists(wpath.c_str()))
    {
        m_strCodecsPath = wpath.c_str();
    }
}

CString PlayerCodecs::GetCodecsPath()
{
    return m_strCodecsPath;
}


HRESULT PlayerCodecs::LoadCodecs()
{
    if (m_bLoaded)
    {
        player_log(kLogLevelTrace, _T("PlayerCodecs::LoadCodecs, already loaded"));
        return S_OK;
    }

    HRESULT hr = E_FAIL;

    CStringA text;
    bool rcloaded = LoadResource(IDR_CODECSINFO, text, _T("TEXT"));
    if (rcloaded)
    {
        hr = ParseCodecsInfoConfig(text);
    }

    if (SUCCEEDED(hr))
    {
        player_log(kLogLevelTrace, _T("PlayerCodecs::LoadCodecs, loaded ok"));
        m_bLoaded = true;
    }

    if (m_strCodecsPath.IsEmpty())
    {
        TCHAR szCodecsPath[MAX_PATH] = {0};
        ::GetModuleFileName(g_hInstance, szCodecsPath, MAX_PATH);
        ::PathRemoveFileSpec(szCodecsPath);
        ::PathAppend(szCodecsPath, _T("codecs"));

        m_strCodecsPath = szCodecsPath;
    }

    //LoadPreloadCodecs();

    return hr;
}

void PlayerCodecs::FreeCodecs()
{
    player_log(kLogLevelTrace, _T("PlayerCodecs::FreeCodecs"));

    CodecsInfo* info = NULL;
    POSITION pos = m_CodecsList.GetHeadPosition();
    while (pos)
    {
        info = m_CodecsList.GetNext(pos);
        if (info)
        {
            SAFE_DELETE(info);
        }
    }
    
    m_CodecsList.RemoveAll();
    m_bLoaded = false;

    UnloadExternalObjects();
}

HRESULT PlayerCodecs::ChangeCurrentDirectory()
{
    if (!m_strOldCurrentDirectory.IsEmpty())
        return S_OK;

    TCHAR szPath[MAX_PATH];
    DWORD dwResult = ::GetCurrentDirectory(MAX_PATH-1, szPath);
    if(dwResult > 0)
    {
        m_strOldCurrentDirectory = szPath;
        BOOL bOK = ::SetCurrentDirectory(m_strCodecsPath);

        player_log(kLogLevelTrace, _T("Change current directory, %s => %s, result = %d"),
            m_strOldCurrentDirectory, m_strCodecsPath, bOK);
    }
    return S_OK;
}

HRESULT PlayerCodecs::ResotreCurrentDirectory()
{
    if(m_strOldCurrentDirectory.IsEmpty())
        return S_OK;

    TCHAR szPath[MAX_PATH];
    DWORD dwResult = ::GetCurrentDirectory(MAX_PATH-1, szPath);
    BOOL bOK = ::SetCurrentDirectory(m_strOldCurrentDirectory);

    player_log(kLogLevelTrace, _T("Restore current directory, %s => %s, result = %d"),
        dwResult>0 ? szPath : _T(""), m_strOldCurrentDirectory, bOK);

    m_strOldCurrentDirectory.Empty();

    return S_OK;
}

HRESULT PlayerCodecs::CreateCodecsObject(CodecsInfo* info,
                                         IBaseFilter** ppBF,
                                         CInterfaceList<IUnknown, &IID_IUnknown>& pUnks,
                                         void* pParam)
{
    CheckPointer(info, E_POINTER);

    HRESULT hr = E_FAIL;

    if (info->type == kCodecsTypeVideoRenderer)
    {
        hr = CreateVideoRenderer(info, ppBF, pUnks, pParam);
        return hr;
    }
    else if (info->pathflag == kCodecsPathFlagReg)
    {
        hr = CreateRegCodecs(info, ppBF, pUnks, pParam);
    }
    else if (info->pathflag == kCodecsPathFlagFile)
    {
        hr = CreateFileCodecs(info, ppBF, pUnks, pParam);
    }
    else if (info->pathflag == kCodecsPathFlagInner)
    {
        hr = CreateInnerCodecs(info, ppBF, pUnks, pParam);
    }

    if (SUCCEEDED(hr))
    {
        player_log(kLogLevelTrace, _T("PlayerCodecs::CreateCodecsObject, create filter '%s' succeeded"), info->name);
    }
    else
    {
        player_log(kLogLevelTrace, _T("PlayerCodecs::CreateCodecsObject, create filter '%s' failed, hr = 0x%08x"), info->name, hr);
    }

    return hr;
}

HRESULT PlayerCodecs::CreateRegCodecs(CodecsInfo* info, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks, void* pParam)
{
    HRESULT hr = E_FAIL;

    // audio render

    if (info->clsid != GUID_NULL)
    {
        CComQIPtr<IBaseFilter> pBF;

        REFCLSID clsid = info->clsid;
        if (FAILED(hr = pBF.CoCreateInstance(clsid)))
        {
            player_log(kLogLevelTrace, _T("PlayerCodecs::CreateRegCodecs, CoCreateInstance for %s failed, hr = 0x%08X"),
                CStringFromGUID(info->clsid), hr);
            return hr;
        }

        *ppBF = pBF.Detach();

        hr = S_OK;
    }
    else if (info->name.GetLength() > 0)
    {
        CComPtr<IBindCtx> pBC;
        ::CreateBindCtx(0, &pBC);

        CComPtr<IMoniker> pMoniker;

        ULONG chEaten;
        if (S_OK != ::MkParseDisplayName(pBC, CComBSTR(info->name), &chEaten, &pMoniker)) {
            player_log(kLogLevelTrace, _T("PlayerCodecs::CreateRegCodecs, MkParseDisplayName failed"));
            hr = E_FAIL;
        }
        else if (SUCCEEDED(hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)ppBF))) {
            //info->clsid = CStringFromGUID(GetCLSID(*ppBF));
        }
    }

    return hr;
}

HRESULT PlayerCodecs::CreateVideoRenderer(CodecsInfo* info,
                                          IBaseFilter** ppBF,
                                          CInterfaceList<IUnknown, &IID_IUnknown>& pUnks,
                                          void* pParam)
{
    HRESULT hr = E_FAIL;

    //GUID clsid = GUIDFromCString(info->clsid);
    REFCLSID clsid = info->clsid;

    ///*
    if (clsid == CLSID_EnhancedVideoRenderer ||
        clsid == CLSID_EVRAllocatorPresenter)
    {
        BaseVideoRenderer* pVR = (BaseVideoRenderer*)pParam;
        if (pVR)
        {
            hr = pVR->CreateRenderer(ppBF);
        }
    }
//     else
//     {
//         CComPtr<IBaseFilter> pBF;
//         if (SUCCEEDED(hr = pBF.CoCreateInstance(clsid)))
//         {
//             *ppBF = pBF.Detach();
//         }
//         else
//         {
//             player_log(kLogLevelError, _T("CoCreateInstance for %s failed, hr = 0x%08x"), CStringFromGUID(info->clsid), hr);
//         }
//     }
    //*/

    if (!*ppBF)
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT PlayerCodecs::CreateFileCodecs(CodecsInfo* info, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks, void* pParam)
{
    HRESULT hr = E_FAIL;

    if (info->path.GetLength() <= 0)
    {
        player_log(kLogLevelError, _T("PlayerCodecs::CreateFileCodecs, CodecsInfo got invalid path"));
        return hr;
    }

//     CString codecspath(m_strCodecsPath);
//     ::PathAppend(codecspath.GetBuffer(MAX_PATH), info->path);
//     codecspath.ReleaseBuffer();

    CString codecspath = info->path;
    //player_log(kLogLevelTrace, _T("Loading filter, codecspath = %s"), codecspath);

    REFCLSID clsid = info->clsid;
    CString name(info->name);

    // ffdshow
    if (name.MakeLower().Find(_T("ffdshow")) != -1)
    {
        CComPtr<IffDShowBase> pIffDShowBase;
        hr = LoadExternalObject(codecspath, clsid, IID_IffDShowBase, (void**)&pIffDShowBase);
        if (SUCCEEDED(hr))
        {
            ConfigFFDShow((void*)pIffDShowBase, info->clsid);
            hr = pIffDShowBase->QueryInterface(IID_IBaseFilter, (void**)ppBF);
        }
    }
    else
    {
        hr = LoadExternalFilter(codecspath, clsid, ppBF);
    }

    if (FAILED(hr))
    {
        //player_log(kLogLevelTrace, _T("Loading filter failed, hr = 0x%08X"), hr);
    }

    return hr;
}

HRESULT PlayerCodecs::CreateInnerCodecs(CodecsInfo* info,
                                        IBaseFilter** ppBF,
                                        CInterfaceList<IUnknown, &IID_IUnknown>& pUnks,
                                        void* pParam)
{
    CheckPointer(info, E_POINTER);
    CheckPointer(ppBF, E_POINTER);

    HRESULT hr = E_FAIL;

    CComPtr<IBaseFilter> pBF;

    REFCLSID clsid = info->clsid;
    // PlayerAsyncReader
    if (clsid == __uuidof(PlayerAsyncReader))
    {
        CAsyncStream* pStream = (CAsyncStream*)pParam;
        if (pStream == NULL)
            return E_INVALIDARG;
        pBF = new PlayerAsyncReader(pStream);
        if (pBF)
            hr = S_OK;
        //hr = PlayerInnerFilter<PlayerAsyncReader>::Create(&pBF);
    }
    else if (clsid == __uuidof(CNullTextRenderer))
    {
        HRESULT hr2 = S_OK;
        pBF = new CNullTextRenderer(NULL, &hr2);
        if (pBF && SUCCEEDED(hr2))
            hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        *ppBF = pBF.Detach();
    }

    return hr;
}

HRESULT PlayerCodecs::ParseCodecsInfoConfig(const char* config)
{
    HRESULT hr = S_OK;

    rapidxml::xml_document<char> doc;

    try
    {
        doc.parse<0>((char*)config);
    }
    catch(rapidxml::parse_error& error)
    {
        CString strError(error.what());
        player_log(kLogLevelError, _T("RapidXml got parse error:%s"), strError);
        return E_FAIL;
    }

    rapidxml::xml_node<char>* node = doc.first_node();
    if (node)
        node = node->first_node("codecs");
    for(; node!=NULL; node=node->next_sibling())
    {
        if (_stricmp(node->name(), "codecs") != 0)
        {
            continue;
        }

        CodecsInfo* info  = new CodecsInfo();
        if (info == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        rapidxml::xml_attribute<char>* attr = node->first_attribute();
        for(; attr!=NULL; attr=attr->next_attribute())
        {
            SetCodecsInfo(info, attr->name(), attr->value());
        }

        rapidxml::xml_node<char>* subnode = node->first_node();
        for (; subnode!=NULL; subnode=subnode->next_sibling())
        {
            SetCodecsInfo(info, (void*)subnode);
        }

        m_CodecsList.AddTail(info);
    }

    return hr;
}

HRESULT PlayerCodecs::LoadPreloadCodecs()
{
    POSITION pos = m_CodecsList.GetHeadPosition();
    while (pos)
    {
        CodecsInfo* pInfo = m_CodecsList.GetNext(pos);
        if (pInfo && pInfo->preload)
        {
            IBaseFilter* pBF = NULL;
            HRESULT hrLoad = LoadExternalFilter(pInfo->path, pInfo->clsid, &pBF);
            if (FAILED(hrLoad))
            {
                player_log(kLogLevelError, _T("PlayerCodecs::LoadPreloadCodecs, LoadExternalFilter failed, hr = 0x%08x"), hrLoad);
            }
        }
    }

    return S_OK;
}

HRESULT PlayerCodecs::SetCodecsInfo(CodecsInfo* info, const char* key, const char* val)
{
    CheckPointer(info, E_POINTER);

    HRESULT hr = S_OK;

    if (_stricmp(key, "enable") == 0)
    {
        info->enable = (bool)(!!strtoul(val, NULL, 10));
    }
    else if (_stricmp(key, "priority") == 0)
    {
        info->priority = strtoul(val, NULL, 10);
    }
    else if (_stricmp(key, "name") == 0)
    {
        info->name = CStringA(val);
    }
    else if (_stricmp(key, "pathflag") == 0)
    {
        info->pathflag = CodecsPathFlagFromText(val);
    }
    else if (_stricmp(key, "path") == 0)
    {
        info->path = CStringA(val);
    }
    else if (_stricmp(key, "clsid") == 0)
    {
        info->clsid = GUIDFromCString(CString(val));//CStringA(val);
    }
    else if (_stricmp(key, "category") == 0)
    {
        info->category = CodecsCategoryFromText(val);
    }
    else if (_stricmp(key, "type") == 0)
    {
        info->type = CodecsTypeFromText(val);
    }
    else if (_stricmp(key, "catedata") == 0)
    {
        info->catedata = CStringA(val);
    }
    else if (_stricmp(key, "preload") == 0)
    {
        info->preload = !!strtoul(val, NULL, 10);
    }
    else
    {
        //g_utility.Log(_T("Unknown filter info item:%s"), pcszKey);
        assert(0);
        hr = E_FAIL;
    }

    return hr;
}

HRESULT PlayerCodecs::SetCodecsInfo(CodecsInfo* info, void* subnode)
{
    CheckPointer(subnode, E_POINTER);

    HRESULT hr = S_OK;

    rapidxml::xml_node<char>* node = (rapidxml::xml_node<char>*)subnode;
    rapidxml::xml_attribute<char>* attr = 0;

    const char* key = node->name();

    if (_stricmp(key, "protocol") == 0)
    {
        attr = node->first_attribute("value");
        if (attr)
        {
            CString strVal(attr->value());
            info->protocols.AddTail(strVal);
        }
    }
    else if (_stricmp(key, "ext") == 0)
    {
        attr = node->first_attribute("value");
        if (attr)
        {
            CString strVal(attr->value());
            info->extensions.AddTail(strVal);
        }
    }
    else if (_stricmp(key, "checkbyte") == 0)
    {
        attr = node->first_attribute("value");
        if (attr)
        {
            CString strVal(attr->value());
            CheckByteItem item;	
            int i = strVal.Find(_T("|"));
            if(i > 0)
            {
                item.checkbyte = strVal.Mid(0, i);
                const GUID* guid = KnownGuid::Find(CStringA(strVal.Mid(i+1)));
                if (guid)
                {
                    item.subtype = *guid;
                }
                else
                {
                    player_log(kLogLevelWarn, _T("## %s subtype not in global GUID map"), strVal.Mid(i+1));
                }
            }
            else
            {
                item.checkbyte = strVal;
            }
            info->checkbytes.AddTail(item);
        }
    }
    else if (_stricmp(key, "depend") == 0)
    {
        attr = node->first_attribute("dll");
        if (attr)
        {
            CString strVal(attr->value());
            info->depends.AddTail(strVal);
        }
    }
    else if (_stricmp(key, "mediatype") == 0)
    {
        rapidxml::xml_attribute<char>* major=node->first_attribute("major");
        rapidxml::xml_attribute<char>* sub=node->first_attribute("sub");

        if (major && sub)
        {
            MediaTypeItem item;
            const GUID* pGUID = NULL;
            
            pGUID = KnownGuid::Find(major->value());
            if (pGUID)
                item.majortype = *pGUID;
            else
            {
                player_log(kLogLevelWarn, _T("## %s not in the global GUID map"), CString(major->value()));
                item.majortype = GUIDFromCString(CString(major->value()));
            }
            pGUID = KnownGuid::Find(sub->value());
            if (pGUID)
                item.subtype = *pGUID;
            else
            {
                player_log(kLogLevelWarn, _T("## %s not in the global GUID map"), CString(sub->value()));
                item.subtype = GUIDFromCString(CString(sub->value()));
            }

            info->mediatypes.AddTail(item);
        }
    }
    else if (_stricmp(key, "preload") == 0)
    {
        attr = node->first_attribute("dll");
        if (attr)
        {
            CString strVal(attr->value());
            info->preloads.AddTail(strVal);
        }
    }
    else
    {
        //g_utility.Log(_T("Unknown filter info item:%s"), pcszKey);
        assert(0);
        hr = E_FAIL;
    }

    return hr;
}


CodecsInfo* PlayerCodecs::FindCodecsInfo(REFCLSID clsid)
{
    CodecsInfo* pFound = NULL;
    POSITION pos = m_CodecsList.GetHeadPosition();
    while (pos)
    {
        CodecsInfo* info = m_CodecsList.GetNext(pos);
        if (info)
        {
            if (clsid == info->clsid)
            {
                pFound = info;
                break;
            }
        }
    }

    return pFound;
}

void PlayerCodecs::ConfigFFDShow(void* pffdshowbase, REFCLSID clsid)
{
    IffDShowBase* pffdshow = (IffDShowBase*)pffdshowbase;
    if(pffdshow == NULL)
        return;

    // Set generic options
    pffdshow->putParamStr(IDFF_installPath, m_strCodecsPath);
    pffdshow->putParam(IDFF_isWhitelist, 0);
    pffdshow->putParam(IDFF_trayIcon, 0);

    // Set for special filter
    //if(_tcsicmp(pcszGUID, _T("{0B0EFF97-C750-462C-9488-B10E7D87F1A6}")) == 0)
//     if (clsid == CLSID_FFDShowDXVADecoder)
//     {
//         player_log(kLogLevelTrace, _T("Is ffdshow DXVA decoder, enable text pin"));
//         pffdshow->putParam(IDFF_subTextpin, 1);
//     }
//     else
//     {
//         player_log(kLogLevelTrace, _T("Is not ffdshow DXVA decoder, disable text pin"));
//         pffdshow->putParam(IDFF_subTextpin, 0);
//     }
    pffdshow->putParam(IDFF_subTextpin, 0);

    // Set codecs
    pffdshow->putParam(IDFF_xvid, 1);
    pffdshow->putParam(IDFF_div3, 1);
    pffdshow->putParam(IDFF_mp4v, 1);
    pffdshow->putParam(IDFF_dx50, 1);
    pffdshow->putParam(IDFF_fvfw, 1);
    pffdshow->putParam(IDFF_mp43, 1);
    pffdshow->putParam(IDFF_mp42, 1);
    pffdshow->putParam(IDFF_mp41, 1);
    pffdshow->putParam(IDFF_h263, 1);
    pffdshow->putParam(IDFF_h264, 1);
    pffdshow->putParam(IDFF_h261, 1);
    pffdshow->putParam(IDFF_wmv1, 0xc);
    pffdshow->putParam(IDFF_wmv2, 0xc);
    pffdshow->putParam(IDFF_wmv3, 0xc);
    pffdshow->putParam(IDFF_wvc1, 0xc);
    pffdshow->putParam(IDFF_cavs, 1);
    pffdshow->putParam(IDFF_vp5, 1);
    pffdshow->putParam(IDFF_vp6, 1);
    pffdshow->putParam(IDFF_vp6f, 1);
    pffdshow->putParam(IDFF_rt21, 1);
    pffdshow->putParam(IDFF_vixl, 1);
    pffdshow->putParam(IDFF_aasc, 1);
    pffdshow->putParam(IDFF_qtrpza, 1);
    pffdshow->putParam(IDFF_mjpg, 1);
    pffdshow->putParam(IDFF_dvsd, 1);
    pffdshow->putParam(IDFF_hfyu, 1);
    pffdshow->putParam(IDFF_cyuv, 1);
    pffdshow->putParam(IDFF_mpg1, 5);
    pffdshow->putParam(IDFF_mpg2, 5);
    pffdshow->putParam(IDFF_mpegAVI, 1);
    pffdshow->putParam(IDFF_asv1, 1);
    pffdshow->putParam(IDFF_vcr1, 1);
    pffdshow->putParam(IDFF_rle, 1);
    pffdshow->putParam(IDFF_theo, 1);
    pffdshow->putParam(IDFF_rv10, 1);
    pffdshow->putParam(IDFF_ffv1, 1);
    pffdshow->putParam(IDFF_vp3, 1);
    pffdshow->putParam(IDFF_tscc, 1);
    pffdshow->putParam(IDFF_rawv, 1);
    pffdshow->putParam(IDFF_alternateUncompressed, 1);
    pffdshow->putParam(IDFF_svq1, 1);
    pffdshow->putParam(IDFF_svq3, 1);
    pffdshow->putParam(IDFF_cram, 1);
    pffdshow->putParam(IDFF_iv32, 1);
    pffdshow->putParam(IDFF_cvid, 1);
    pffdshow->putParam(IDFF_mszh, 1);
    pffdshow->putParam(IDFF_zlib, 1);
    pffdshow->putParam(IDFF_flv1, 1);
    pffdshow->putParam(IDFF_8bps, 1);
    pffdshow->putParam(IDFF_png1, 1);
    pffdshow->putParam(IDFF_qtrle, 1);
    pffdshow->putParam(IDFF_duck, 1);
    pffdshow->putParam(IDFF_qpeg, 1);
    pffdshow->putParam(IDFF_loco, 1);
    pffdshow->putParam(IDFF_wnv1, 1);
    pffdshow->putParam(IDFF_cscd, 1);
    pffdshow->putParam(IDFF_zmbv, 1);
    pffdshow->putParam(IDFF_ulti, 1);
    pffdshow->putParam(IDFF_wma7, 1);
    pffdshow->putParam(IDFF_wma8, 1);
    pffdshow->putParam(IDFF_mp2, 1);
    pffdshow->putParam(IDFF_mp3, 1);
    pffdshow->putParam(IDFF_ac3, 1);
    pffdshow->putParam(IDFF_eac3, 1);
    pffdshow->putParam(IDFF_dts, 1);
    pffdshow->putParam(IDFF_dtsinwav, 1);
    pffdshow->putParam(IDFF_aac, 1);
    pffdshow->putParam(IDFF_amr, 1);
    pffdshow->putParam(IDFF_iadpcm, 1);
    pffdshow->putParam(IDFF_msadpcm, 1);
    pffdshow->putParam(IDFF_otherAdpcm, 1);
    pffdshow->putParam(IDFF_law, 1);
    pffdshow->putParam(IDFF_gsm, 1);
    pffdshow->putParam(IDFF_flac, 1);
    pffdshow->putParam(IDFF_tta, 1);
    pffdshow->putParam(IDFF_qdm2, 1);
    pffdshow->putParam(IDFF_mace, 1);
    pffdshow->putParam(IDFF_truespeech, 1);
    pffdshow->putParam(IDFF_vorbis, 1);
    pffdshow->putParam(IDFF_lpcm, 4);
    pffdshow->putParam(IDFF_fps1, 0);
    pffdshow->putParam(IDFF_ra, 1);
    pffdshow->putParam(IDFF_imc, 1);
    pffdshow->putParam(IDFF_mss2, 0xc);
    pffdshow->putParam(IDFF_wvp2, 0xc);
    pffdshow->putParam(IDFF_em2v, 1);
    pffdshow->putParam(IDFF_avrn, 1);
    pffdshow->putParam(IDFF_cdvc, 1);
    pffdshow->putParam(IDFF_atrac3, 1);
    pffdshow->putParam(IDFF_nellymoser, 1);
    pffdshow->putParam(IDFF_wavpack, 1);
    pffdshow->putParam(IDFF_rawa, 4);
    pffdshow->putParam(IDFF_avisV, 0xa);
    pffdshow->putParam(IDFF_avisA, 0xa);
    pffdshow->putParam(IDFF_mlp, 1);
    pffdshow->putParam(IDFF_truehd, 1);
    pffdshow->putParam(IDFF_rv40, 0);
    pffdshow->putParam(IDFF_rv30, 0);
    pffdshow->putParam(IDFF_cook, 1);
    pffdshow->putParam(IDFF_vp8, 1);
    pffdshow->putParam(IDFF_iv50, 1);
    pffdshow->putParam(IDFF_i263, 0);
}

static CodecsCategory CodecsCategoryFromText(const char* cate)
{
    for (int i=0; i<_countof(s_CodecsCategoryAll); ++i)
    {
        if (_stricmp(s_CodecsCategoryAll[i].name, cate) == 0)
        {
            return s_CodecsCategoryAll[i].category;
        }
    }
    return kCodecsCategoryUnknown;
}

static CodecsType CodecsTypeFromText(const char* type)
{
    for (int i=0; i<_countof(s_CodecsTypeAll); ++i)
    {
        if (_stricmp(s_CodecsTypeAll[i].name, type) == 0)
        {
            return s_CodecsTypeAll[i].type;
        }
    }
    return kCodecsTypeUnknown;
}

static CodecsPathFlag CodecsPathFlagFromText(const char* flag)
{
    for (int i=0; i<_countof(s_CodecsPathFlagAll); ++i)
    {
        if (_stricmp(s_CodecsPathFlagAll[i].name, flag) == 0)
        {
            return s_CodecsPathFlagAll[i].flag;
        }
    }
    return kCodecsPathFlagUnknown;
}