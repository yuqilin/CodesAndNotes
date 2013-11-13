#include "stdafx.h"
#include "PlayerAsyncReader.h"

PlayerAsyncReader::PlayerAsyncReader(HRESULT* phr)
: CAsyncReader(NAME("PlayerAsyncReader"), NULL, &m_stream, phr, __uuidof(this))
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

//     if (!m_stream.Load(pszFileName)) {
//         return E_FAIL;
//     }

    m_fn = pszFileName;

    mt.majortype = MEDIATYPE_Stream;
    mt.subtype = m_stream.m_subtype;
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
