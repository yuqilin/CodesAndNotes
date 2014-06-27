
#ifdef _MSC_VER
#define inline __inline
//#define strdup _strdup
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include "concate.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "log.h"

int main(int argc, char** argv)
{
    char szPath[MAX_PATH];
    GetModuleFileNameA(NULL, szPath, MAX_PATH);
    char* slash = strrchr(szPath, '\\');
    szPath[slash - szPath] = 0;
    int set = SetCurrentDirectoryA(szPath);

    if (argc < 3) {
        printf("usage: %s input output\n", argv[0]);
        return 1;
    }

    char* inputs = strdup(argv[1]);
    char* output = strdup(argv[2]);

    xlog_init("C:\\Users\\yuqilin\\Desktop\\ÀëÏß·Ö¶Î\\log.txt");

    ConcatContext * ctx = concat_new();

    concat_set_inputs(ctx, inputs);

    concat_set_output(ctx, output);

    concat_process(ctx);

    concat_free(ctx);

    free(inputs);
    free(output);

    xlog_uninit();

    return 0;
}