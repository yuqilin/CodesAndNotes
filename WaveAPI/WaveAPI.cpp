
#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

#define BUFFER_SIZE		(44100 * 16 * 2 / 8 * 5)	//录制声音长度
#define FRAGMENT_SIZE	1024						//缓存区大小
#define FRAGMENT_NUM	4							//缓存区个数

static unsigned char buffer[BUFFER_SIZE] = {0};
static int buf_count = 0;

void CALLBACK waveInProc(HWAVEIN hwi,
						 UINT uMsg,
						 DWORD_PTR dwInstance,
						 DWORD_PTR dwParam1,
						 DWORD_PTR dwParam2);


void CALLBACK waveOutProc(HWAVEOUT hwo,
						 UINT uMsg,
						 DWORD_PTR dwInstance,
						 DWORD_PTR dwParam1,
						 DWORD_PTR dwParam2);

int main(int argc, char* argv[])
{
	/* Record Sound */

	// Device
	int nReturn = waveInGetNumDevs();
	printf("Number of Audio Input Devices：%d\n", nReturn);

	for (int i=0; i<nReturn; i++)
	{
		WAVEINCAPS wic;
		waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS));
		printf("#%02d Device Info:\n", i);
		printf("\tmanufacturer ID:%d\n", wic.wMid);
		printf("\tproduct ID:%d\n", wic.wPid);
		printf("\tversion of the driver:%d\n", wic.vDriverVersion);
		printf("\tproduct name:%s\n", wic.szPname);
		printf("\tformats supported:%d\n", wic.dwFormats);
		printf("\tnumber of channels supported:%d\n", wic.wChannels);
		printf("\tstructure packing:%d\n", wic.wReserved1);
	}

	// Open
	HWAVEIN hWaveIn;
	WAVEFORMATEX wfx;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = 44100;
	wfx.nAvgBytesPerSec = 44100 * 16 * 2 / 8;
	wfx.nBlockAlign = 4;
	wfx.wBitsPerSample = 16;
	wfx.cbSize = 0;

	waveInOpen(&hWaveIn, WAVE_MAPPER, &wfx, 
		(DWORD_PTR)waveInProc, 0, CALLBACK_FUNCTION);

	WAVEINCAPS wic;
	waveInGetDevCaps((UINT)hWaveIn, &wic, sizeof(WAVEINCAPS));
	printf("Audio Input Device Opened :%s\n", wic.szPname);

	// Prepare buffer
	static WAVEHDR wh[FRAGMENT_NUM];
	for (int i=0; i<FRAGMENT_NUM; i++)
	{
		wh[i].lpData = new char[FRAGMENT_SIZE];  
		wh[i].dwBufferLength = FRAGMENT_SIZE;  
		wh[i].dwBytesRecorded = 0;  
		wh[i].dwUser = NULL;  
		wh[i].dwFlags = 0;  
		wh[i].dwLoops = 1;  
		wh[i].lpNext = NULL;  
		wh[i].reserved = 0;  

		waveInPrepareHeader(hWaveIn, &wh[i], sizeof(WAVEHDR));
		waveInAddBuffer(hWaveIn, &wh[i], sizeof(WAVEHDR));
	}

	// record   
	printf("Start to Record...\n");  

	buf_count = 0;  
	waveInStart(hWaveIn);  

	while (buf_count < BUFFER_SIZE)  
	{  
		Sleep(1);  
	}  

	printf("Record Over!\n\n");  

	// clean   
	waveInStop(hWaveIn);  
	waveInReset(hWaveIn);  
	for (int i=0; i<FRAGMENT_NUM; i++)  
	{  
		waveInUnprepareHeader(hWaveIn, &wh[i], sizeof(WAVEHDR));  
		delete wh[i].lpData;  
	}  
	waveInClose(hWaveIn);  

	system("pause");  
	printf("\n");  

	/* Play Sound */  

	// Device   
	nReturn = waveOutGetNumDevs();  
	printf("\nNumber Of Audio Output Devices：%d\n", nReturn);  
	for (int i=0; i<nReturn; i++)  
	{  
		WAVEOUTCAPS woc;  
		waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS));  
		printf("#%02d Device Info:\n", i);
		printf("\tmanufacturer ID:%d\n", woc.wMid);
		printf("\tproduct ID:%d\n", woc.wPid);
		printf("\tversion of the driver:%d\n", woc.vDriverVersion);
		printf("\tproduct name:%s\n", woc.szPname);
		printf("\tformats supported:%d\n", woc.dwFormats);
		printf("\tnumber of channels supported:%d\n", woc.wChannels);
		printf("\tstructure packing:%d\n", woc.wReserved1);
	}  

	// open   
	HWAVEOUT hWaveOut;  
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);  

	WAVEOUTCAPS woc;  
	waveOutGetDevCaps((UINT_PTR)hWaveOut, &woc, sizeof(WAVEOUTCAPS));  
	printf("Audio Output Device Opened :%s\n", woc.szPname);  

	// prepare buffer   
	WAVEHDR wavhdr;  
	wavhdr.lpData = (LPSTR)buffer;  
	wavhdr.dwBufferLength = BUFFER_SIZE;  
	wavhdr.dwFlags = 0;  
	wavhdr.dwLoops = 0;  

	waveOutPrepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR));

	// play   
	printf("Start to Play...\n");  

	buf_count = 0;  
	waveOutWrite(hWaveOut, &wavhdr, sizeof(WAVEHDR));  
	while (buf_count < BUFFER_SIZE)  
	{  
		Sleep(1);  
	}  

	// clean   
	waveOutReset(hWaveOut);  
	waveOutUnprepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR));  
	waveOutClose(hWaveOut);  

	printf("Play Over!\n\n");  

	return 0;
}

// 录音回调函数
void CALLBACK waveInProc(HWAVEIN hwi,        
						 UINT uMsg,           
						 DWORD_PTR dwInstance,    
						 DWORD_PTR dwParam1,    
						 DWORD_PTR dwParam2)  
{  
	LPWAVEHDR pwh = (LPWAVEHDR)dwParam1;  

	if ((WIM_DATA==uMsg) && (buf_count<BUFFER_SIZE))  
	{  
		int temp = BUFFER_SIZE - buf_count;  
		temp = (temp>pwh->dwBytesRecorded) ? pwh->dwBytesRecorded : temp;  
		memcpy(buffer+buf_count, pwh->lpData, temp);  
		buf_count += temp;  

		waveInAddBuffer(hwi, pwh, sizeof(WAVEHDR));
	}  
}

// 放音回调函数   
void CALLBACK waveOutProc(HWAVEOUT hwo,     
						  UINT uMsg,           
						  DWORD_PTR dwInstance,     
						  DWORD_PTR dwParam1,      
						  DWORD_PTR dwParam2)  
{  
	if (WOM_DONE == uMsg)  
	{  
		buf_count = BUFFER_SIZE;  
	}  
}  