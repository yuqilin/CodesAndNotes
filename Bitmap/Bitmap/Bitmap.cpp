
#include <windows.h>
#include <tchar.h>
#include <fstream>
#include <vector>
#include <string>

HBITMAP GetSrcBmp(HDC hDC, DWORD dwWidth, DWORD dwHeight);
BOOL SaveBmp(HBITMAP hBitmap, LPCTSTR lpszFileName);
void memsetd(void* dst, unsigned int c, int nbytes);

int _tmain(int argc, _TCHAR* argv[])
{
	HDC hDC;
	HBITMAP hBmp;
	DWORD dwWidth, dwHeight;
	LPCTSTR lpszFileName = _T("test.bmp");

	dwWidth = GetSystemMetrics(SM_CXSCREEN);
	dwHeight = GetSystemMetrics(SM_CYSCREEN);

	//hDC = GetDC(NULL);
	hBmp = GetSrcBmp(NULL, dwWidth, dwHeight);

	//SaveBmp(hBmp, lpszFileName);

	//ReleaseDC(NULL, hDC);
	
	return 0;
}

HBITMAP GetSrcBmp(HDC hDC, DWORD dwWidth, DWORD dwHeight)
{
	HDC hMemDC;
	HBITMAP hBmp;
// 	BITMAPINFO bmpInfo;
// 	BYTE* pBmpBits;
// 	BITMAPINFOHEADER bmpInfoHeader;	
// 	ZeroMemory(&bmpInfoHeader, sizeof(BITMAPINFOHEADER));
// 	bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
// 	bmpInfoHeader.biWidth = dwWidth;
// 	bmpInfoHeader.biHeight = dwHeight;
// 	bmpInfoHeader.biPlanes = 1;
// 	bmpInfoHeader.biBitCount = 32;
// 	bmpInfoHeader.biCompression = BI_RGB;
// 	
// 	bmpInfo.bmiHeader = bmpInfoHeader;
// 	
// 	hMemDC = CreateCompatibleDC(hDC);
// 	hBmp = CreateDIBSection(hDC, &bmpInfo, DIB_RGB_COLORS, (void**)&pBmpBits, NULL, 0);
// 	SelectObject(hMemDC, hBmp);
// 	BitBlt(hMemDC, 0, 0, dwWidth, dwHeight, hDC, 0, 0, SRCCOPY);
// 	DeleteDC(hMemDC);

	LPCTSTR lpszText1 = _T("123\r\nÖÐ¹ú\r\n");

	LPCTSTR lpszFileName = _T("1.txt");
	std::ifstream hFile(lpszFileName);

	std::vector<std::string> lines;
	std::string text;
	std::string line;
	int nLineNum = 0;
	while(std::getline(hFile, line))
	{
		char num[10];
		itoa(++nLineNum, num, 10);
		text.append(num);
		text.append(".");
		text.append(line);
		text.append("\r\n");
	}

 	WCHAR* pszText = new WCHAR [text.length()+1];
 	MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, pszText, text.length()); 


	struct {BITMAPINFOHEADER bih; DWORD mask[3];} b = {{sizeof(BITMAPINFOHEADER), dwWidth, (int)dwHeight, 1, 32, BI_BITFIELDS, 0, 0, 0, 0, 0}, 0xFF0000, 0x00FF00, 0x0000FF};
	
	// create memory device context
	hMemDC = CreateCompatibleDC(NULL);
	// create memory compatible bitmap
	//hBmp = CreateCompatibleBitmap(hMemDC, dwWidth, dwHeight);

	hBmp = CreateDIBSection(hMemDC, (BITMAPINFO*)&b, DIB_RGB_COLORS, NULL, NULL, 0);
	BITMAP bm;
	GetObject(hBmp, sizeof(bm), &bm);
	memsetd(bm.bmBits, 0xFFFFFFFF, bm.bmHeight*bm.bmWidthBytes);

	// select bitmap into memory device context
	SelectObject(hMemDC, hBmp);

	int nPointSize = 24;
	int nFontHeight = -MulDiv(nPointSize, GetDeviceCaps(hMemDC, LOGPIXELSY), 72);

 	HFONT hFont = CreateFont(nFontHeight,
 		0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
 		ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 
 		PROOF_QUALITY, VARIABLE_PITCH, NULL);
 
 	HGDIOBJ hOld = SelectObject(hMemDC, hFont);
	
	//SetTextAlign(hMemDC, TA_LEFT);
	// set text background
	SetBkMode(hMemDC, TRANSPARENT);
	// set text color
	SetTextColor(hMemDC, RGB(0, 0, 0xFF));
	//SetMapMode(hMemDC, MM_TEXT);
	// paint text
	//BOOL bRet = TextOut(hMemDC, 0, 0, lpszText1, _tcslen(lpszText1));
	RECT rcText = {0, 0, dwWidth, dwHeight};
	BOOL bRet = DrawText(hMemDC, pszText, _tcslen(pszText), &rcText, DT_LEFT);

	SaveBmp(hBmp, _T("test.bmp"));

	SelectObject(hMemDC, hOld);

	DeleteDC(hMemDC);
	DeleteObject(hBmp);
	DeleteObject(hFont);

	delete[] pszText;
	return NULL;
}

BOOL SaveBmp(HBITMAP hBmp, LPCTSTR lpszFileName)
{
	DWORD dwBytesWritten = 0;
	
	BITMAP bmp;
	GetObject(hBmp, sizeof(BITMAP), &bmp);
	
	HANDLE hFile = CreateFile(lpszFileName,
							  GENERIC_WRITE,
							  FILE_SHARE_READ,
							  NULL,
							  CREATE_ALWAYS,
							  NULL,
							  NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	BITMAPFILEHEADER bmpFileHeader;
	BITMAPINFOHEADER bmpInfoHeader;
	ZeroMemory(&bmpFileHeader, sizeof(BITMAPFILEHEADER));
	ZeroMemory(&bmpInfoHeader, sizeof(BITMAPINFOHEADER));

	bmpFileHeader.bfType = 'MB';
	bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+bmp.bmWidthBytes*bmp.bmHeight;
	bmpFileHeader.bfReserved1 = 0;
	bmpFileHeader.bfReserved2 = 0;
	bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfoHeader.biWidth = bmp.bmWidth;
	bmpInfoHeader.biHeight = bmp.bmHeight;
	bmpInfoHeader.biPlanes = bmp.bmPlanes;
	bmpInfoHeader.biBitCount = bmp.bmBitsPixel;
	bmpInfoHeader.biCompression = BI_RGB;


	WriteFile(hFile, &bmpFileHeader, sizeof(bmpFileHeader), &dwBytesWritten, NULL);

	WriteFile(hFile, &bmpInfoHeader, sizeof(bmpInfoHeader), &dwBytesWritten, NULL);

	WriteFile(hFile, bmp.bmBits, bmp.bmWidthBytes*bmp.bmHeight, &dwBytesWritten, NULL);

	return TRUE;
}


void memsetd(void* dst, unsigned int c, int nbytes)
{
	__asm
	{
		mov eax, c
			mov ecx, nbytes
			shr ecx, 2
			mov edi, dst
			cld
			rep stosd
	}
}