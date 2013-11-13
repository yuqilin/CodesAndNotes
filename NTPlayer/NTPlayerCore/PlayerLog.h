#pragma once

typedef enum tagPlayerLogLevel {
    kLogLevelTrace,
    kLogLevelDebug,
    kLogLevelInfo,
    kLogLevelWarn,
    kLogLevelError,
    kLogLevelFatal,
} PlayerLogLevel;

void player_log(int level, const wchar_t* format, ...);

void player_log_set_callback(void (*)(int, const wchar_t*, va_list));
void player_log_set_level(int level);


