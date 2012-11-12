//------------------------------------------------------------------------------
// File: WavDest.h
//
// Desc: DirectShow sample code - header file for WAV audio file writer filter.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#pragma warning(disable: 4097 4511 4512 4514 4705)

#include "faac.h"
#include "faaccfg.h"

#define AAC_VERSION_MPEG4		0
#define AAC_VERSION_MPEG2		1

#define AAC_OBJECT_MAIN			1
#define AAC_OBJECT_LOW			2
#define AAC_OBJECT_SSR			3
#define AAC_OBJECT_LTP			4

#define AAC_OUTPUT_RAW			0
#define AAC_OUTPUT_ADTS			1

struct AACConfig
{
	int			version;
	int			object_type;
	int			output_type;
	int			bitrate;
};

struct AACInfo
{
	int			samplerate;
	int			channels;
	int			frame_size;
	__int64		frames_done;
};


class CWavDestOutputPin : public CTransformOutputPin
{
public:
    CWavDestOutputPin(CTransformFilter *pFilter, HRESULT * phr);

    STDMETHODIMP EnumMediaTypes( IEnumMediaTypes **ppEnum );
    HRESULT CheckMediaType(const CMediaType* pmt);
};

class CWavDestFilter : public CTransformFilter
{

public:

    DECLARE_IUNKNOWN;
  
    CWavDestFilter(LPUNKNOWN pUnk, HRESULT *pHr);
    ~CWavDestFilter();
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *pHr);

    HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
    HRESULT Receive(IMediaSample *pSample);

    HRESULT CheckInputType(const CMediaType* mtIn) ;
    HRESULT CheckTransform(const CMediaType *mtIn,const CMediaType *mtOut);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType) ;

    HRESULT DecideBufferSize(IMemAllocator *pAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

    HRESULT StartStreaming();
    HRESULT StopStreaming();

    HRESULT CompleteConnect(PIN_DIRECTION direction,IPin *pReceivePin) { return S_OK; }

private:

    HRESULT Copy(IMediaSample *pSource, IMediaSample *pDest) const;
    HRESULT Transform(IMediaSample *pMediaSample);
    HRESULT Transform(AM_MEDIA_TYPE *pType, const signed char ContrastLevel) const;

    ULONG m_cbWavData;
    ULONG m_cbHeader;

	faacEncHandle						m_encoder;
	AACConfig							m_config;
	AACInfo								m_info;
	int									m_encoding_delay;		// (in samples)

	void CloseEncoder();
	int OpenEncoder();
};
