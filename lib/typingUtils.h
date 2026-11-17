#ifndef TYPINGUTILS_H
#define TYPINGUTILS_H

#include "termUtils.h"
#include <stdbool.h>
#include <stdlib.h>

// UNTYPED = 0 for calloc (just to be safe)
typedef enum { UNTYPED = 0, INCORRECT, CORRECT } CharState;

struct Colors {
  char *untyped;
  char *incorrect;
  char *correct;
};

// 3 size_t's are apparently too big to be 'safely' atomic
// which is important because eventually the stopwatch and main thread
// will be sharing one of these
struct UserStats {
  int correct;
  int typed;
  int textLen;
};

struct TypingInfo {
  int textLen;
  char *userInput;
  char *text;
  CharState *charStates;
};

extern const struct Colors COLORS;

extern int cheatThreshold;

void ansiPrint(int modes, ...);
void moveCursor(int row, int col);
void moveToCol(int col);
void hideCursor(void);
void showCursor(void);
void saveScreen(void);
void restoreScreen(void);
void eraseScreen(void);
void eraseCurToEnd(void);
void resetModes(void);
void setCheatThreshold(int threshold);
void handleCheat(void);
char *getCharColor(CharState charState);
void displayInput(int pos, int textOffset, const struct TypingInfo *data,
                  const struct WinSize *winSize);
void displayUserStats(const struct UserStats *userStats, double elapsedTime);

#endif
