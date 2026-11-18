#include "typingUtils.h"
#include "ansi.h"
#include "termUtils.h"
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const struct Colors COLORS = {
    .untyped = "\033[2m", .incorrect = "\033[31m", .correct = "\033[38;5;76m"};

// max wpm limit before it thinks user is cheating (can be set)
int cheatThreshold = INT32_MAX;

void setCheatThreshold(int threshold) { cheatThreshold = threshold; }
void handleCheat(void) {
  // clear screen
  eraseScreen();
  moveCursor(1, 1);

  // the line
  printf("You dirty cheater.");
  fflush(stdout);
  sleep(2);

  // clear again and signal to restore term
  eraseScreen();
  moveCursor(1, 1);
  killpg(0, SIGTERM);
}

char *getCharColor(CharState charState) {
  char *color;
  switch (charState) {
  case INCORRECT:
    color = COLORS.incorrect;
    break;
  case CORRECT:
    color = COLORS.correct;
    break;
  case UNTYPED:
    color = COLORS.untyped;
    break;
  }

  return color;
}

void getPages(const struct TypingInfo *typing, struct DisplayInfo *disp) {
  int pagePos = 0;
  int currPage = 1;
  int pageSize = disp->cols * (disp->rows - disp->textOffset);
  // offset of each page in the text
  // page 0 with offset of 0
  int *pageOffsets = calloc(1, sizeof(int));
  int i = 0;

  while (i < typing->textLen) {
    // a row is winSize.cols long.
    // if the newline char is in the middle of the line, it still advances to
    // the next row, but only to the first column
    // so just add winSize.cols minus the cursor's current pos in the line
    if (typing->text[i] == '\n')
      pagePos += disp->cols - (pagePos % disp->cols);

    // cutoff text before running out of terminal space
    else
      pagePos++;

    i++;

    // crossing a page
    if (pagePos >= pageSize) {
      pagePos = 0;

      pageOffsets = realloc(pageOffsets, sizeof(&pageOffsets) + sizeof(int));
      pageOffsets[currPage++] = i;
    }
  }
  if (pagePos < pageSize)
    pageOffsets[currPage] = i;
  disp->pages = currPage;

  disp->pageOffsets = pageOffsets;
}

// return page slice of text
char *getPage(const struct TypingInfo *typing, struct DisplayInfo *disp) {
  int offset = disp->pageOffsets[typing->currPage];

  return &typing->text[offset];
}

// colorizes each character based their status
// does not write past the screen
void displayInput(int pos, const struct TypingInfo *typing,
                  const struct DisplayInfo *disp) {
  char *text = typing->page;
  int currPage = typing->currPage;
  int userOffset = disp->pageOffsets[currPage];

  int textSize;
  if (disp->pages <= 1)
    textSize = typing->textLen;
  else
    textSize = disp->pageOffsets[currPage + 1] - disp->pageOffsets[currPage];

  bool atCursor, untyped, covered;
  int userPos;

  // skip trailing newlines
  int start = 0;
  for (; text[start] == '\n'; start++)
    ;

  for (int i = start; i < textSize; i++) {
    userPos = i + userOffset;

    atCursor = userPos - start == pos;
    untyped = typing->charStates[userPos] == UNTYPED;
    covered = isblank(typing->userInput[userPos]) && !isblank(text[i]);

    // custom cursor
    if (atCursor)
      printf("%s%s%c", BOLD, UNDERLINE, text[i]);

    // colorize user userInput (green/red)
    else if (!untyped && !covered)
      printf("%s%c", getCharColor(typing->charStates[userPos]),
             typing->userInput[userPos]);

    // show faint text if user inputted a space/tab over it
    // or if its not typed yet
    else
      printf("%s%c", FAINT, text[i]);

    resetModes();
  }
}

void displayTypingStats(const struct TypingStats *tStats) {
  printf("%sWpm: %.0lf, Avg Wpm: %.0lf, Accuracy: %.0lf%%, Progress: %.0lf%%",
         ERASE_LINE, tStats->wpm, tStats->avgWpm, tStats->accuracy,
         tStats->progress);
}

void calculateTypingStats(const struct UserStats *userStats,
                          struct TypingStats *tStats, double elapsedTime) {
  // pretty useful
  static double prevTime = 0;
  static double prevTyped = 0;

  // for delta wpm
  double dt = elapsedTime - prevTime;
  double dTyped = userStats->typed - prevTyped;

  // arbitrary 5-letter words per minute
  tStats->wpm = (double)dTyped / 5 / dt * 60;
  tStats->avgWpm = (double)userStats->typed / 5 / elapsedTime * 60;

  tStats->accuracy = (double)userStats->correct / userStats->typed * 100;
  tStats->progress = (double)userStats->correct / userStats->textLen * 100;

  // values are saved for the next function call
  prevTime = elapsedTime;
  prevTyped = userStats->typed;

  // exit if user is "typing" too fast
  if (tStats->wpm >= cheatThreshold) {
    handleCheat();
    return;
  }
}
