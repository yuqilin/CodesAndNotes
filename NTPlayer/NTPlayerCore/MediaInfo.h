#pragma once

class PlayerBaseStream;

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
struct MEMORY_DATA
{
    int	    size;
    BYTE *	data;

    MEMORY_DATA() {
        size = 0;
        data = NULL;
    }

    ~MEMORY_DATA() {
        Reset();
    }

    void Reset() {
        SAFE_DELETE(data);
        size = 0;
    }
};

class MediaInfo
{
public:
    MediaInfo(LPCTSTR lpUrl, HRESULT& hr);
    ~MediaInfo();

    inline LPCTSTR GetUrl() {
        return m_pUrl;
    }
    LPCTSTR GetExtension() {
        return m_pExtension;
    }
    LPCTSTR GetTitle() {
        return m_pTitle;
    }
    LPCTSTR GetProtocol() {
        return m_pProtocol;
    }
    MediaProtocol GetProtocol() {
        return m_Protocol;
    }
    MEMORY_DATA* GetHeader() {
        return &m_Header;
    }

    void SetStream(PlayerBaseStream* stream) {
        m_pStream = stream;
    }
    void SetSize(UINT64 nSize) {
        m_nSize = nSize;
    }

protected:
    HRESULT Create(LPCTSTR pcszUrl);
    void    Destroy();

protected:
    TCHAR *			m_pUrl;
    TCHAR *			m_pExtension;
    TCHAR *         m_pTitle;
    TCHAR *         m_pProtocol;
    MediaProtocol   m_Protocol;

    PlayerBaseStream*	    m_pStream;
    UINT64          m_nSize;
    MEMORY_DATA     m_Header;
};

