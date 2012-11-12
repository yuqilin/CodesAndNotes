
#include "faac.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <assert.h>
#include <dshow.h>
#include <comutil.h>
#include <vector>

#define BUFFER_SIZE 4096

typedef struct 
{
	WORD wFormatTag;
	WORD wChannels;
	DWORD dwSamplesPerSec;
	DWORD dwAvgBytesPerSec;
	WORD wBlockAlign;
	WORD wBitsPerSample;
} WAVEFORMATINFO;

int ParseWaveFile(void* fpInput, WAVEFORMATINFO* waveFormat, int* nDataPos);


/*
#define BeginEnumSysDev(clsid, pMoniker)                                                                      \
{                                                                                                             \
	CComPtr<ICreateDevEnum> pDevEnum4$##clsid;                                                                \
	pDevEnum4$##clsid.CoCreateInstance(CLSID_SystemDeviceEnum);                                               \
	CComPtr<IEnumMoniker> pClassEnum4$##clsid;                                                                \
	if (SUCCEEDED(pDevEnum4$##clsid->CreateClassEnumerator(clsid, &pClassEnum4$##clsid, 0))                   \
	&& pClassEnum4$##clsid)                                                                               \
{                                                                                                         \
	for (CComPtr<IMoniker> pMoniker; pClassEnum4$##clsid->Next(1, &pMoniker, 0) == S_OK; pMoniker = NULL) \
{

#define EndEnumSysDev }}}
//*/


//HRESULT EnumSystemDevice(const CLSID& clsDeviceCategory, const TCHAR* pcszDeviceName, IBaseFilter** ppFilter);

typedef struct tagDeviceInfo{
	TCHAR szFriendlyName[MAX_PATH];
	TCHAR szDescription[MAX_PATH];
	TCHAR szDevicePath[MAX_PATH];
	LONG lWaveInID;
	IBaseFilter* pFilter;
	GUID clsid;
}DeviceInfo;

HRESULT EnumSystemDevices(const CLSID& category, std::vector<DeviceInfo*>& devices);
void ClearInfo(std::vector<DeviceInfo*>& devices);



int main(int argc, char* argv[])
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
		return -1;

	std::vector<DeviceInfo*> devices;
	EnumSystemDevices(CLSID_AudioInputDeviceCategory, devices);
	ClearInfo(devices);
	
	EnumSystemDevices(CLSID_AudioRendererCategory, devices);
	ClearInfo(devices);


	FILE* fpInput;
	FILE* fpOutput;

	errno_t err;
	int nRet;
	int nBytesRead;
	int nPCMBufferSize;
	BYTE* pbPCMBuffer = NULL;
	BYTE* pbAACBuffer = NULL;
	

	WAVEFORMATINFO waveFormat;
	ULONG nInputSamples;
	ULONG nMaxOutputBytes;

	faacEncHandle hEncoder;
	faacEncConfigurationPtr pConfiguration;

	const char* pszInputFileName = argv[1];//"D:\\MediaSample\\wav\\11k16bitpcm.wav";

	if (pszInputFileName == NULL)
		goto error_out;

	char pszOutputFileName[MAX_PATH];// = "D:\\MediaSample\\wav\\8k16bitpcm.aac";

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath_s(pszInputFileName, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, szFName, _MAX_FNAME, szExt, _MAX_EXT);

	_makepath_s(pszOutputFileName, szDrive, szDir, szFName, "aac");

	err = fopen_s(&fpInput, pszInputFileName, "rb");
	err = fopen_s(&fpOutput, pszOutputFileName, "wb");

	// Parse WAVE file
	int nDataPos = 0;
	if (!ParseWaveFile(fpInput, &waveFormat, &nDataPos))
		goto error_out;

	// Open FAAC encoder
	hEncoder = faacEncOpen(waveFormat.dwSamplesPerSec, waveFormat.wChannels, &nInputSamples, &nMaxOutputBytes);
	if (hEncoder == NULL)
	{
		goto error_out;
	}

	nPCMBufferSize = nInputSamples * waveFormat.wBitsPerSample / 8;
	pbPCMBuffer = new BYTE[nPCMBufferSize];
	pbAACBuffer = new BYTE[nMaxOutputBytes];

	// Get current encoding configuration
	pConfiguration = faacEncGetCurrentConfiguration(hEncoder);
	switch (waveFormat.wBitsPerSample)
	{
	case 16:
		pConfiguration->inputFormat = FAAC_INPUT_16BIT;
		break;
	case 24:
		pConfiguration->inputFormat = FAAC_INPUT_24BIT;
		break;
	case 32:
		pConfiguration->inputFormat = FAAC_INPUT_32BIT;
		break;
	default:
		goto error_out;
	}
	
	// Set encoding configuration
	nRet = faacEncSetConfiguration(hEncoder, pConfiguration);

	fseek(fpInput, nDataPos, SEEK_SET);
	for (int i=0; true; i++)
	{
		// 读入的实际字节数，最大不会超过nPCMBufferSize，一般只有读到文件尾时才不是这个值
		nBytesRead = fread(pbPCMBuffer, 1, nPCMBufferSize, fpInput);
		if (nBytesRead <= 0)
			break;

		// 输入样本数，用实际读入字节数计算，一般只有读到文件尾时才不是nPCMBufferSize/(waveFormat.wBitsPerSample/8);
		nInputSamples = nBytesRead / (waveFormat.wBitsPerSample / 8);

		// Encode
		nRet = faacEncEncode(hEncoder, (int*)pbPCMBuffer, nInputSamples, pbAACBuffer, nMaxOutputBytes);
// 		if (nRet != 0)
// 			break;
		int nWritten = fwrite(pbAACBuffer, 1, nRet, fpOutput);
		assert(nWritten == nRet);
	}

	// Close FAAC encoder
	nRet = faacEncClose(hEncoder);

	delete[] pbPCMBuffer;
	delete[] pbAACBuffer;
	fclose(fpInput);
	fclose(fpOutput);

	::CoUninitialize();

	return 0;

