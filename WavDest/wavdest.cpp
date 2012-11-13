//------------------------------------------------------------------------------
// File: WavDest.cpp
//
// Desc: DirectShow sample code - a filter for writing WAV audio files (based
//       on CTransformFilter).
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
// To use this filter to write audio data into a WAV file: 
//
// Use GraphEdit (or a custom DirectShow app) to build a filter graph
// with an audio stream connected to this filter's input pin and the File Writer
// filter connected to its output pin. Run the graph and you will have
// written a wave file.
//
//=============================================================================
//=============================================================================

#include <streams.h>
#include "wavdest.h"
#include <aviriff.h>
#include <malloc.h>


// {3C78B8E2-6C4D-11d1-ADE2-0000F8754B99}
static const GUID CLSID_WavDest =
{ 0x3c78b8e2, 0x6c4d, 0x11d1, { 0xad, 0xe2, 0x0, 0x0, 0xf8, 0x75, 0x4b, 0x99 } };


const AMOVIESETUP_FILTER sudWavDest =
{
    &CLSID_WavDest,           // clsID
    L"WAV Dest",              // strName
    MERIT_DO_NOT_USE,         // dwMerit
    0,                        // nPins
    0                         // lpPin
};


// Global data
CFactoryTemplate g_Templates[]= {
    {L"WAV Dest", &CLSID_WavDest, CWavDestFilter::CreateInstance, NULL, &sudWavDest},
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


// ------------------------------------------------------------------------
// filter constructor

#pragma warning(disable:4355)


CWavDestFilter::CWavDestFilter(LPUNKNOWN pUnk, HRESULT *phr) :
                CTransformFilter(NAME("WavDest filter"), pUnk, CLSID_WavDest)
{
    ASSERT(m_pOutput == 0);
    ASSERT(phr);

    if(SUCCEEDED(*phr))
    {
        // Create an output pin so we can have control over the connection
        // media type.
        CWavDestOutputPin *pOut = new CWavDestOutputPin(this, phr);

        if(pOut)
        {
            if(SUCCEEDED(*phr))
            {
                m_pOutput = pOut;
            }
            else
            {
                delete pOut;
            }
        }
        else
        {
            *phr = E_OUTOFMEMORY;
        }

        //
        // NOTE!: If we've created our own output pin we must also create
        // the input pin ourselves because the CTransformFilter base class 
        // will create an extra output pin if the input pin wasn't created.        
        //
        CTransformInputPin *pIn = new CTransformInputPin(NAME("Transform input pin"),
                                        this,              // Owner filter
                                        phr,               // Result code
                                        L"In");            // Pin name
        // a failed return code should delete the object
        if(pIn)
        {
            if(SUCCEEDED(*phr))
            {
                m_pInput = pIn;
            }
            else
            {
                delete pIn;
            }
        }
        else
        {
            *phr = E_OUTOFMEMORY;
        }


		m_outbuf = NULL;
		m_outbuf_size = 0;

		// info
		m_info.samplerate		= 0;
		m_info.channels		= 0;
		m_info.frame_size		= 0;
		m_info.frames_done	= 0;
		
		// encoder
		m_encoder				= NULL;
		m_extradata			= NULL;
		m_extradata_size		= 0;

		// buffer
		m_enc_buffer	= (short*)malloc(6 * 4096 * sizeof(short));
		m_enc_temp	= (short*)malloc(6 * 4096 * sizeof(short));
		m_buf_time_samples = 0;

		// LoadSettings
		m_config.version		= AAC_VERSION_MPEG4;
		m_config.object_type	= AAC_OBJECT_LOW;
		m_config.output_type	= AAC_OUTPUT_RAW;
		m_config.bitrate		= 128000;
    }
}


// ------------------------------------------------------------------------
// destructor

CWavDestFilter::~CWavDestFilter()
{
	if (m_enc_buffer)
		free(m_enc_buffer);
	m_enc_buffer	= NULL;
	if (m_enc_temp)
		free(m_enc_temp);
	m_enc_temp	= NULL;
}


CUnknown * WINAPI CWavDestFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    return new CWavDestFilter(pUnk, phr);
}


//
// CWavDestFilter::CheckTransform
//
// To be able to transform, the formats must be identical
//
HRESULT CWavDestFilter::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    HRESULT hr;

    if(FAILED(hr = CheckInputType(mtIn)))
    {
        return hr;
    }

    return NOERROR;

} // CheckTransform


// overridden because we need to know if Deliver() failed.

