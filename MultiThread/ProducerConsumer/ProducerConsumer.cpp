
// 1 producer, 1 consumer, 1 buffer
// 2 events, one for buffer_empty, the other for buffer_full
// 1 critical_section for the buffer r/w

#include <windows.h>
#include <process.h>
#include <stdio.h>

const int kMaxProductNumber = 10;
int g_iBufferIndex;

CRITICAL_SECTION g_csLock;
HANDLE g_hEventBufferEmpty, g_hEventBufferFull;


BOOL SetConsoleColor(WORD wAttributes);
unsigned int __stdcall ProducerThreadFunc(LPVOID lpParam);
unsigned int __stdcall ConsumerThreadFunc(LPVOID lpParam);

int main()
{
	printf("Producer-Consumer Problem, Sample: 1 producer, 1 consumer, 1 buffer\n");

	InitializeCriticalSection(&g_csLock);

	g_hEventBufferEmpty = CreateEvent(NULL, FALSE, TRUE, NULL);
	g_hEventBufferFull = CreateEvent(NULL, FALSE, FALSE, NULL);

	const int kThreadNum = 2;
	HANDLE hThreads[kThreadNum];

	hThreads[0] = (HANDLE)_beginthreadex(NULL, 0, ProducerThreadFunc, NULL, 0, NULL);
	hThreads[1] = (HANDLE)_beginthreadex(NULL, 0, ConsumerThreadFunc, NULL, 0, NULL);
	WaitForMultipleObjects(kThreadNum, hThreads, TRUE, INFINITE);
	CloseHandle(hThreads[0]);
	CloseHandle(hThreads[1]);


	CloseHandle(g_hEventBufferEmpty);
	CloseHandle(g_hEventBufferFull);
	DeleteCriticalSection(&g_csLock);
	return 0;
}

BOOL SetConsoleColor(WORD wAttributes)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE)
		return FALSE;
	return SetConsoleTextAttribute(hConsole, wAttributes);
}

unsigned int __stdcall ProducerThreadFunc(LPVOID lpParam)
{
	for (int i=0; i<kMaxProductNumber; i++)
	{
		// wait buffer_empty event
		WaitForSingleObject(g_hEventBufferEmpty, INFINITE);

		// mutually access the buffer
		EnterCriticalSection(&g_csLock);

		g_iBufferIndex = i;
		printf("@Producer is producing %d into buffer\n", i);
		
		LeaveCriticalSection(&g_csLock);

		SetEvent(g_hEventBufferFull);
	}

	return 0;
}

unsigned int __stdcall ConsumerThreadFunc(LPVOID lpParam)
{
	volatile bool flag = true;

	while (flag)
	{
		// wait buffer_full event
		WaitForSingleObject(g_hEventBufferFull, INFINITE);

		// mutally access the buffer
		EnterCriticalSection(&g_csLock);

		SetConsoleColor(FOREGROUND_GREEN);
		printf("#Consumer is consuming %d from buffer\n", g_iBufferIndex);
		SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

		if (g_iBufferIndex == kMaxProductNumber)
			flag = false;

		LeaveCriticalSection(&g_csLock);

		SetEvent(g_hEventBufferEmpty);

		Sleep(10);
	}

	return 0;
}
