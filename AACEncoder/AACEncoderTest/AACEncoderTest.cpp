
#include "faac.h"
#include <windows.h>
#include <stdio.h>
#include <assert.h>

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

int main(int argc, char* argv[])
{
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
		return 0;

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
	return 0;

error_out:
	delete[] pbPCMBuffer;
	delete[] pbAACBuffer;
	fclose(fpInput);
	fclose(fpOutput);
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