#include "stdafx.h"
#include "PlayerAsyncReader.h"

PlayerAsyncReader::PlayerAsyncReader(CAsyncStream* pStream)
: CAsyncReader(NAME("PlayerAsyncReader"), NULL, pStream, NULL, __uuidof(this))
{
}

PlayerAsyncReader::~PlayerAsyncReader()
{

}

STDMETHODIMP PlayerAsyncReader::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI(IFileSourceFilter)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP PlayerAsyncReader::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt)
{
    CMediaType mt;
    m_mt = mt;

    m_fn = pszFileName;

    mt.majortype = MEDIATYPE_Stream;
    mt.subtype = GUID_NULL;

    if (pmt)
        mt.subtype = pmt->subtype;

    m_mt = mt;

    return S_OK;
}

STDMETHODIMP PlayerAsyncReader::GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt)
{
    if (!ppszFileName) {
        return E_POINTER;
    }

    *ppszFileName = (LPOLESTR)CoTaskMemAlloc((m_fn.GetLength() + 1) * sizeof(WCHAR));
    if (!(*ppszFileName)) {
        return E_OUTOFMEMORY;
    }

    wcscpy_s(*ppszFileName, m_fn.GetLength() + 1, m_fn);

    return S_OK;
}
