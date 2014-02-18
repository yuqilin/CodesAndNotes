#include "ColorIt.h"
#include <stdio.h>

static const int s_ImageWidth = 16;
static const int s_ImageHeight = 16;

static int s_ImageArray[s_ImageHeight][s_ImageWidth] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0,
    0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0,
    0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int a[3][5] = {
    0,  1,  2,  3,  4, 
    5,  6,  7,  8,  9,
    10, 11, 12, 13, 14
};

int main(int argc, char **argv)
{
    for (int i=0; i<3; i++)
    {
        for (int j=0; j<5; j++)
        {
            printf("%d ", a[i][j]);
        }
        printf("\n");
    }

    printf("%d ", a[1][2]);

    SetImage((int**)s_ImageArray, s_ImageWidth, s_ImageHeight);

    PrintImagePixel();

    int x = 4, y = 3;
    int old_color = 0;
    int new_color = 3;
    int boundary_color = 1;
    ScanLineSeedFill(x, y, old_color, new_color, boundary_color);

    printf("\n\n");
    PrintImagePixel();

    getchar();
    return 0;
}