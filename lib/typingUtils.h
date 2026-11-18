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

struct TypingStats {
  double wpm;
  double avgWpm;
  double progress;
  double accuracy;
};

struct TypingInfo {
  int textLen;
  int currPage;
  char *userInput;
  char *text;
  char *page;
  CharState *charStates;
};

struct DisplayInfo {
  int textOffset;
  int pages;
  int *pageOffsets;
  int rows;
  int cols;
};

extern const struct Colors COLORS;

extern int cheatThreshold;

void setCheatThreshold(int threshold);
void handleCheat(void);
char *getCharColor(CharState charState);
void getPages(const struct TypingInfo *typing, struct DisplayInfo *disp);
char *getPage(const struct TypingInfo *typing, struct DisplayInfo *disp);
void updateDisplay(int pos, const struct TypingInfo *typing,
                   const struct DisplayInfo *disp);
void displayPageInfo(const struct TypingInfo *typing,
                     const struct DisplayInfo *disp);
void displayInput(int pos, const struct TypingInfo *typing,
                  const struct DisplayInfo *disp);
void displayTypingStats(const struct TypingStats *tStats);
void calculateTypingStats(const struct UserStats *userStats,
                          struct TypingStats *tStats, double elapsedTime);

#endif
