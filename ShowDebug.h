

inline void ShowDebug(const TCHAR * pcszFormat, ... )
{
	TCHAR szBuffer[1024];

	va_list vl;
	va_start(vl, pcszFormat);
	_vstprintf_s(szBuffer, pcszFormat, vl);
	va_end(vl);

	::OutputDebugString(szBuffer);
}
