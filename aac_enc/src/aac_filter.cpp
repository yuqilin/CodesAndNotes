//-----------------------------------------------------------------------------
//
//	Monogram AAC Encoder
//
//	Base on GNU LGPL libfaac
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

//-----------------------------------------------------------------------------
//
//	CAACEncoder
//
//-----------------------------------------------------------------------------
CAACEncoder::CAACEncoder(LPUNKNOWN pUnk, HRESULT *phr) :
	CTransformFilter(_T("MONOGRAM AAC Encoder"), pUnk, CLSID_MonogramAACEncoder),
	enc_buffer(NULL),
	enc_temp(NULL)
{
	// sicko je oukey
	if (phr) *phr = NOERROR;

	// vyrobime piny
	m_pInput = new CTransformInputPin(NAME("Input"), this, phr, L"In");
	m_pOutput = new CTransformOutputPin(NAME("Output"), this, phr, L"Out");

	// info
	info.samplerate		= 0;
	info.channels		= 0;
	info.frame_size		= 0;
	info.frames_done	= 0;
	samples_in = 0;
	samples_out = 0;

	// enkoder
	encoder				= NULL;
	extradata			= NULL;
	extradata_size		= 0;

	// buffer na data
	enc_buffer	= (short*)malloc(6 * 4096 * sizeof(short));
	enc_temp	= (short*)malloc(6 * 4096 * sizeof(short));
	buf_time_samples = 0;

	m_pStream = NULL;

	LoadSettings();
}

CAACEncoder::~CAACEncoder()
{
	if (enc_buffer)	free(enc_buffer);	enc_buffer	= NULL;
	if (enc_temp)	free(enc_temp);		enc_temp	= NULL;
}

CUnknown *WINAPI CAACEncoder::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
	CAACEncoder *enc = new CAACEncoder(pUnk, phr);
	if (!enc) {
		if (phr) *phr = E_OUTOFMEMORY;
	}
	return enc;
}

STDMETHODIMP CAACEncoder::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_ISpecifyPropertyPages) {
		return GetInterface((ISpecifyPropertyPages*)this, ppv);
	} else
	if (riid == IID_IMonogramAACEncoder) {
		return GetInterface((IMonogramAACEncoder*)this, ppv);
	} else
		return __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CAACEncoder::GetPages(CAUUID *pPages)
{
    CheckPointer(pPages,E_POINTER);

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

	*(pPages->pElems) = CLSID_MonogramAACEncoderPropertypage;
    return NOERROR;
}

HRESULT CAACEncoder::CheckInputType(const CMediaType *mtIn)
{
	/*
		Podporujeme len niektore typy
	*/
	if (mtIn->majortype != MEDIATYPE_Audio) return E_FAIL;
	if (mtIn->subtype != MEDIASUBTYPE_PCM) return E_FAIL;
	if (mtIn->formattype != FORMAT_WaveFormatEx) return E_FAIL;

	return NOERROR;
}

HRESULT CAACEncoder::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
	HRESULT hr = CheckInputType(mtIn);
	if (FAILED(hr)) return hr;

	// aj vystup by mal byt slusny
// 	if (mtOut->majortype != MEDIATYPE_Audio) return E_FAIL;
// 	if (mtOut->subtype != MEDIASUBTYPE_AAC) return E_FAIL;

	return NOERROR;
}

HRESULT CAACEncoder::GetMediaType(int iPosition, CMediaType *pmt)
{
	if (iPosition < 0) return E_INVALIDARG;
	if (iPosition > 0) return VFW_S_NO_MORE_ITEMS;

	// mal by byt zapojeny input pin a mal by byt aspon volajaky enkoder otvoreny
	if (m_pInput->IsConnected() == FALSE) return E_FAIL;
	if (!encoder) return E_FAIL;

// 	pmt->majortype	= MEDIATYPE_Audio;
// 	pmt->subtype	= MEDIASUBTYPE_AAC;
// 	pmt->formattype	= FORMAT_WaveFormatEx;
// 
// 	WAVEFORMATEX *wfx	= (WAVEFORMATEX*)pmt->AllocFormatBuffer(sizeof(*wfx) + extradata_size);
// 	memset(wfx, 0, sizeof(*wfx));
// 	wfx->cbSize			= extradata_size;
// 	wfx->nChannels		= info.channels;
// 	wfx->nSamplesPerSec	= info.samplerate;
// 	wfx->wFormatTag		= 0xff;					// AAC
// 
// 	// decoder specific info
// 	BYTE		*ex		= ((BYTE*)wfx) + sizeof(*wfx);
// 	memcpy(ex, extradata, extradata_size);
	
	CheckPointer(pmt, E_POINTER);
	pmt->SetType(&MEDIATYPE_Stream);
	pmt->SetSubtype(&MEDIASUBTYPE_NULL);

	return NOERROR;
}

