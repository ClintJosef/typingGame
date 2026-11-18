#include "ansi.h"
#include "args.h"
#include "stopwatch.h"
#include "termUtils.h"
#include "textGen.h"
#include "typingUtils.h"
#include <ctype.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void startGame(char *text, int textOffset);
int clamp(int a, int min, int max);

int main(int argc, char *argv[]) {
  int textOffset = 4;
  long long swSleepMs = 500;
  int cheatThreshold = 1e6;
  struct TextSize defaultTsz = {
      .paragraphs = 2,
      .minSntn = 3,
      .maxSntn = 5,
      .minWordSntn = 4,
      .maxWordSntn = 6,
  };

  setCheatThreshold(cheatThreshold);
  srand(time(NULL));
  char *text = parseArgs(argc, argv, &defaultTsz);
  init();

  startStopwatch(swSleepMs);
  startGame(text, textOffset);

  killStopWatch();
  cleanup();
}

// main loop
void startGame(char *text, int textOffset) {
  // lotta game info
  int size = strlen(text);
  char *input = calloc(size + 1, sizeof(char));
  CharState *charStates = calloc(size, sizeof(CharState));
  struct WinSize winSize = getWinSize();
  // +1 offset for page number
  struct DisplayInfo disp = {
      .textOffset = textOffset + 1, .rows = winSize.rows, .cols = winSize.cols};
  struct UserStats userStats = {.typed = 0, .correct = 0, .textLen = size};
  struct TypingInfo typingInfo = {.textLen = size,
                                  .currPage = 0,
                                  .userInput = input,
                                  .text = text,
                                  .charStates = charStates};
  getPages(&typingInfo, &disp);
  typingInfo.page = getPage(&typingInfo, &disp);

  // intial text (lock from stopwatch before writing to stdout)
  flockfile(stdout);
  moveCursor(textOffset, 1);
  // always one extra page at the end that i cant get rid of for some reason
  printf("Page (%d/%d): Use arrow keys or '[' and ']'\n", typingInfo.currPage,
         disp.pages - 1);
  displayInput(0, &typingInfo, &disp);
  funlockfile(stdout);

  unsigned char c;
  int i = 0;
  int pageOffset = 0;
  bool isNextPage, isPrevPage, isBackspace, isPrintable, isMatching;
  while (userStats.correct < size) {
    // read from the provided input stream since stdin was "used up"
    c = fgetc(getInputStream());

    // ']' for next and '[' for prev page
    // or arrow keys
    isNextPage = c == 93;
    isPrevPage = c == 91;

    isBackspace = c == 127 || c == 8;
    isPrintable = isprint(c);
    isMatching = i < size && c == text[i];

    flockfile(stdout);

    // enter (advance page)
    if ((isNextPage || isPrevPage) && disp.pages > 1) {
      int inc = isNextPage ? 1 : -1;
      typingInfo.currPage = clamp(typingInfo.currPage + inc, 0, disp.pages);

      i = pageOffset = disp.pageOffsets[typingInfo.currPage];
      typingInfo.page = getPage(&typingInfo, &disp);

      eraseScreen();
    }

    // backspace
    else if (isBackspace && i - pageOffset > 0) {
      // handle deleting newlines first
      while (i > 0 && text[i - 1] == '\n') {
        i--;
        userStats.correct--;
        userStats.typed--;
        charStates[i] = UNTYPED;
      }

      // normal deletion
      i--;
      userStats.typed--;
      if (charStates[i] == CORRECT)
        userStats.correct--;
      charStates[i] = UNTYPED;
    }

    // can be printed
    else if (isPrintable && i < size) {
      // match chars
      if (isMatching) {
        charStates[i] = CORRECT;
        userStats.correct++;
      } else
        charStates[i] = INCORRECT;
      input[i] = c;
      i++;
      userStats.typed++;
    }

    // handle adding newlines
    while (!(isNextPage || isPrevPage) && i < size && text[i] == '\n') {
      userStats.correct++;
      userStats.typed++;
      input[i] = '\n';
      i++;
    }

    // always gotta have the null byte
    input[i] = '\0';

    updateStats(&userStats);

    moveCursor(textOffset, 1);
    printf("Page (%d/%d): Navigate with arrow keys or '[' and ']'\n",
           typingInfo.currPage, disp.pages - 1);
    displayInput(i, &typingInfo, &disp);
    fflush(stdout);

    funlockfile(stdout);
  }

  printf("\nDone!");
  sleep(1);

  // cleanup
  free(input);
  free(charStates);
}

// exclusive max
int clamp(int a, int min, int max) {
  if (a >= max)
    return min;
  if (a < min)
    return max - 1;
  return a;
}
