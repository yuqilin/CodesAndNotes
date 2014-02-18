#include "ColorIt.h"
#include <stack>

struct Point {
    int x;
    int y;

    Point(int a, int b) {
        x = a;
        y = b;
    }
};


typedef std::stack<Point> PointStack;

static int** s_ImageArray = 0;
static int s_ImageWidth = 0;
static int s_ImageHeight = 0;

static int s_BoundaryColor = 1;
static int s_OldColor = 0;
static int s_NewColor = 0;

int FillLineRight(int x, int y, int old_color, int new_color, int boundary_color);
int FillLineLeft(int x, int y, int old_color, int new_color, int boundary_color);
void SearchLineNewSeed(PointStack& stk, int y_left, int y_right, int x, int old_color, int new_color, int boundary_color);
bool IsPixelValid(int x, int y, int old_color, int new_color, int boundary_color);
int SkipInvalidInLine(int x, int y, int x_right, int old_color, int new_color, int boundary_color);
void DrawPixelColor(int x, int y, int color);
int GetPixelColor(int x, int y);

void SetImage(int** image_array, int width, int height) {
    s_ImageArray = image_array;
    s_ImageWidth = width;
    s_ImageHeight = height;
}

void ScanLineSeedFill(int x, int y, int old_color, int new_color, int boundary_color) {
    PointStack stk;

    s_OldColor = old_color;
    s_NewColor = new_color;
    s_BoundaryColor = boundary_color;

    // 1. push seed into stack
    stk.push(Point(x, y));

    while (!stk.empty()) {
        // 2. current seed
        Point seed = stk.top();
        stk.pop();

        // 3. fill right-hand
        int count = FillLineRight(seed.x, seed.y, old_color, new_color, boundary_color);
        if (count == 0)
        {
            int a = 0;
        }
        int y_right = seed.y + count - 1;
        // 4. fill left-hand
        count = FillLineLeft(seed.x, seed.y-1, old_color, new_color, boundary_color);
        int y_left = seed.y - count;

        if (y_left == y_right)
        {
            int a = 1;
        }
        else
        {
            printf("%d %d\n", y_left, y_right);
        }

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

bool IsPixelValid(int x, int y, int old_color, int new_color, int boundary_color) {
    bool bValid = false;

    int color = GetPixelColor(x, y);
    if (color == old_color && color != new_color && color != boundary_color) {
        bValid = true;
    }

    return bValid;
}

int FillLineRight(int x, int y, int old_color, int new_color, int boundary_color) {
    int count = 0;
    while (y <= s_ImageWidth && IsPixelValid(x, y, old_color, new_color, boundary_color)) {
        DrawPixelColor(x, y, new_color);
        y++;
        count++;
    }
    return count;
}

int FillLineLeft(int x, int y, int old_color, int new_color, int boundary_color) {
    int count = 0;
    while (y >= 0 && IsPixelValid(x, y, old_color, new_color, boundary_color)) {
        DrawPixelColor(x, y, new_color);
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

// xÐÐyÁÐ
int GetPixelColor(int x, int y) {
    int color = -1;
    if (x < s_ImageHeight && y < s_ImageWidth) {
        color = *((int*)s_ImageArray + s_ImageWidth * x + y);
    }
    return color;
}

void DrawPixelColor(int x, int y, int color) {
    if (x < s_ImageWidth && y < s_ImageHeight) {
        *((int*)s_ImageArray + s_ImageWidth * x + y) = color;
    }
    //PrintImagePixel();
}

void PrintImagePixel() {
    printf("\n");
    int x, y;
    for (x=0; x<s_ImageHeight; x++) {
        for (y=0; y<s_ImageWidth; y++) {
            printf("%d ", GetPixelColor(x, y));
        }
        printf("\n");
    }
}