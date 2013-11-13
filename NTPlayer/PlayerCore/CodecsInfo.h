#ifndef _PLAYERCORE_CODECSINFO_H_
#define _PLAYERCORE_CODECSINFO_H_

#include <atlstr.h>
#include <atlcoll.h>

using ATL::CString;

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
//     bool enable;
//     DWORD priority;
    UINT64 merit;
    CString name;
    CString	pathflag;
    CString	path;
    CodecsCategory category;
    CString	catedata;
    CodecsType type;
    GUID clsid;
    CAtlList<CString> protocols;
    CAtlList<CString> extensions;
    CAtlList<CString> depends;
    CAtlList<CString> preloads;
    CAtlList<CheckByteItem> checkbytes;
    CAtlList<MediaTypeItem> mediatypes;

    CodecsInfo()
    {
        this->merit = MERIT64_DO_NOT_USE;
        this->category = kCodecsCategoryUnknown;
        this->type = kCodecsTypeUnknown;
        this->clsid = GUID_NULL;
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

    bool IsVideoType() {
        return 
            ( type == kCodecsTypeVideoDecoder   ||
            type == kCodecsTypeVideoEffect    ||
            type == kCodecsTypeVideoEncoder   ||
            type == kCodecsTypeVideoRenderer  );

    }

    bool IsAudioType() {
        return 
            ( type == kCodecsTypeAudioDecoder   ||
            type == kCodecsTypeAudioEffect    ||
            type == kCodecsTypeAudioEncoder   ||
            type == kCodecsTypeAudioRenderer  );
    }
};


typedef CAtlList<CodecsInfo*>       CodecsInfoList;

#endif