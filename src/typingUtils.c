#include "typingUtils.h"
#include "ansi.h"
#include "termUtils.h"
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

const struct Colors COLORS = {
    .untyped = "\033[2m", .incorrect = "\033[31m", .correct = "\033[38;5;76m"};

// max wpm limit before it thinks user is cheating (can be set)
static int cheatThreshold = INT32_MAX;
static _Atomic(bool) cheating = false;

void setCheatThreshold(int threshold) { cheatThreshold = threshold; }
void setCheating(void) { cheating = true; }
bool isCheating(void) { return cheating; }
void handleCheat(void) {
  // clear screen
  eraseScreen();
  moveCursor(1, 1);

  // the line
  printf("You dirty cheater.");
  fflush(stdout);
  sleep(2);
}

const char *getCharColor(CharState charState) {
  switch (charState) {
  case INCORRECT:
    return COLORS.incorrect;
    break;
  case CORRECT:
    return COLORS.correct;
    break;
  case UNTYPED:
    return COLORS.untyped;
    break;
  }
}

// calculates page offsets based on window size and newline characters
// returns the count of non-newline characters
size_t getPages(const struct TypingInfo *typing, struct DisplayInfo *disp) {
  int pagePos = 0;
  int currPage = 1;
  int pageSize = disp->cols * (disp->rows - disp->textOffset);
  // offset of each page in the text
  // page 0 with offset of 0
  size_t *pageOffsets = calloc(1, sizeof(size_t));

  size_t i = 0;
  size_t charCount = 0;

  while (i < typing->textLen) {
    // a row is winSize.cols long.
    // if the newline char is in the middle of the line, it still advances to
    // the next row, but only to the first column
    // so just add winSize.cols minus the cursor's current pos in the line
    if (typing->text[i] == '\n')
      pagePos += disp->cols - (pagePos % disp->cols);

    // normal char
    else {
      pagePos++;
      charCount++;
    }

    i++;
    // crossing a page
    if (pagePos >= pageSize) {
      pageOffsets = realloc(pageOffsets, (currPage + 1) * sizeof(size_t));
      pageOffsets[currPage++] = i;
      pagePos %= pageSize;
    }
  }
  // currPage always 1 more by the end of the loop
  disp->pages = currPage - 1;
  disp->pageOffsets = pageOffsets;

  return charCount;
}

// update current page
void getPage(struct TypingInfo *typing, const struct DisplayInfo *disp) {
  int currPage = typing->currPage;
  size_t offset = disp->pageOffsets[currPage];
  typing->page = &typing->text[offset];

  // only one page -> just the whole text length
  if (disp->pages < 1)
    typing->pageSize = typing->textLen;

  // last page -> start of this page up to the end of the text
  else if (currPage == disp->pages)
    typing->pageSize = typing->textLen - disp->pageOffsets[currPage];

  // between 1 and {pages} -> start of this page up to the next
  else
    typing->pageSize =
        disp->pageOffsets[currPage + 1] - disp->pageOffsets[currPage];
}

// display all text-related stuff
void updateDisplay(size_t pos, const struct TypingInfo *typing,
                   const struct DisplayInfo *disp) {
  moveCursor(disp->textOffset - 1, 1);
  if (disp->pages >= 1)
    displayPageInfo(typing, disp);
  displayInput(pos, typing, disp);
}

// colorizes each character based their typing state
// does not write past the screen
void displayInput(size_t pos, const struct TypingInfo *typing,
                  const struct DisplayInfo *disp) {
  // assuming getPage was called
  char *text = typing->page;
  int currPage = typing->currPage;
  int pageSize = typing->pageSize;
  size_t userOffset = disp->pageOffsets[currPage];

  size_t userPos;
  bool atCursor, untyped, covered;

  // skip trailing newlines
  int start = 0;
  while (text[start] == '\n') {
    start++;
  }
  if (pos - userOffset < start) {
    pos += start;
  }

  for (int i = start; i < pageSize; i++) {
    userPos = i + userOffset;

    atCursor = userPos == pos;
    untyped = typing->charStates[userPos] == UNTYPED;
    covered = isblank(typing->userInput[userPos]) && !isblank(text[i]);

    // custom cursor
    if (atCursor)
      printf("%s%s%c", BOLD, UNDERLINE, text[i]);

    // colorize user userInput (green/red)
    else if (!untyped && !covered)
      printf("%s%c", getCharColor(typing->charStates[userPos]),
             typing->userInput[userPos]);

    // show original text if untyped
    // or if it wouldve been covered by a space
    else
      printf("%s%s%c", ITALIC, FAINT, text[i]);

    resetModes();
  }
}

void displayPageInfo(const struct TypingInfo *typing,
                     const struct DisplayInfo *disp) {
  static char fmt[] = "Page (%d/%d): Use '[' or ']'\n";
  printf(fmt, typing->currPage + 1, disp->pages + 1);
}

void displayTypingStats(const struct TypingStats *tStats) {
  static char fmt[] =
      "\rWpm: %.0lf, Avg Wpm: %.0lf, Accuracy: %.0lf%%, Progress: %.0lf%%";
  eraseLine();
  printf(fmt, tStats->wpm, tStats->avgWpm, tStats->accuracy, tStats->progress);
}

void calculateTypingStats(const struct UserStats *userStats,
                          struct TypingStats *tStats, double elapsedTime) {
  // pretty useful
  static double prevTime = 0;
  static double prevTyped = 0;

  // for delta wpm
  double dt = elapsedTime - prevTime;
  double dTyped = userStats->typed - prevTyped;

  // values are saved for the next function call
  prevTime = elapsedTime;
  prevTyped = userStats->typed;

  // arbitrary 5-letter words per minute
  tStats->wpm = (double)dTyped / 5 / dt * 60;
  tStats->avgWpm = (double)userStats->typed / 5 / elapsedTime * 60;

  tStats->accuracy = (double)userStats->correct / userStats->typed * 100;
  tStats->progress = (double)userStats->typed / userStats->textLen * 100;

  // exit if user is "typing" too fast
  if (tStats->wpm >= cheatThreshold)
    setCheating();
}
