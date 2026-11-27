#include "typingUtils.h"
#include "ansi.h"
#include "termUtils.h"
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
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

const char *getCharColor(CharState charState) {
  switch (charState) {
  case INCORRECT:
    return COLORS.incorrect;
    break;
  case CORRECT:
    return COLORS.correct;
    break;
  default:
    return COLORS.untyped;
  }
}

// basically calculates text metadata (DisplayInfo)
// returns the count of non-newline characters
size_t getPages(const struct TypingInfo *typing, struct DisplayInfo *disp) {
  int pageHeight = disp->rows - disp->textOffset - 1;
  size_t len = typing->textLen;
  size_t maxPages = len / pageHeight;

  size_t *lineOffsets = calloc(len, sizeof(size_t));
  size_t *pageOffsets = calloc(maxPages, sizeof(size_t));
  int *pageLines = calloc(maxPages, sizeof(int));

  // page 0 line 0 at offset of 0
  // count up from there
  int pages = 1;
  int lines = 1;
  int col = 0;

  // all non-newline chars
  size_t charCount = 0;

  for (size_t i = 0; i < len; i++) {
    bool eol = false;

    // eol through newline char
    if (typing->text[i] == '\n')
      eol = true;
    else {
      col++;
      charCount++;

      // eol through line wraparound (cols reached)
      eol = col >= disp->cols;
    }

    // end of line
    if (eol) {
      col = 0;

      // next line/page start
      size_t offset = i + 1;

      // next page
      if (pageLines[pages - 1] >= pageHeight) {
        pageOffsets[pages] = offset;
        pageLines[pages] = 0;
        pages++;
      }
      // next line
      else
        pageLines[pages - 1]++;

      lines++;
      lineOffsets[lines - 1] = offset;
    }
  }

  // since pages start at page 0, these are both 1 extra by the end of the loop
  disp->pages = pages - 1;
  disp->lines = lines - 1;

  disp->pageOffsets = pageOffsets;
  disp->lineOffsets = lineOffsets;
  disp->pageLines = pageLines;
  disp->pageHeight = pageHeight;

  return charCount;
}

// update current page
void getPage(struct TypingInfo *typing, const struct DisplayInfo *disp) {
  int currPage = typing->currPage;
  size_t offset = disp->pageOffsets[currPage];
  typing->page = &typing->text[offset];
  int pageSize;

  // only one page -> just the whole text length
  if (disp->pages < 1)
    pageSize = typing->textLen;

  // last page -> start of this page up to the end of the text
  else if (currPage == disp->pages)
    pageSize = typing->textLen - disp->pageOffsets[currPage];

  // between 1 and {pages} -> start of this page up to the next
  else
    pageSize = disp->pageOffsets[currPage + 1] - disp->pageOffsets[currPage];

  typing->pageSize = pageSize;
}

// find line based on index using binary search
// works because an offset array is already sorted
int findLine(size_t pos, const struct DisplayInfo *disp) {
  int left = 0;
  int right = disp->lines;
  int mid;

  while (left <= right) {
    mid = (left + right) / 2;

    if (disp->lineOffsets[mid] <= pos)
      left = mid + 1;
    else
      right = mid - 1;
  }

  // return last line whose offset <= pos
  return left - 1;
}

int getLineLength(int line, size_t textLen, const struct DisplayInfo *disp) {
  size_t start = disp->lineOffsets[line];
  size_t end = line < disp->lines ? disp->lineOffsets[line + 1] : textLen;

  return end - start;
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
  static char fmt[] = "\rPage: %d/%d";
  static char controls[] =
      " | Navigate with '[' or ']' (pages), ← ↑ → ↓ (cursor)\n";

  printf(fmt, typing->currPage + 1, disp->pages + 1);
  printf("%s", controls);
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
