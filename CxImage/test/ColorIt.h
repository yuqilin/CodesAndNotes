#ifndef __COLORIT_H__
#define __COLORIT_H__

typedef int (*cbGetPixelColor)(int x, int y);
typedef void (*cbDrawPixelColor)(int x, int y, int new_color);

void SetImageInfo(void* image, int width, int height,
                  cbGetPixelColor cb_GetPixelColor,
                  cbDrawPixelColor cb_DrawPixelColor);
void ScanLineSeedFill(int x, int y, int new_color, int boundary_color);


#endif