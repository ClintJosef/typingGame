// functions to translate text indices to window positions and move them
#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "typingUtils.h"

int wrap(int a, int min, int max);
size_t moveX(size_t pos, int dir, const struct TypingInfo *typing,
             const struct DisplayInfo *disp);
size_t moveY(size_t pos, int dir, const struct TypingInfo *typing,
             const struct DisplayInfo *disp);

#endif
