#include "stdafx.h"
#include "NTPlayerImpl.h"

CCodecsManager CNTPlayer::m_CodecsManager;

CNTPlayer::CNTPlayer()
{

}

CNTPlayer::~CNTPlayer()
{

}

HRESULT CNTPlayer::LoadCodecsInfo()
{
    m_CodecsManager.LoadCodecsInfo();

    return S_OK;
}

HRESULT CNTPlayer::LoadPlayerSettings()
{
    return S_OK;
}

HRESULT CNTPlayer::GetCodecsInfoString(const char** info)
{
    if (info)
    {
        *info = m_CodecsManager.GetCodecsInfoString();
    }
    return S_OK;
}