HRESULT CAACEncoder::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp)
{
	/*
		Volime si buffer na prenos
	*/

	pProp->cbBuffer		= 64 * 1024;			// pevne dane a hotovo
	pProp->cBuffers		= 2;

	ALLOCATOR_PROPERTIES	act;
	HRESULT hr = pAlloc->SetProperties(pProp, &act);
	if (FAILED(hr)) return hr;

	// len tak pre istotu
	if (act.cbBuffer < pProp->cbBuffer) return E_FAIL;
	return NOERROR;
}


HRESULT CAACEncoder::SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt)
{
	if (direction == PINDIR_INPUT) {
		/*
			Musime odchytit vstupny format, aby sme vedeli dobre
			inicializovat enkoder
		*/
		WAVEFORMATEX	*wfx = (WAVEFORMATEX*)pmt->pbFormat;
		if (!wfx) return E_FAIL;

		// teraz nas zaujima vacero veci:
		if (wfx->wBitsPerSample != 16) return E_FAIL;
		info.channels	= wfx->nChannels;
		info.samplerate	= wfx->nSamplesPerSec;

		if (encoder) {
			CloseEncoder();
		}

		// otvorime novy enkoder
		int ret = OpenEncoder();
		if (ret < 0) {
			CloseEncoder();
			return E_FAIL;
		}

	} else
	if (direction == PINDIR_OUTPUT) {
	}
	return NOERROR;
}

HRESULT CAACEncoder::BreakConnect(PIN_DIRECTION dir)
{
	if (m_pStream)
	{
		m_pStream->Release();
		m_pStream = NULL;
	}
	if (dir == PINDIR_INPUT) {
		// tu zrusime instanciu enkodera a odpojime output pin
		CloseEncoder();

		// odpojime
		if (m_pOutput->IsConnected()) {
			m_pOutput->GetConnected()->Disconnect();
			m_pOutput->Disconnect();
		}

		info.channels = 0;
		info.samplerate = 0;

	} else
	if (dir == PINDIR_OUTPUT) {
		// tu nerobime nic specialne - len zrusime pocitadla
		CAutoLock	lck(&lock_info);
		info.frames_done = 0;
	}
	return __super::BreakConnect(dir);
}

HRESULT CAACEncoder::ProcessPCM(short *pcm, int time_samples)
{
	/**************************************************************************
	*
	*	We process samples in units of (frame_size) samples. We use
	*	our temporary buffer to store samples that do not fit into the
	*	frame_size chunk.
	*
	***************************************************************************/

	int		needed_time_samples = info.frame_size / info.channels;
	int		left = time_samples + buf_time_samples;

	while (left >= needed_time_samples) {

		// copy data to the enc buffer
		if (buf_time_samples > 0) {
			memcpy(enc_buffer, enc_temp, (buf_time_samples * info.channels) * sizeof(short));
		}

		// assemble one frame of PCM data
		int	tocopy = (needed_time_samples - buf_time_samples) * info.channels;
		memcpy(enc_buffer + (buf_time_samples*info.channels), pcm, tocopy*sizeof(short));
		pcm += tocopy;

		// and deliver data downstream
		HRESULT hr = EncodeFrame();
		if (FAILED(hr)) return hr;

		// adjust buffers
		buf_time_samples = 0;
		left -= needed_time_samples;
	}

	// keep a copy of the data for later processing
	int		time_left = left - buf_time_samples;
	if (time_left > 0) {
		memcpy(enc_temp + buf_time_samples*info.channels, pcm, (time_left*info.channels)*sizeof(short));
	}
	buf_time_samples = left;

	return NOERROR;
}

