#include "ansi.h"
#include "args.h"
#include "movement.h"
#include "stopwatch.h"
#include "termUtils.h"
#include "textGen.h"
#include "typingUtils.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

bool startGame(char *text, int textOffset, int maxBufSize);
void endGame(bool cheated);

int main(int argc, char *argv[]) {
  int cheatThreshold = 800;
  int maxBufSize = 4;

  long long tickMs = 1000 / 60;
  int statsTicks = 30;

  int textOffset = 4;
  struct TextSize defaultTsz = {
      .paragraphs = 2,
      .minSntn = 3,
      .maxSntn = 5,
      .minWordSntn = 4,
      .maxWordSntn = 6,
  };

  // setup
  setCheatThreshold(cheatThreshold);
  srand(time(NULL));
  char *text = parseArgs(argc, argv, &defaultTsz);
  init();

  // game itself
  startStopwatch(tickMs, statsTicks);
  int result = startGame(text, textOffset, maxBufSize);

  // cleanup
  killStopWatch();
  endGame(result);
  cleanup();
}

// main loop
// return 1 (true) if user cheated
bool startGame(char *text, int textOffset, int maxBufSize) {
  // lotta game info
  int size = strlen(text);
  char *input = calloc(size + 1, sizeof(char));
  CharState *charStates = calloc(size, sizeof(CharState));
  struct WinSize winSize = getWinSize();
  // +1 offset for page number
  struct DisplayInfo disp = {
      .textOffset = textOffset + 1, .rows = winSize.rows, .cols = winSize.cols};
  struct UserStats userStats = {.typed = 0, .correct = 0, .textLen = size};
  struct TypingInfo typing = {.textLen = size,
                              .currPage = 0,
                              .userInput = input,
                              .text = text,
                              .charStates = charStates};
  size_t validChars = getPages(&typing, &disp);
  getPage(&typing, &disp);

  // intial text (lock stdout from stopwatch before writing)
  flockfile(stdout);
  updateDisplay(0, &typing, &disp);
  fflush(stdout);
  funlockfile(stdout);

  size_t i = 0;
  size_t pageOffset = 0;
  int *pagePositions = calloc(disp.pages, sizeof(int));

  char c;
  bool escaped, left, right, up, down;
  bool isNextPage, isPrevPage, isBackspace, isPrintable;

  while (userStats.correct < validChars) {
    if (pollChar(&c, maxBufSize) == 1)
      continue;

    // ']' for next and '[' for prev page
    isBackspace = c == 127 || c == 8;
    isNextPage = c == 93 && !escaped;
    isPrevPage = c == 91 && !escaped;
    isPrintable = isprint(c) && !escaped;

    // catch arrow keys
    escaped = isEscaped(c);
    if (escaped) {
      // left = 'D' (68), right = 'C' (67)
      // up = 'A' (65), down = 'B' (66)
      left = c == 68;
      right = c == 67;
      up = c == 65;
      down = c == 66;
    }

    // move cursor
    if (escaped && (left || right || up || down)) {
      // woah function pointer
      MoveFunc moveFunc = left || right ? moveX : moveY;

      int dir = right || down ? 1 : -1;
      i = moveFunc(i, dir, &typing, &disp);

      left = right = up = down = false;
    }

    // navigate pages
    else if ((isNextPage || isPrevPage) && disp.pages >= 1) {
      // save cursor position
      pagePositions[typing.currPage] = i - pageOffset;

      int inc = isNextPage ? 1 : -1;
      typing.currPage = wrap(typing.currPage + inc, 0, disp.pages);
      getPage(&typing, &disp);

      // restore cursor position
      i = pageOffset = disp.pageOffsets[typing.currPage];
      i += pagePositions[typing.currPage];

      // erase current page from the screen
      flockfile(stdout);
      moveCursor(textOffset, 1);
      eraseCurToEnd();
      funlockfile(stdout);
    }

    // backspace
    else if (isBackspace && i - pageOffset > 0) {
      // handle deleting newlines first
      while (i - pageOffset > 0 && text[i - 1] == '\n')
        i--;

      // normal deletion
      if (i - pageOffset > 0) {
        i--;
        if (charStates[i] == CORRECT)
          userStats.correct--;
        userStats.typed--;
        charStates[i] = UNTYPED;
      }
    }

    // can be printed and within page
    else if (isPrintable && i - pageOffset < typing.pageSize) {
      // match chars
      if (c == text[i]) {
        charStates[i] = CORRECT;
        userStats.correct++;
      } else
        charStates[i] = INCORRECT;

      input[i] = c;
      i++;
      userStats.typed++;
    }

    // handle adding newlines
    while (i < size && text[i] == '\n')
      input[i++] = '\n';

    flockfile(stdout);
    updateDisplay(i, &typing, &disp);
    fflush(stdout);
    funlockfile(stdout);

    updateStats(&userStats);
    if (isCheating())
      break;
  }

  // cleanup
  free(text);
  free(input);
  free(charStates);
  free(pagePositions);
  free(disp.pageOffsets);
  free(disp.lineOffsets);

  return isCheating();
}

void endGame(bool cheated) {
  // clear screen for cheater
  if (cheated) {
    eraseScreen();
    moveCursor(1, 1);
  }

  printf("%s\n", cheated ? "You dirty cheater." : "\nDone!");
  fflush(stdout);
  sleep(2);
}
