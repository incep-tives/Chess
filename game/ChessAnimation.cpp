#include "ChessAnimation.h"
#include <windows.h>

void ChessAnimation::start(int fr, int fc, int tr, int tc, unsigned int now) {
    fromRow = fr;
    fromCol = fc;
    toRow = tr;
    toCol = tc;
    startTime = now;
    animating = true;
}

void ChessAnimation::reset() {
    animating = false;
    fromRow = fromCol = toRow = toCol = -1;
    startTime = 0;
}

bool ChessAnimation::update() {
    if (!animating) return false;
    unsigned int now = GetTickCount();
    unsigned int elapsed = now - startTime;
    if (elapsed >= duration) {
        animating = false;
        return false;
    }
    return true;
}

float ChessAnimation::progress() const {
    if (!animating) return 1.0f;
    unsigned int now = GetTickCount();
    unsigned int elapsed = now - startTime;
    if (elapsed >= duration) return 1.0f;
    return static_cast<float>(elapsed) / duration;
}