HRESULT CAACEncoder::EncodeFrame()
{
	/*
		This assumes we have the exact number of samples in the enc. buffer
	*/

	int				time_samples = info.frame_size / info.channels;
	double			ddur		 = (time_samples * 10000000.0) / (double)info.samplerate;
	REFERENCE_TIME	tdur		 = (REFERENCE_TIME)ddur;

	// adjust input timestamps
	samples_in += time_samples;
	
	// output buffer
	BYTE			outbuf[16*1024];
	int				outsize = sizeof(outbuf);

	// encode the frame
	int	ret = faacEncEncode(encoder, (int*)enc_buffer, info.frame_size, outbuf, outsize);
	if (ret > 0) {

		REFERENCE_TIME		rtStart, rtStop;
		//IMediaSample		*sample = NULL;
		//HRESULT				hr;

		//hr = GetDeliveryBuffer(&sample);
		//if (FAILED(hr)) return hr;

		// we update the encoding delay value
		if (encoding_delay <= 0) {
			encoding_delay = (samples_in - samples_out);
		}

		rtStart		= rtOutput + llMulDiv(samples_out, 10000000, info.samplerate, 0);
		samples_out += time_samples;
		rtStop		= rtOutput + llMulDiv(samples_out, 10000000, info.samplerate, 0);

		HRESULT hr;
		ULONG cbWritten = 0;
		if (m_pStream)
		{
			hr = m_pStream->Write(outbuf, ret, &cbWritten);
		}

//		sample->SetTime(&rtStart, &rtStop);

		// napiseme data
// 		BYTE	*out;
// 		sample->GetPointer(&out);
// 		memcpy(out, outbuf, ret);
// 		sample->SetActualDataLength(ret);

//		hr = m_pOutput->Deliver(sample);
//		sample->Release();

		//if (FAILED(hr)) return hr;
		info.frames_done ++;
	}

	return NOERROR;
}

HRESULT CAACEncoder::Receive(IMediaSample *pSample)
{
#define ENABLE_TRACES
#undef ENABLE_TRACES

	//-------------------------------------------------------------------------
	//	Handle Timestamps
	//-------------------------------------------------------------------------
	REFERENCE_TIME		rtStart, rtStop;
	HRESULT hr = pSample->GetTime(&rtStart, &rtStop);
	
	if (hr != NOERROR) {
		// some capture devices do not set timestamps
		// and output buffers from time to time...
		REFERENCE_TIME	time;
		m_pClock->GetTime(&time);
		rtStart = time - m_tStart;
		rtStop  = rtStart;
		hr = NOERROR;
	}

	if (hr == NOERROR) {

		// subtract the duration of buffer
		long size = pSample->GetActualDataLength();
		int  time_samples = size / (info.channels * sizeof(short));

		REFERENCE_TIME	tdur = llMulDiv(time_samples, 10000000, info.samplerate, 0);
		//rtStart -= tdur;

		if (!got_time) {
			HandleDiscontinuity();
			rtInput = rtStart;
			rtOutput = rtStart;
			got_time = true;

		#ifdef ENABLE_TRACES
			TRACE("--------------------------------\n");
			TRACE("New Input Time: %I64d\n", rtStart);
		#endif
		}

		// handle jitter
		double		dd = ((double)(samples_in + buf_time_samples) * 10000000.0 / (double)info.samplerate) + rtInput;
		__int64		rt_in  = (__int64)dd;
		__int64		jitter = (rtStart - rt_in);

		#ifdef ENABLE_TRACES
			double		ddo = ((double)(samples_out) * 10000000.0 / (double)info.samplerate) + rtOutput;
			__int64		rt_out = (__int64)ddo;
			TRACE("Out-In dif = %I64d\n", rt_out - rt_in);
		#endif

		if (jitter < 0) jitter = -jitter;

		// 100 milliseconds is too much for us
		if (jitter > 1000000) {
			HandleDiscontinuity();
			rtInput = rtStart;
			rtOutput = rtStart;
		}

		// reset timestamps on discontinuity
		if (pSample->IsDiscontinuity() == NOERROR) {
			// EHM. Don't ask how come this is okay.
			// It took me several hours to figure out
			// how to make this work with my stupid webcam

			// --- MUST BE COMMENTED ---
			//HandleDiscontinuity();

			rtInput = rtStart;
			samples_in = 0;

			double		delay_dur = ((double)encoding_delay * 10000000.0) / (double)info.samplerate;
			rtOutput = rtStart + (__int64)delay_dur;
			samples_out = 0;
		}
	}

	if (!m_pOutput->IsConnected()) return NOERROR;

	ASSERT(encoder);
	HRESULT	hrRet = NOERROR;

	//-------------------------------------------------------------------------
	//	Handle PCM data
	//-------------------------------------------------------------------------

	BYTE	*buf;
	long	size;
	pSample->GetPointer(&buf);
	size = pSample->GetActualDataLength();

	short	*in_buf = (short*)buf;
	long	in_samples   = size / sizeof(short);
	int		time_samples = in_samples / info.channels;

	hrRet = ProcessPCM(in_buf, time_samples);
	return hrRet;
}