HRESULT CWavDestFilter::Receive(IMediaSample *pSample)
{
    ULONG cbOld = m_cbWavData;
    HRESULT hr = CTransformFilter::Receive(pSample);

    // don't update the count if Deliver() downstream fails.
    if(hr != S_OK)
    {
        m_cbWavData = cbOld;
    }

	if (m_outbuf)
	{
		delete[] m_outbuf;
		m_outbuf = NULL;
	}
	m_outbuf_size = 0;

    return hr;
}

//
// CWavDestFilter::Transform
//
//
HRESULT CWavDestFilter::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
    REFERENCE_TIME rtStart, rtEnd;

    // First just copy the data to the output sample
    //HRESULT hr = Copy(pIn, pOut);
	HRESULT hr = NOERROR;
	BYTE	*buf;
	long	size;
	pIn->GetPointer(&buf);
	size = pIn->GetActualDataLength();

	short	*in_buf = (short*)buf;
	long	in_samples   = size / sizeof(short);
	int		time_samples = in_samples / m_info.channels;

	m_outbuf = new BYTE[size];
	m_outbuf_size = 0;


	hr = ProcessPCM(in_buf, time_samples);

    if(FAILED(hr))
    {
        return hr;
    }

	REFERENCE_TIME TimeStart, TimeEnd;
	if(NOERROR == pIn->GetTime(&TimeStart, &TimeEnd))
	{
		pOut->SetTime(&TimeStart, &TimeEnd);
	}

	LONGLONG MediaStart, MediaEnd;
	if(pIn->GetMediaTime(&MediaStart,&MediaEnd) == NOERROR)
	{
		pOut->SetMediaTime(&MediaStart,&MediaEnd);
	}

	// Copy the media type
	AM_MEDIA_TYPE *pMediaType;
	pIn->GetMediaType(&pMediaType);
	pOut->SetMediaType(pMediaType);
	DeleteMediaType(pMediaType);

	hr = pOut->SetActualDataLength(m_outbuf_size);
	BYTE* outbuf;
	pOut->GetPointer(&outbuf);
	memcpy(outbuf, m_outbuf, m_outbuf_size);

    // Prepare it for writing    
    LONG lActual = pOut->GetActualDataLength();

    if(m_cbWavData + m_cbHeader + lActual < m_cbWavData + m_cbHeader)
    { 
        return E_FAIL;      // overflow
    }

    rtStart = m_cbWavData + m_cbHeader;
    rtEnd   = rtStart + lActual;
    m_cbWavData += lActual;

    EXECUTE_ASSERT(pOut->SetTime(&rtStart, &rtEnd) == S_OK);

    return S_OK;
}


//
// CWavDestFilter::Copy
//
// Make destination an identical copy of source
//
HRESULT CWavDestFilter::Copy(IMediaSample *pSource, IMediaSample *pDest) const
{
    CheckPointer(pSource,E_POINTER);
    CheckPointer(pDest,E_POINTER);

    // Copy the sample data

    BYTE *pSourceBuffer, *pDestBuffer;
    long lSourceSize = pSource->GetActualDataLength();

#ifdef DEBUG    
    long lDestSize = pDest->GetSize();
    ASSERT(lDestSize >= lSourceSize);
#endif

    pSource->GetPointer(&pSourceBuffer);
    pDest->GetPointer(&pDestBuffer);

    CopyMemory((PVOID) pDestBuffer,(PVOID) pSourceBuffer,lSourceSize);

    // Copy the sample times

    REFERENCE_TIME TimeStart, TimeEnd;
    if(NOERROR == pSource->GetTime(&TimeStart, &TimeEnd))
    {
        pDest->SetTime(&TimeStart, &TimeEnd);
    }

    LONGLONG MediaStart, MediaEnd;
    if(pSource->GetMediaTime(&MediaStart,&MediaEnd) == NOERROR)
    {
        pDest->SetMediaTime(&MediaStart,&MediaEnd);
    }

    // Copy the media type
    AM_MEDIA_TYPE *pMediaType;
    pSource->GetMediaType(&pMediaType);
    pDest->SetMediaType(pMediaType);
    DeleteMediaType(pMediaType);

    // Copy the actual data length
    long lDataLength = pSource->GetActualDataLength();
    pDest->SetActualDataLength(lDataLength);

    return NOERROR;

} // Copy


//
// CheckInputType
//
HRESULT CWavDestFilter::CheckInputType(const CMediaType* mtIn)
{
    if(mtIn->formattype == FORMAT_WaveFormatEx)
    {
        return S_OK;
    }
    return S_FALSE;
}