error_out:
	delete[] pbPCMBuffer;
	delete[] pbAACBuffer;
	fclose(fpInput);
	fclose(fpOutput);
	
	::CoUninitialize();

	return -1;
}

int ParseWaveFile(void* fpIn, WAVEFORMATINFO* waveFormat, int* nDataPos)
{
	FILE* fpInput = (FILE*)fpIn;
	fseek(fpInput, 0, SEEK_END);
	long fsize = ftell(fpInput);

	if (fsize < 12)
		return 0;

	DWORD tag_id, tag_size;
	size_t read_bytes;
	long pos = 0;

	rewind(fpInput);
	read_bytes = fread(&tag_id, sizeof(DWORD), 1, fpInput);
	assert(read_bytes == 1);
	pos += 4;
	read_bytes = fread(&tag_size, sizeof(DWORD), 1, fpInput);
	assert(read_bytes == 1);
	pos += 4;

	assert(tag_id == 'FFIR');
	assert(tag_size+8 == fsize);

	read_bytes = fread(&tag_id, sizeof(DWORD), 1, fpInput);
	assert(read_bytes == 1);
	assert(tag_id == 'EVAW');
	pos += 4;

	while (pos < fsize)
	{
		fseek(fpInput, pos, SEEK_SET);
		read_bytes = fread(&tag_id, sizeof(DWORD), 1, fpInput);
		assert(read_bytes == 1);
		pos += 4;

		read_bytes = fread(&tag_size, sizeof(DWORD), 1, fpInput);
		assert(read_bytes == 1);
		pos += 4;

		switch (tag_id)
		{
		case ' tmf':
			read_bytes = fread((BYTE *)waveFormat, sizeof(WAVEFORMATINFO), 1, fpInput);
			assert(read_bytes == 1);
			break;
		case 'atad':
			*nDataPos = pos;
			break;
		}

		pos += tag_size;
	}
	
	return 1;
}

int EnumCaptureDevice(const CLSID& clsDeviceCategory, const TCHAR* pcszDeviceName, IBaseFilter** ppFilter)
{
	HRESULT hr;
	ICreateDevEnum* pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void**)&pSysDevEnum);
	if (SUCCEEDED(hr))
	{
		// Obtain a class enumerator for the audio input device category
		IEnumMoniker* pEnumCat = NULL;
		hr = pSysDevEnum->CreateClassEnumerator(clsDeviceCategory/*CLSID_AudioInputDeviceCategory*/, &pEnumCat, 0);
		if (hr == S_OK)
		{
			// Enumerate the monikers
			IMoniker* pMoniker = NULL;
			ULONG cFetched;
			while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
			{
				IPropertyBag* pPropBag = NULL;
				hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag,
					(void**)&pPropBag);
				if (SUCCEEDED(hr))
				{
					// To retrieve the filter's friendly name
					VARIANT varName;
					VariantInit(&varName);
					hr = pPropBag->Read(L"FriendlyName", &varName, NULL);
					if (SUCCEEDED(hr))
					{
						// Display the name in your UI somehow
						LPCTSTR pcszName = _bstr_t(varName.bstrVal);
						if (lstrcmpi(pcszName, pcszDeviceName) == 0)
						{
							// TODO:
						}
					}
					VariantClear(&varName);

					// To create an instance of the filter
					IBaseFilter* pFilter = NULL;
					hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
						(void**)&pFilter);

					// Now add the filter to the graph
					// Remember to release pFilter later
					if (ppFilter && *ppFilter)
					{
						*ppFilter = pFilter;
					}
					else
					{
						pFilter->Release();
					}

					pPropBag->Release();
				}

				pMoniker->Release();
			}
			pEnumCat->Release();
		}
		pSysDevEnum->Release();
	}


	

	return 0;
}


