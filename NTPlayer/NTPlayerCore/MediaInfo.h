#pragma once

//////////////////////////////////////////////////////////////////////////
typedef enum tagMediaProtocol {
    kProtocolUnknown,
    kProtocolFile,
    kProtocolHttp,
    kProtocolHttps,
    kProtocolFtp,
    kProtocolRtsp,
    kProtocolQvod,
    kProtocolBdhd,
    kProtocolLast,
} MediaProtocol;

MediaProtocol ProtocolFromString(LPCTSTR pcszProtocol);

//////////////////////////////////////////////////////////////////////////
struct VideoSize
{
    int		w;
    int		h;
    int		arx;
    int		ary;

    VideoSize() { clear(); }
    void	clear() { memset(this, 0, sizeof(VideoSize)); }
    BOOL	valid() { return (w > 0); }
};

//////////////////////////////////////////////////////////////////////////
class MediaInfo
{
public:
    MediaInfo(LPCTSTR lpUrl, HRESULT& hr);
    ~MediaInfo();

    PCTSTR GetUrl() {
        return m_pUrl;
    }
    LPCTSTR GetProtocol() {
        return m_pProtocol;
    }
    LPCTSTR GetExtension() {
        return m_pExtension;
    }
    LPCTSTR GetTitle() {
        return m_pTitle;
    }

    HRESULT SetTitle(LPCTSTR pTitle) {
        SAFE_DELETE(m_pTitle);
        int len = _tcslen(pTitle) + 1;
        if (len > 0)
        {
            m_pTitle = new TCHAR[len];
            if (m_pTitle == NULL)
                return E_OUTOFMEMORY;
            _tcscpy_s(m_pTitle, len, pTitle);
            return S_OK;
        }
        else
            return E_INVALIDARG;
    }
    
    void SetSize(ULONGLONG llSize) {
        m_llSize = llSize;
    }
    void SetDuration(long duration) {
        m_lDuration = duration;
    }
    long GetDuration() {
        return m_lDuration;
    }

    VideoSize* GetVideoSize() {
        return &m_VideoSize;
    }
    void SetVideoSize(VideoSize* pVideoSize) {
        if (pVideoSize)
        {
            m_VideoSize = *pVideoSize;
        }
    }


protected:
    HRESULT Create(LPCTSTR pcszUrl);
    void    Destroy();

protected:
    TCHAR *			m_pUrl;
    TCHAR *			m_pExtension;
    TCHAR *         m_pTitle;
    TCHAR *         m_pProtocol;
    ULONGLONG       m_llSize;
    LONG            m_lDuration;
    VideoSize       m_VideoSize;
};

