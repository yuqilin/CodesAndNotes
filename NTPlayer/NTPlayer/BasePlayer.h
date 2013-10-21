#ifndef _NTPLAYER_BASEPLAYER_H_
#define _NTPLAYER_BASEPLAYER_H_

class CBasePlayer
{
public:
    // play control
    virtual HRESULT Open(CMediaInfo* pMediaInfo) = 0;
    virtual HRESULT Close() = 0;
    virtual HRESULT Play() = 0;
    virtual HRESULT Pause() = 0;
    //virtual HRESULT Abort(BOOL bStop) = 0;

    virtual long    GetDuration() = 0;
    virtual long    GetPlayPos() = 0 ;
    virtual HRESULT SetPlayPos(long nPosToPlay) = 0;

    virtual int     GetVolume() = 0;
    virtual HRESULT SetVolume(int nVolume) = 0;

    virtual BOOL    GetMute() = 0;
    virtual HRESULT SetMute(BOOL bMute) = 0;

protected:
    CMediaInfo*     m_pMediaInfo;
};

#endif