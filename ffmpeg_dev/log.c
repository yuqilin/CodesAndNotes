#include "log.h"
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static FILE* g_file = NULL;

static const char* g_tag = "[concate]";

int xlog_init(const char* filepath)
{
    g_file = fopen(filepath, "w+");
    if (!g_file)
        return -1;
    return 0;
}

int xlog_uninit()
{
    if (g_file)
        fclose(g_file);
    g_file = NULL;
    return 0;
}

int xlog(const char* format, ...)
{
    char* p;
    char text[2048] = {0};
    va_list vl = NULL;

    strcpy(text, g_tag);
    p = text + strlen(g_tag);

    va_start(vl, format);
    vsprintf(p, format, vl);
    va_end(vl);
    
    strcat(text, "\r\n");

    if (g_file)
        fwrite(text, strlen(text), 1, g_file);
    return 0;
}