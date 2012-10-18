BOOL MByteToWChar(LPCSTR lpcszSrc, LPWSTR lpwszDst, DWORD dwSize)
{
	// Get the required size of the buffer that receives the Unicode 
	// string. 
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar(CP_ACP, 0, lpcszSrc, -1, NULL, 0);

	if(dwSize < dwMinSize)
	{
		return FALSE;
	}
	// Convert headers from ASCII to Unicode.
	MultiByteToWideChar(CP_ACP, 0, lpcszSrc, -1, lpwszDst, dwMinSize); 
	return TRUE;
}

BOOL WCharToMByte(LPCWSTR lpcwszSrc, LPSTR lpszDst, DWORD dwSize)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_ACP, NULL, lpcwszSrc, -1, NULL, 0, NULL, FALSE);
	if(dwSize < dwMinSize)
	{
		return FALSE;
	}
	WideCharToMultiByte(CP_ACP, NULL, lpcwszSrc, -1, lpszDst, dwSize, NULL, FALSE);
	return TRUE;
}