HRESULT CAACEncoder::HandleDiscontinuity()
{
	// reset the encoder
	buf_time_samples = 0;
	CloseEncoder();
	OpenEncoder();

	samples_in = 0;
	samples_out = 0;

	return NOERROR;
}

HRESULT CAACEncoder::GetDeliveryBuffer(IMediaSample **sample)
{
    IMediaSample *pOutSample;
    HRESULT hr = m_pOutput->GetDeliveryBuffer(&pOutSample, NULL, NULL, 0);
    *sample = pOutSample;
    if (FAILED(hr)) {
        return hr;
    }

	// ak sa zmenil type, tak aktualizujeme nase info
	AM_MEDIA_TYPE *mt;
	if (pOutSample->GetMediaType(&mt) == NOERROR) {
		CMediaType _mt(*mt);
		SetMediaType(PINDIR_OUTPUT, &_mt);
		DeleteMediaType(mt);
	}

	return NOERROR;
}

STDMETHODIMP CAACEncoder::GetConfig(AACConfig *config)
{
	CAutoLock	lck(&lock_info);
	config->version		= this->config.version;
	config->object_type	= this->config.object_type;
	config->output_type	= this->config.output_type;
	config->bitrate		= this->config.bitrate;
	return NOERROR;
}

STDMETHODIMP CAACEncoder::SetConfig(AACConfig *config)
{
	if (m_State != State_Stopped) {
		return E_FAIL;
	}
	if (m_pOutput->IsConnected()) {
		return E_FAIL;
	}

	CAutoLock	lck(&lock_info);
	this->config.version		= config->version;
	this->config.object_type	= config->object_type;
	this->config.output_type	= config->output_type;
	this->config.bitrate		= config->bitrate;

	if (m_pInput->IsConnected()) {
		CloseEncoder();
		int ret = OpenEncoder();
		if (ret < 0) {
			CloseEncoder();
		}
	}

	SaveSettings();
	return NOERROR;
}

STDMETHODIMP CAACEncoder::GetInfo(AACInfo *info)
{
	CAutoLock	lck(&lock_info);
	info->samplerate	= this->info.samplerate;
	info->channels		= this->info.channels;
	info->frame_size	= this->info.frame_size;
	info->frames_done	= this->info.frames_done;	
	return NOERROR;
}

void CAACEncoder::CloseEncoder()
{
	if (!encoder) return ;
	faacEncClose(encoder);
	encoder = NULL;
	info.frame_size = 0;
	info.frames_done = 0;

	// decoder specific veci
	if (extradata) {
		free(extradata);
		extradata = NULL;
	}
	extradata_size = 0;
}

