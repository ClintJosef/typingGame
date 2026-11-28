// functions to translate text indices to window positions and move them
#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "typingUtils.h"

typedef size_t (*MoveFunc)(size_t, int, const TypingInfo *,
                           const DisplayInfo *);

int wrap(int a, int min, int max);
size_t moveX(size_t pos, int dir, const TypingInfo *typing,
             const DisplayInfo *disp);
size_t moveY(size_t pos, int dir, const TypingInfo *typing,
             const DisplayInfo *disp);

#endif