HRESULT EnumSystemDevices(const CLSID& category, std::vector<DeviceInfo*>& devices)
{
	HRESULT hr = S_OK;
	ICreateDevEnum* pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void**)&pSysDevEnum);
	if (SUCCEEDED(hr))
	{
		IEnumMoniker* pEnumCat = NULL;
		hr = pSysDevEnum->CreateClassEnumerator(category, &pEnumCat, 0);
		if (hr == S_OK)
		{
			IMoniker* pMoniker = NULL;
			ULONG cFetched;
			while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
			{
				DeviceInfo* pInfo = new DeviceInfo;
				memset(pInfo, 0, sizeof(DeviceInfo));

				IPropertyBag* pPropBag = NULL;
				hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag,
					(void**)&pPropBag);
				if (SUCCEEDED(hr))
				{
					VARIANT var;
					VariantInit(&var);

					hr = pPropBag->Read(L"FriendlyName", &var, NULL);
					if (SUCCEEDED(hr))
					{
						LPCTSTR pcszName = _bstr_t(var.bstrVal);
						_tcscpy_s(pInfo->szFriendlyName, pcszName);
						VariantClear(&var);
					}

					hr = pPropBag->Read(L"Description", &var, NULL);
					if (SUCCEEDED(hr))
					{
						LPCTSTR pcszDescription = _bstr_t(var.bstrVal);
						_tcscpy_s(pInfo->szDescription, pcszDescription);
						VariantClear(&var);
					}
					
					hr = pPropBag->Read(L"DevicePath", &var, NULL);
					if (SUCCEEDED(hr))
					{
						LPCTSTR pcszPath = _bstr_t(var.bstrVal);
						_tcscpy_s(pInfo->szDevicePath, pcszPath);
						VariantClear(&var);
					}

					hr = pPropBag->Read(L"WaveInID", &var, NULL);
					if (SUCCEEDED(hr))
					{
						LONG lWaveInID = var.lVal;
						pInfo->lWaveInID = lWaveInID;
						VariantClear(&var);
					}

					// To create an instance of the filter
					IBaseFilter* pFilter = NULL;
					hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
						(void**)&pFilter);

					// Remember to release pFilter later
					if (SUCCEEDED(hr))
					{
						pInfo->pFilter = pFilter;

						CLSID clsid;
						hr = pFilter->GetClassID(&clsid);
						if (SUCCEEDED(hr))
						{
							pInfo->clsid = clsid;
						}

						IEnumPins *pEnumPins = NULL;
						IPin *pPin = NULL;
						IAMAudioInputMixer *pMixer = NULL;
						hr = pFilter->EnumPins(&pEnumPins);
						if (SUCCEEDED(hr))
						{
							while (pEnumPins->Next(1, &pPin, NULL) == S_OK)
							{
								PIN_DIRECTION dir;
								hr = pPin->QueryDirection(&dir);
								if (SUCCEEDED(hr))
								{
									if (dir == PINDIR_INPUT)
									{
										hr = pPin->QueryInterface(IID_IAMAudioInputMixer, (void**)&pMixer);
										if (SUCCEEDED(hr))
										{
											BOOL bEnable = FALSE;
											if (S_OK == pMixer->get_Enable(&bEnable))
											{
												if (bEnable)
												{
													double dLevel = 0.0;
													if (S_OK == pMixer->get_MixLevel(&dLevel))
														printf("MixLevel=%f", dLevel);
												}
											}
											pMixer->Release();
										}
									}
								}
								pPin->Release();
							}
							pEnumPins->Release();
						}
					}	
					pPropBag->Release();
				}
				pMoniker->Release();

				devices.push_back(pInfo);
			}
			pEnumCat->Release();
		}
		pSysDevEnum->Release();
	}

	return hr;
}

void ClearInfo(std::vector<DeviceInfo*>& devices)
{
	std::vector<DeviceInfo*>::iterator it;
	for (it=devices.begin(); it!=devices.end(); ++it)
	{
		IBaseFilter* pFilter = (*it)->pFilter;
		if (pFilter != NULL)
		{
			pFilter->Release();
		}
		delete (*it);
	}
	devices.clear();
}