int CAACEncoder::OpenEncoder()
{
	// neotvarame 2x a checkneme na sprostosti
	if (encoder) return -1;
	if (info.samplerate == 0 || info.channels == 0) return -1;

	unsigned long	framesize;
	unsigned long	ob;

	// reset encoding delay
	encoding_delay = 0;

	// spravime enkoder
	encoder = faacEncOpen(info.samplerate, info.channels, &framesize, &ob);
	if (!encoder) return -1;

	// nastavime veci
	info.frame_size  = framesize;
	info.frames_done = 0;

	// parametre kodovania
	faacEncConfigurationPtr	conf;
	conf = faacEncGetCurrentConfiguration(encoder);
	if (!conf) {
		faacEncClose(encoder);
		encoder = NULL;
		info.frame_size = 0;
		return -1;
	}

	conf->mpegVersion	= config.version;
	conf->aacObjectType	= config.object_type;
	conf->allowMidside	= 0;
	conf->bandWidth		= 0;
	conf->bitRate		= config.bitrate / info.channels;
	conf->inputFormat	= FAAC_INPUT_16BIT;
	conf->useLfe		= 0;
	conf->useTns		= 0;
	conf->shortctl		= SHORTCTL_NORMAL;
	conf->outputFormat	= (config.output_type > 0 ? 1 : 0);
	faacEncSetConfiguration(encoder, conf);

	// nastavime extradata
	BYTE *ex_ptr = NULL;
	unsigned long ex_size;
	faacEncGetDecoderSpecificInfo(encoder, &ex_ptr, &ex_size);
	if (ex_ptr && ex_size > 0) {
		extradata_size = ex_size;
		extradata = (BYTE*)malloc(ex_size);
		memcpy(extradata, ex_ptr, ex_size);
	}

	return 0;
}

HRESULT CAACEncoder::StartStreaming()
{
	// zresetujeme veci
	got_time = false;
	buf_time_samples = 0;
	info.frames_done = 0;
	samples_in = 0;
	samples_out = 0;
	was_disc = false;

	if(m_pOutput->IsConnected() == FALSE)
		return E_FAIL;

	IPin * pDwnstrmInputPin = m_pOutput->GetConnected();
	if(!pDwnstrmInputPin)
		return E_FAIL;

	if (m_pStream)
	{
		m_pStream->Release();
		m_pStream = NULL;
	}

	HRESULT hr = ((IMemInputPin *) pDwnstrmInputPin)->QueryInterface(IID_IStream, 
		(void **)&m_pStream);
	if (FAILED(hr))
		return hr;

	return NOERROR;
}

HRESULT CAACEncoder::EndFlush()
{
	got_time = false;
	buf_time_samples = 0;
	info.frames_done = 0;
	samples_in = 0;
	samples_out = 0;
	was_disc = false;
	return __super::EndFlush();
}

void CAACEncoder::LoadSettings()
{
	CRegKey		reg;

	if (reg.Create(HKEY_CURRENT_USER, _T("Software\\MONOGRAM\\MONOGRAM AAC Encoder")) == ERROR_SUCCESS) {

		DWORD p;

		if (reg.QueryDWORDValue(_T("MPEG Version"), p) != ERROR_SUCCESS) p = 0;	if (p > 1) p = 0;
		config.version = p;

		// low complexity as default
		if (reg.QueryDWORDValue(_T("Object Type"), p) != ERROR_SUCCESS) p = 2;	
		if (p == 0 || p > 4) p = 2;
		config.object_type = p;

		// raw as default
		if (reg.QueryDWORDValue(_T("Output"), p) != ERROR_SUCCESS) p = 0;	
		if (p > 1) p = 0;
		config.output_type = p;

		// 128 kbps 
		if (reg.QueryDWORDValue(_T("Bitrate"), p) != ERROR_SUCCESS) p = 128000;	
		if (p > 512000) p = 128000;
		config.bitrate = p;

	} else {
		config.version		= AAC_VERSION_MPEG4;
		config.object_type	= AAC_OBJECT_LOW;
		config.output_type	= AAC_OUTPUT_RAW;
		config.bitrate		= 128000;
	}
}

void CAACEncoder::SaveSettings()
{
	CRegKey		reg;
	if (reg.Create(HKEY_CURRENT_USER, _T("Software\\MONOGRAM\\MONOGRAM AAC Encoder")) == ERROR_SUCCESS) {
		reg.SetDWORDValue(_T("MPEG Version"),	config.version);
		reg.SetDWORDValue(_T("Object Type"),	config.object_type);
		reg.SetDWORDValue(_T("Output"),			config.output_type);
		reg.SetDWORDValue(_T("Bitrate"),		config.bitrate);
	}
}