//
// GetMediaType
//
HRESULT CWavDestFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    ASSERT(iPosition == 0 || iPosition == 1);

    if(iPosition == 0)
    {
        CheckPointer(pMediaType,E_POINTER);

        pMediaType->SetType(&MEDIATYPE_Stream);
        pMediaType->SetSubtype(&MEDIASUBTYPE_WAVE);
        return S_OK;
    }

    return VFW_S_NO_MORE_ITEMS;
}

HRESULT CWavDestFilter::SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt)
{
	if (direction == PINDIR_INPUT)
	{
		/*
			Musime odchytit vstupny format, aby sme vedeli dobre
			inicializovat enkoder
		*/
		WAVEFORMATEX	*wfx = (WAVEFORMATEX*)pmt->pbFormat;
		if (!wfx) return E_FAIL;

		// teraz nas zaujima vacero veci:
		if (wfx->wBitsPerSample != 16) return E_FAIL;
		m_info.channels	= wfx->nChannels;
		m_info.samplerate	= wfx->nSamplesPerSec;

		if (m_encoder)
		{
			CloseEncoder();
		}

		// otvorime novy enkoder
		int ret = OpenEncoder();
		if (ret < 0)
		{
			CloseEncoder();
			return E_FAIL;
		}

	} 
	else if (direction == PINDIR_OUTPUT)
	{
	}
	return NOERROR;
}

//
// DecideBufferSize
//
// Tell the output pin's allocator what size buffers we
// require. Can only do this when the input is connected
//
HRESULT CWavDestFilter::DecideBufferSize(IMemAllocator *pAlloc,
                                         ALLOCATOR_PROPERTIES *pProperties)
{
    HRESULT hr = NOERROR;

    // Is the input pin connected
    if(m_pInput->IsConnected() == FALSE)
    {
        return E_UNEXPECTED;
    }

    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProperties,E_POINTER);

    pProperties->cBuffers = 1;
    pProperties->cbAlign  = 1;

    // Get input pin's allocator size and use that
    ALLOCATOR_PROPERTIES InProps;
    IMemAllocator * pInAlloc = NULL;

    hr = m_pInput->GetAllocator(&pInAlloc);
    if(SUCCEEDED(hr))
    {
        hr = pInAlloc->GetProperties(&InProps);
        if(SUCCEEDED(hr))
        {
            pProperties->cbBuffer = InProps.cbBuffer;
        }
        pInAlloc->Release();
    }

    if(FAILED(hr))
        return hr;

    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if(FAILED(hr))
    {
        return hr;
    }

    ASSERT(Actual.cBuffers == 1);

    if(pProperties->cBuffers > Actual.cBuffers ||
        pProperties->cbBuffer > Actual.cbBuffer)
    {
        return E_FAIL;
    }

    return NOERROR;

} // DecideBufferSize


//
// StartStreaming
//
// Compute the header size to allow space for us to write it at the end.
//
// 00000000    RIFF (00568BFE) 'WAVE'
// 0000000C        fmt  (00000010)
// 00000024        data (00568700)
// 0056872C
//
HRESULT CWavDestFilter::StartStreaming()
{
    // leave space for the header
//     m_cbHeader = sizeof(RIFFLIST) + 
//                  sizeof(RIFFCHUNK) + 
//                  m_pInput->CurrentMediaType().FormatLength() + 
//                  sizeof(RIFFCHUNK);
	m_cbHeader = 0;

    m_cbWavData = 0;

	m_info.frames_done = 0;
	m_buf_time_samples = 0;

// 	if (!m_encoder)
// 	{
// 		int ret = OpenEncoder();
// 		if (ret < 0)
// 		{
// 			CloseEncoder();
// 			return E_FAIL;
// 		}
// 	}

    return S_OK;
}


