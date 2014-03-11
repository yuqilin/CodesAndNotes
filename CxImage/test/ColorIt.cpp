#include "ColorIt.h"
#include <stack>
#include <math.h>
#include <windows.h>

struct Point {
    int x;
    int y;

    Point(int a, int b) {
        x = a;
        y = b;
    }
};


typedef std::stack<Point> PointStack;

static void* s_Image = 0;
static int s_ImageWidth = 0;
static int s_ImageHeight = 0;

static cbGetPixelColor s_cbGetPixelColor = 0;
static cbDrawPixelColor s_cbDrawPixelColor = 0;

int FillLineRight(int x, int y, int old_color, int new_color, int boundary_color);
int FillLineLeft(int x, int y, int old_color, int new_color, int boundary_color);
void SearchLineNewSeed(PointStack& stk, int y_left, int y_right, int x, int old_color, int new_color, int boundary_color);
bool IsPixelValid(int x, int y, int old_color, int new_color, int boundary_color);
int SkipInvalidInLine(int x, int y, int x_right, int old_color, int new_color, int boundary_color);

inline void ShowDebug(const char* format, ... )
{
#if 0
    char buffer[1024];

    va_list vl;
    va_start(vl, format);
    vsprintf(buffer, format, vl);
    va_end(vl);

    ::OutputDebugStringA(buffer);
#endif
}

void SetImageInfo(void* image, int width, int height,
                  cbGetPixelColor cb_GetPixelColor,
                  cbDrawPixelColor cb_DrawPixelColor) {
    s_Image = image;
    s_ImageWidth = width;
    s_ImageHeight = height;
    s_cbGetPixelColor = cb_GetPixelColor;
    s_cbDrawPixelColor = cb_DrawPixelColor;
}

void ScanLineSeedFill(int x, int y, int new_color, int boundary_color) {
    int old_color = s_cbGetPixelColor(x, y);

    ShowDebug("ColorIt: ScanLineSeedFill, old_color = %08x", old_color);

    PointStack stk;

    // 1. push seed into stack
    stk.push(Point(x, y));

    while (!stk.empty()) {
        // 2. current seed
        Point seed = stk.top();
        stk.pop();

        // 3. fill right-hand
        int count = FillLineRight(seed.x, seed.y, old_color, new_color, boundary_color);
        
        int y_right = seed.y + count - 1;
        // 4. fill left-hand
        count = FillLineLeft(seed.x, seed.y-1, old_color, new_color, boundary_color);
        int y_left = seed.y - count;

//         if (y_left == y_right)
//         {
//             int a = 1;
//         }
//         else
//         {
//             printf("%d %d\n", y_left, y_right);
//         }

        // 5. scan adjacent line
        if (seed.x > 0)
        {
            SearchLineNewSeed(stk, y_left, y_right, seed.x-1, old_color, new_color, boundary_color);
        }
        if (seed.x < s_ImageHeight-1)
        {
            SearchLineNewSeed(stk, y_left, y_right, seed.x+1, old_color, new_color, boundary_color);
        }
    }
}

bool ColorComp(int a, int b) {
#if 1
    int absR = abs(GetRValue(a) - GetRValue(b));
    int absG = abs(GetGValue(a) - GetGValue(b));
    int absB = abs(GetBValue(a) - GetBValue(b));
    float distance = ((float)(absR * absR + absG * absG + absB * absB));
    if (distance - 10000.0f < 0.01)
        return true;
    else
        return false;
#else
    return (a == b);
#endif
}

bool IsPixelValid(int x, int y, int old_color, int new_color, int boundary_color) {
    bool bValid = false;

    int color = s_cbGetPixelColor(x, y);
    bool bSimilarTarget = ColorComp(color, old_color);
    bool bSimilarBoundary = ColorComp(color, boundary_color);
    if (bSimilarTarget && color != new_color && !bSimilarBoundary) {
        bValid = true;
    }
    else
    {
        bValid = false;
        ShowDebug("ColorIt: IsPixelValid, [%d,%d] clr(%08x) not valid",
            x, y, color);
    }
    return bValid;
}

int FillLineRight(int x, int y, int old_color, int new_color, int boundary_color) {
    int count = 0;
    while (y < s_ImageWidth && IsPixelValid(x, y, old_color, new_color, boundary_color)) {
        s_cbDrawPixelColor(x, y, new_color);
        y++;
        count++;
    }
    return count;
}

int FillLineLeft(int x, int y, int old_color, int new_color, int boundary_color) {
    int count = 0;
    while (y >= 0 && IsPixelValid(x, y, old_color, new_color, boundary_color)) {
        s_cbDrawPixelColor(x, y, new_color);
        y--;
        count++;
    }
    return count;
}

int SkipInvalidInLine(int x, int y, int y_right, int old_color, int new_color, int boundary_color) {
    int count = 0;

    while (y <= y_right && !IsPixelValid(x, y, old_color, new_color, boundary_color)) {
        y++;
        count++;
    }

    return count;
}

void SearchLineNewSeed(PointStack& stk, int y_left, int y_right, int x,
                       int old_color, int new_color, int boundary_color) {
    int yt = y_left;
    bool findNewSeed = false;

    while (yt <= y_right) {
        findNewSeed = false;
        while (IsPixelValid(x, yt, old_color, new_color, boundary_color) && (yt <= y_right)) {
            findNewSeed = true;
            yt++;
        }
        if (findNewSeed) {
            if (yt == y_right && IsPixelValid(x, yt, old_color, new_color, boundary_color)) {
                stk.push(Point(x, yt));
                printf("push %d,%d\n", x, yt);
            } else {
                stk.push(Point(x, yt-1));
                printf("push %d,%d\n", x, yt-1);

            }
        }

        // skip invalid point in line
        int yspan = SkipInvalidInLine(x, yt, y_right, old_color, new_color, boundary_color);
        yt += (yspan == 0) ? 1 : yspan;
    }
}


