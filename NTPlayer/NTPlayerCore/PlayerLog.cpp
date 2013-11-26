#include "stdafx.h"
#include "PlayerLog.h"

static int player_log_level = kLogLevelInfo;

void show_debug_callback(int level, const wchar_t* format, va_list vl);

#if 1
static void (*player_log_callback)(int, const wchar_t*, va_list) = show_debug_callback;
#else
static void (*player_log_callback)(int, const wchar_t*, va_list) = NULL;
#endif

void show_debug_callback(int level, const wchar_t* format, va_list vl)
{
    SYSTEMTIME st;
    ::GetLocalTime(&st);

//     CStringW strLog, strText;
//     strLog.FormatV(format, vl);
//     strText.Format(_T("NTPlayer[%02d:%02d:%02d.%03d][%04X]: %s\r\n"),
//         st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, ::GetCurrentThreadId(), strLog);
    const int kMaxLogLength = 2048;
    TCHAR strLog[kMaxLogLength], strText[kMaxLogLength];
    _vsnwprintf_s(strLog, _countof(strLog), _TRUNCATE, format, vl);
    _snwprintf_s(strText, _countof(strText), _TRUNCATE, _T("NTPlayer[%02d:%02d:%02d.%03d][%04X]: %s\r\n"),
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, ::GetCurrentThreadId(), strLog);

//      wchar_t buffer[1024] = L"PlayerCore: ";
//      _vsnwprintf_s(buffer+wcslen(buffer), _countof(buffer)-wcslen(buffer), _TRUNCATE, format, vl);
    ::OutputDebugStringW(strText);
}

void player_log(int level, const wchar_t* format, ...)
{
    if (player_log_callback)
    {
        va_list vl;
        va_start(vl, format);
        player_log_callback(level, format, vl);
        va_end(vl);
    }
}

void player_log_set_callback(void (*callback)(int, const wchar_t*, va_list))
{
    player_log_callback = callback;
}

void player_log_set_level(int level)
{
    player_log_level = level;
}
