#ifndef _PLAYERCORE_PLAYERCODECS_H_
#define _PLAYERCORE_PLAYERCODECS_H_

#include "CodecsInfo.h"

class PlayerCodecs
{
public:
    PlayerCodecs();
    ~PlayerCodecs();

    void            SetCodecsPath(const char* path);
    PlayerResult    LoadCodecs();
    void            FreeCodecs();

    CodecsInfoList& GetCodecsInfoList() {
        return codecs_list_;
    }

    static PlayerResult CreateCodecsObject(CodecsInfo* info, IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);

protected:
    CString codecs_path_;
    CodecsInfoList codecs_list_;

};

#endif