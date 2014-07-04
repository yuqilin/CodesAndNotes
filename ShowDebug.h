#ifndef _SHOW_DEBUG_H_
#define _SHOW_DEBUG_H_

#ifdef ENABLE_SHOW_DEBUG

#define PREFIX_A(text)      text
#define PREFIX_W(text)      L##text

#define PREFIX              "PlayerCore"

#define __STRINGA(text)     text
#define __STRINGW(text)     L##text
#define STRINGA(text)       __STRINGA(text)
#define STRINGW(text)       __STRINGW(text)

#define DEBUG_PREFIX_A      STRINGA(PREFIX)
#define DEBUG_PREFIX_W      STRINGW(PREFIX)

#include <windows.h>
#include <stdio.h>
#ifdef _UNICODE
#define ShowDebug           ShowDebugW
#else
#define ShowDebug           ShowDebugA
#endif



inline void ShowDebugA(const char* pcszFormat, ... )
{
    char szBuffer[1024] = DEBUG_PREFIX_A;

    va_list vl;
    va_start(vl, pcszFormat);
    _vsnprintf_s(szBuffer+strlen(szBuffer), _countof(szBuffer)-strlen(szBuffer), _TRUNCATE, pcszFormat, vl);
    va_end(vl);

    ::OutputDebugStringA(szBuffer);
}

inline void ShowDebugW(const wchar_t* pcszFormat, ... )
{
    wchar_t szBuffer[1024] = DEBUG_PREFIX_W;

    va_list vl;
    va_start(vl, pcszFormat);
    _vsnwprintf_s(szBuffer+wcslen(szBuffer), _countof(szBuffer)-wcslen(szBuffer), _TRUNCATE, pcszFormat, vl);
    va_end(vl);

    ::OutputDebugStringW(szBuffer);
}

#else   // ENABLE_SHOW_DEBUG

#define ShowDebug(...)

#endif  // ENABLE_SHOW_DEBUG
