#ifndef __NTPLAYER_NTPLAYERIMPL_H__
#define __NTPLAYER_NTPLAYERIMPL_H__

#include "CodecsManager.h"

class CNTPlayer
{
public:
    CNTPlayer();
    ~CNTPlayer();

    HRESULT LoadCodecsInfo();
    HRESULT LoadPlayerSettings();

    HRESULT GetCodecsInfoString(const char** info);

private:
    static CCodecsManager m_CodecsManager;
};

#endif