#ifndef __COLORIT_H__
#define __COLORIT_H__

void SetImage(int** image_array, int width, int height);
void PrintImagePixel();
void ScanLineSeedFill(int x, int y, int old_color, int new_color, int boundary_color);


#endif