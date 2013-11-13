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
    wchar_t buffer[1024] = L"PlayerCore: ";
    _vsnwprintf_s(buffer+wcslen(buffer), _countof(buffer)-wcslen(buffer), _TRUNCATE, format, vl);
    ::OutputDebugStringW(buffer);
    ::OutputDebugStringW(_T("\n"));
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
