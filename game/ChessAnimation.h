#pragma once
#include <windows.h>
#include <gdiplus.h>

class ChessAnimation {
public:
    bool animating = false;
    int fromRow = -1, fromCol = -1;
    int toRow = -1, toCol = -1;
    unsigned int startTime = 0;
    unsigned int duration = 100;

    void start(int fr, int fc, int tr, int tc, unsigned int now);
    void reset();
    bool update();
    float progress() const;
};