
// 1 Producer, 2 Consumers, 4 Buffers
#include <windows.h>
#include <process.h>
#include <stdio.h>

BOOL SetConsoleColor(WORD wAttributes);
unsigned int __stdcall ProducerThreadFunc(LPVOID lpParam);
unsigned int __stdcall ConsumerThreadFunc(LPVOID lpParam);

const int kMaxProductNumber = 8;
const int kBufferSize = 4;

int g_Buffer[kBufferSize];
int g_iProducerIndex, g_iConsumerIndex;

CRITICAL_SECTION g_csLock;
HANDLE g_hSemaphoreBufferEmpty, g_hSemaphoreBufferFull;


int main()
{
	printf("Producer-Consumer-Problem, Sample: 1 Producer, 2 Consumers, 4 Buffers\n");

	InitializeCriticalSection(&g_csLock);

	g_hSemaphoreBufferEmpty = CreateSemaphore(NULL, kBufferSize, kBufferSize, NULL);
	g_hSemaphoreBufferFull = CreateSemaphore(NULL, 0, kBufferSize, NULL);
	g_iProducerIndex = 0;
	g_iConsumerIndex = 0;
	memset(g_Buffer, 0, sizeof(g_Buffer));

	const int kThreadNum = 3;
	HANDLE hThreads[kThreadNum];

	hThreads[0] = (HANDLE)_beginthreadex(NULL, 0, ProducerThreadFunc, NULL, 0, NULL);
	hThreads[1] = (HANDLE)_beginthreadex(NULL, 0, ConsumerThreadFunc, NULL, 0, NULL);
	hThreads[2] = (HANDLE)_beginthreadex(NULL, 0, ConsumerThreadFunc, NULL, 0, NULL);

	WaitForMultipleObjects(kThreadNum, hThreads, TRUE, INFINITE);
	for (int i=0; i<kThreadNum; i++)
		CloseHandle(hThreads[i]);

	CloseHandle(g_hSemaphoreBufferEmpty);
	CloseHandle(g_hSemaphoreBufferFull);	
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
		// wait for buffer is empty
		WaitForSingleObject(g_hSemaphoreBufferEmpty, INFINITE);

		EnterCriticalSection(&g_csLock);
		g_Buffer[g_iProducerIndex] = i;
		printf("@Producer is producing data %d into buffer %d\n", g_Buffer[g_iProducerIndex], g_iProducerIndex);
		g_iProducerIndex = (g_iProducerIndex+1) % kBufferSize;
		LeaveCriticalSection(&g_csLock);

		// notify consumers
		ReleaseSemaphore(g_hSemaphoreBufferFull, 1, NULL);
	}
	printf("Producer finished task, thread exited\n");
	return 0;
}

unsigned int __stdcall ConsumerThreadFunc(LPVOID lpParam)
{
	while (true)
	{
		// wait for non-empty buffer
		WaitForSingleObject(g_hSemaphoreBufferFull, INFINITE);

		EnterCriticalSection(&g_csLock);
		SetConsoleColor(FOREGROUND_GREEN);
		printf("#%d Consumer is consuming data %d from buffer %d\n", GetCurrentThreadId(), g_Buffer[g_iConsumerIndex], g_iConsumerIndex);
		SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

		// end
		if (g_Buffer[g_iConsumerIndex] == kMaxProductNumber-1)
		{
			LeaveCriticalSection(&g_csLock);
			// notify other consumers
			ReleaseSemaphore(g_hSemaphoreBufferFull, 1, NULL);
			break;
		}
		g_iConsumerIndex = (g_iConsumerIndex+1) % kBufferSize;

		LeaveCriticalSection(&g_csLock);
		Sleep(50);

		ReleaseSemaphore(g_hSemaphoreBufferEmpty, 1, NULL);
	}
	SetConsoleColor(FOREGROUND_GREEN);
	printf("#d Consumer received notification, thread exit\n", GetCurrentThreadId());
	SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	return 0;
}
