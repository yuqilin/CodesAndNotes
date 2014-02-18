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

static int s_ImageWidth = 16;
static int s_ImageHeight = 16;

static int s_ImageArray[s_ImageWidth][s_ImageHeight] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0,
    0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0,
    0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int FillLineRight(int x, int y, int old_color, int new_color, int boundary_color);
int FillLineLeft(int x, int y, int old_color, int new_color, int boundary_color);
void SearchLineNewSeed(PointStack& stk, int x_left, int x_right, int y, int old_color, int new_color, int boundary_color);
bool IsPixelValid(int x, int y, int old_color, int new_color, int boundary_color);
int SkipInvalidInLine(int x, int y, int x_right, int old_color, int new_color, int boundary_color);
void DrawPixelColor(int x, int y, int color);
int GetPixelColor(int x, int y);
void PrintImagePixel();

void ScanLineSeedFill(int x, int y, int old_color, int new_color, int boundary_color) {
    PointStack stk;

    int x_left = 0, x_right = 0;

    // 1. push seed into stack
    stk.push(Point(x, y));

    while (!stk.empty()) {
        // 2. current seed
        Point seed = stk.top();
        stk.pop();

        // 3. fill right-hand
        int count = FillLineRight(seed.x, seed.y, old_color, new_color, boundary_color);
        int x_right = seed.x + count - 1;
        // 4. fill left-hand
        count = FillLineLeft(seed.x, seed.y, old_color, new_color, boundary_color);
        int x_left = seed.x - count;

        // 5. scan adjacent line
        SearchLineNewSeed(stk, x_left, x_right, seed.y-1, old_color, new_color, boundary_color);
        SearchLineNewSeed(stk, x_left, x_right, seed.y+1, old_color, new_color, boundary_color);
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
    while (x <= s_ImageHeight && IsPixelValid(x, y, old_color, new_color, boundary_color)) {
        DrawPixelColor(x, y, new_color);
        x++;
        count++;
    }
    return count;
}

int FillLineLeft(int x, int y, int old_color, int new_color, int boundary_color) {
    int count = 0;
    while (x >= 0 && IsPixelValid(x, y, old_color, new_color, boundary_color)) {
        DrawPixelColor(x, y, new_color);
        x--;
        count++;
    }
    return count;
}

int SkipInvalidInLine(int x, int y, int x_right, int old_color, int new_color, int boundary_color) {
    int count = 0;

    while (x <= x_right && IsPixelValid(x, y, old_color, new_color, boundary_color)) {
        x++;
        count++;
    }

    return count;
}

void SearchLineNewSeed(PointStack& stk, int x_left, int x_right, int y,
                       int old_color, int new_color, int boundary_color) {
    int xt = x_left;
    bool findNewSeed = false;

    while (xt <= x_right) {
        findNewSeed = false;
        while (IsPixelValid(xt, y, old_color, new_color, boundary_color) && (xt < x_right)) {
            findNewSeed = true;
            xt++;
        }
        if (findNewSeed) {
            if (IsPixelValid(xt, y, old_color, new_color, boundary_color) && (xt == x_right)) {
                stk.push(Point(xt, y));
            } else {
                stk.push(Point(xt-1, y));
            }
        }

        // skip invalid point in line
        int xspan = SkipInvalidInLine(xt, y, x_right, old_color, new_color, boundary_color);
        xt += (xspan == 0) ? 1 : xspan;
    }
}

int GetPixelColor(int x, int y) {
    int color = 0;
    if (x < s_ImageWidth && y < s_ImageHeight) {
        color = s_ImageArray[x][y];
    }
    return color;
}

void DrawPixelColor(int x, int y, int color) {
    if (x < s_ImageWidth && y < s_ImageHeight) {
        s_ImageArray[x][y] = color;
    }
}

void PrintImagePixel() {
    int x, y;
    for (x=0; x<s_ImageWidth; x++) {
        for (y=0; y<s_ImageHeight; y++) {
            printf("%d ", s_ImageArray[x][y]);
        }
        printf("\n");
    }
}