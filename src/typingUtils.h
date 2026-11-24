#ifndef TYPINGUTILS_H
#define TYPINGUTILS_H

#include "termUtils.h"
#include <stdbool.h>

// UNTYPED = 0 for calloc (just to be safe)
typedef enum { UNTYPED = 0, INCORRECT, CORRECT } CharState;

struct Colors {
  const char *untyped;
  const char *incorrect;
  const char *correct;
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
  char *userInput;
  char *text;
  size_t textLen;
  int currPage;
  int pageSize;
  char *page;
  CharState *charStates;
};

struct DisplayInfo {
  int textOffset;
  int pages;
  int lines;
  size_t *pageOffsets;
  size_t *lineOffsets;
  int rows;
  int cols;
};

extern const struct Colors COLORS;

void setCheatThreshold(int threshold);
void setCheating(void);
bool isCheating(void);
void handleCheat(void);
const char *getCharColor(CharState charState);
size_t getPages(const struct TypingInfo *typing, struct DisplayInfo *disp);
void getPage(struct TypingInfo *typing, const struct DisplayInfo *disp);
int getPageSize(const struct TypingInfo *typing,
                const struct DisplayInfo *disp);
int findLine(size_t pos, const struct DisplayInfo *disp);
int getLineLength(int line, const struct DisplayInfo *disp);
void updateDisplay(size_t pos, const struct TypingInfo *typing,
                   const struct DisplayInfo *disp);
void displayPageInfo(const struct TypingInfo *typing,
                     const struct DisplayInfo *disp);
void displayInput(size_t pos, const struct TypingInfo *typing,
                  const struct DisplayInfo *disp);
void displayTypingStats(const struct TypingStats *tStats);
void calculateTypingStats(const struct UserStats *userStats,
                          struct TypingStats *tStats, double elapsedTime);

#endif