//
// StopStreaming
//
// Write out the header
//
HRESULT CWavDestFilter::StopStreaming()
{
	m_buf_time_samples = 0;
	m_info.frames_done = 0;

	//CloseEncoder();

	return S_OK;
	/*
    IStream *pStream;
    if(m_pOutput->IsConnected() == FALSE)
        return E_FAIL;

    IPin * pDwnstrmInputPin = m_pOutput->GetConnected();

    if(!pDwnstrmInputPin)
        return E_FAIL;

    HRESULT hr = ((IMemInputPin *) pDwnstrmInputPin)->QueryInterface(IID_IStream, 
                                                                    (void **)&pStream);
    if(SUCCEEDED(hr))
    {
        BYTE *pb = (BYTE *)_alloca(m_cbHeader);

        RIFFLIST  *pRiffWave = (RIFFLIST *)pb;
        RIFFCHUNK *pRiffFmt  = (RIFFCHUNK *)(pRiffWave + 1);
        RIFFCHUNK *pRiffData = (RIFFCHUNK *)(((BYTE *)(pRiffFmt + 1)) + 
                               m_pInput->CurrentMediaType().FormatLength());

        pRiffData->fcc = FCC('data');
        pRiffData->cb = m_cbWavData;

        pRiffFmt->fcc = FCC('fmt ');
        pRiffFmt->cb = m_pInput->CurrentMediaType().FormatLength();
        CopyMemory(pRiffFmt + 1, m_pInput->CurrentMediaType().Format(), pRiffFmt->cb);

        pRiffWave->fcc = FCC('RIFF');
        pRiffWave->cb = m_cbWavData + m_cbHeader - sizeof(RIFFCHUNK);
        pRiffWave->fccListType = FCC('WAVE');

        LARGE_INTEGER li;
        ZeroMemory(&li, sizeof(li));

        hr = pStream->Seek(li, STREAM_SEEK_SET, 0);
        if(SUCCEEDED(hr))
        {
            hr = pStream->Write(pb, m_cbHeader, 0);
        }
        pStream->Release();
    }

    return hr;
	//*/
}

HRESULT CWavDestFilter::BreakConnect(PIN_DIRECTION dir)
{
	if (dir == PINDIR_INPUT) {
		// tu zrusime instanciu enkodera a odpojime output pin
		CloseEncoder();

		// odpojime
		if (m_pOutput->IsConnected()) {
			m_pOutput->GetConnected()->Disconnect();
			m_pOutput->Disconnect();
		}

		m_info.channels = 0;
		m_info.samplerate = 0;

	} else
		if (dir == PINDIR_OUTPUT) {
			// tu nerobime nic specialne - len zrusime pocitadla
			CAutoLock	lck(&lock_info);
			m_info.frames_done = 0;
		}
		return __super::BreakConnect(dir);
}

int CWavDestFilter::OpenEncoder()
{
	if (m_encoder) return -1;
	if (m_info.samplerate == 0 || m_info.channels == 0) return -1;

	unsigned long	framesize;
	unsigned long	ob;

	// reset encoding delay
	m_encoding_delay = 0;

	m_encoder = faacEncOpen(m_info.samplerate, m_info.channels, &framesize, &ob);
	if (!m_encoder) return -1;

	m_info.frame_size  = framesize;
	m_info.frames_done = 0;

	faacEncConfigurationPtr	conf;
	conf = faacEncGetCurrentConfiguration(m_encoder);
	if (!conf)
	{
		faacEncClose(m_encoder);
		m_encoder = NULL;
		m_info.frame_size = 0;
		return -1;
	}

// 	conf->mpegVersion	= m_config.version;
// 	conf->aacObjectType	= m_config.object_type;
// 	conf->allowMidside	= 0;
// 	conf->bandWidth		= 0;
// 	conf->bitRate		= m_config.bitrate / m_info.channels;
	conf->inputFormat	= FAAC_INPUT_16BIT;
// 	conf->useLfe		= 0;
// 	conf->useTns		= 0;
// 	conf->shortctl		= SHORTCTL_NORMAL;
// 	conf->outputFormat	= (m_config.output_type > 0 ? 1 : 0);
	faacEncSetConfiguration(m_encoder, conf);

	BYTE *ex_ptr = NULL;
	unsigned long ex_size;
	faacEncGetDecoderSpecificInfo(m_encoder, &ex_ptr, &ex_size);
	if (ex_ptr && ex_size > 0)
	{
		m_extradata_size = ex_size;
		m_extradata = (BYTE*)malloc(ex_size);
		memcpy(m_extradata, ex_ptr, ex_size);
	}

	return 0;
}

void CWavDestFilter::CloseEncoder()
{
	if (!m_encoder) return ;
	faacEncClose(m_encoder);
	m_encoder = NULL;
	m_info.frame_size = 0;
	m_info.frames_done = 0;

	if (m_extradata)
	{
		free(m_extradata);
		m_extradata = NULL;
	}
	m_extradata_size = 0;
}

