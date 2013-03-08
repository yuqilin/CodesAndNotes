
#include <windows.h>
#include <tchar.h>
#include <tlhelp32.h>

BOOL	KillProcessByName(const TCHAR* pcszProcessName);

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	BOOL bKilled = KillProcessByName(_T("SohuVA.exe"));

	return bKilled;
}

BOOL KillProcessByName(const TCHAR* pcszProcessName)
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 peEntry;
	peEntry.dwSize = sizeof(peEntry);

	BOOL bNext = Process32First(hSnapShot, &peEntry);

	TCHAR szText[MAX_PATH];

	BOOL bFound = FALSE;
	while (bNext)
	{
		if (_tcsicmp(peEntry.szExeFile, pcszProcessName) == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, (DWORD)peEntry.th32ProcessID);

			if (hProcess != NULL)
			{
				bFound = TRUE;
				_stprintf_s(szText, MAX_PATH, _T("OpenProcess for %s OK"), pcszProcessName);
				//MessageBox(NULL, szText, _T("Tip"), MB_OK);
				TerminateProcess(hProcess, 0);
				CloseHandle(hProcess);
				break;
			}
			else
			{
				_stprintf_s(szText, MAX_PATH, _T("OpenProcess for %s FAILED, LastError=0x%08x"),
					pcszProcessName, GetLastError());
				MessageBox(NULL, szText, _T("Error"), MB_OK);
			}
		}
		bNext = Process32Next(hSnapShot, &peEntry);
	}

	_stprintf_s(szText, MAX_PATH, _T("Process %s "), pcszProcessName);
	if (bFound)
	{
		_tcscat_s(szText, MAX_PATH, _T("found and killed."));
	}
	else
	{
		_tcscat_s(szText, MAX_PATH, _T("not found."));
	}
	MessageBox(NULL, szText, _T("Tip"), MB_OK);

	CloseHandle(hSnapShot);

	return bFound;
}
