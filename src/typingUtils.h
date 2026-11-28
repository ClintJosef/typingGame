#ifndef TYPINGUTILS_H
#define TYPINGUTILS_H

#include "termUtils.h"
#include <stdbool.h>

// UNTYPED = 0 for calloc (just to be safe)
typedef enum { UNTYPED = 0, INCORRECT, CORRECT } CharState;

typedef struct {
  const char *untyped;
  const char *incorrect;
  const char *correct;
} Colors;

// 3 size_t's are apparently too big to be 'safely' atomic
// which is important because eventually the stopwatch and main thread
// will be sharing one of these
typedef struct {
  int correct;
  int typed;
  int textLen;
} UserStats;

typedef struct {
  double wpm;
  double avgWpm;
  double progress;
  double accuracy;
} TypingStats;

typedef struct {
  char *userInput;
  char *text;
  size_t textLen;
  int currPage;
  int pageSize;
  char *page;
  CharState *charStates;
} TypingInfo;

typedef struct {
  int textOffset;
  int pages;
  int lines;
  int pageHeight;
  int *pageLines;
  size_t *pageOffsets;
  size_t *lineOffsets;
  int rows;
  int cols;
} DisplayInfo;

extern const Colors COLORS;

void setCheatThreshold(int threshold);
void setCheating(void);
bool isCheating(void);

const char *getCharColor(CharState charState);
size_t getPages(const TypingInfo *typing, DisplayInfo *disp);
void getPage( TypingInfo *typing, const DisplayInfo *disp);
int getPageSize(const TypingInfo *typing, const DisplayInfo *disp);
int findLine(size_t pos, const DisplayInfo *disp);
int getLineLength(int line, size_t textLen, const DisplayInfo *disp);

void updateDisplay(size_t pos, const TypingInfo *typing,
                   const DisplayInfo *disp);
void displayPageInfo(const TypingInfo *typing, const DisplayInfo *disp);
void displayInput(size_t pos, const TypingInfo *typing,
                  const DisplayInfo *disp);
void displayTypingStats(const TypingStats *tStats);

void calculateTypingStats(const UserStats *userStats, TypingStats *tStats,
                          double elapsedTime);

#endif