HRESULT CWavDestFilter::ProcessPCM(short *pcm, int time_samples)
{
	/**************************************************************************
	*
	*	We process samples in units of (frame_size) samples. We use
	*	our temporary buffer to store samples that do not fit into the
	*	frame_size chunk.
	*
	***************************************************************************/

	int		needed_time_samples = m_info.frame_size / m_info.channels;
	int		left = time_samples + m_buf_time_samples;

	while (left >= needed_time_samples)
	{
		// copy data to the enc buffer
		if (m_buf_time_samples > 0)
		{
			memcpy(m_enc_buffer, m_enc_temp, (m_buf_time_samples * m_info.channels) * sizeof(short));
		}

		// assemble one frame of PCM data
		int	tocopy = (needed_time_samples - m_buf_time_samples) * m_info.channels;
		memcpy(m_enc_buffer + (m_buf_time_samples * m_info.channels), pcm, tocopy*sizeof(short));
		pcm += tocopy;

		// and deliver data downstream
		HRESULT hr = EncodeFrame();
		if (FAILED(hr)) return hr;

		// adjust buffers
		m_buf_time_samples = 0;
		left -= needed_time_samples;
	}

	// keep a copy of the data for later processing
	int		time_left = left - m_buf_time_samples;
	if (time_left > 0)
	{
		memcpy(m_enc_temp + m_buf_time_samples * m_info.channels, pcm, (time_left*m_info.channels)*sizeof(short));
	}
	m_buf_time_samples = left;

	return NOERROR;
}

HRESULT CWavDestFilter::EncodeFrame()
{
	/*
		This assumes we have the exact number of samples in the enc. buffer
	*/

	//int				time_samples = m_info.frame_size / m_info.channels;
	//double			ddur		 = (time_samples * 10000000.0) / (double)m_info.samplerate;
	//REFERENCE_TIME	tdur		 = (REFERENCE_TIME)ddur;

	// adjust input timestamps
	//m_samples_in += time_samples;
	
	// output buffer
	BYTE			outbuf[16*1024];
	int				outsize = sizeof(outbuf);

	// encode the frame
	int	ret = faacEncEncode(m_encoder, (int*)m_enc_buffer, m_info.frame_size, outbuf, outsize);
	if (ret > 0)
	{
		memcpy(m_outbuf+m_outbuf_size, outbuf, ret);
		m_outbuf_size += ret;


// 		REFERENCE_TIME		rtStart, rtStop;
// 		IMediaSample		*sample = NULL;
// 		//HRESULT				hr;
// 
// 		hr = GetDeliveryBuffer(&sample);
// 		//if (FAILED(hr)) return hr;
// 
// 		// we update the encoding delay value
// 		if (m_encoding_delay <= 0)
// 		{
// 			m_encoding_delay = (m_samples_in - m_samples_out);
// 		}
// 
// 		rtStart		= rtOutput + llMulDiv(m_samples_out, 10000000, m_info.samplerate, 0);
// 		m_samples_out += time_samples;
// 		rtStop		= rtOutput + llMulDiv(m_samples_out, 10000000, m_info.samplerate, 0);

//		sample->SetTime(&rtStart, &rtStop);

		// napiseme data
// 		BYTE	*out;
// 		sample->GetPointer(&out);
// 		memcpy(out, outbuf, ret);
// 		sample->SetActualDataLength(ret);

//		hr = m_pOutput->Deliver(sample);
//		sample->Release();

		//if (FAILED(hr)) return hr;
		m_info.frames_done ++;
	}

	return NOERROR;
}

//
// CWavDestOutputPin::CWavDestOutputPin 
//
CWavDestOutputPin::CWavDestOutputPin(CTransformFilter *pFilter, HRESULT * phr) :
        CTransformOutputPin(NAME("WavDest output pin"), pFilter, phr, L"Out")
{
    // Empty
}


//
// CWavDestOutputPin::EnumMediaTypes
//
STDMETHODIMP CWavDestOutputPin::EnumMediaTypes( IEnumMediaTypes **ppEnum )
{
    return CBaseOutputPin::EnumMediaTypes(ppEnum);
}

//
// CWavDestOutputPin::CheckMediaType
//
// Make sure it's our default type
//
HRESULT CWavDestOutputPin::CheckMediaType(const CMediaType* pmt)
{
    CheckPointer(pmt,E_POINTER);

    if(pmt->majortype == MEDIATYPE_Stream && pmt->subtype == MEDIASUBTYPE_WAVE)
        return S_OK;
    else
        return S_FALSE;
}


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}
