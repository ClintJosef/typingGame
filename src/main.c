#include "ansi.h"
#include "args.h"
#include "stopwatch.h"
#include "termUtils.h"
#include "textGen.h"
#include "typingUtils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int startGame(char *text, int textOffset, int maxBufSize);
static int wrap(int a, int min, int max);

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
  if (result)
    handleCheat();
  else {
    printf("\nDone!\n");
    fflush(stdout);
    sleep(1);
  }
  cleanup();
}

// main loop
// return 0 if user won normally, 1 if the user cheated
int startGame(char *text, int textOffset, int maxBufSize) {
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
  size_t validChars = getPages(&typingInfo, &disp);
  getPage(&typingInfo, &disp);

  // intial text (lock stdout from stopwatch before writing)
  flockfile(stdout);
  updateDisplay(0, &typingInfo, &disp);
  fflush(stdout);
  funlockfile(stdout);

  int i = 0;
  size_t pageOffset = 0;
  int *pagePositions = calloc(disp.pages, sizeof(int));

  char c;
  bool escaped, left, right, up, down;
  bool isNextPage, isPrevPage, isBackspace, isPrintable;

  while (userStats.correct < validChars) {
    if (pollChar(&c, maxBufSize) == 1)
      continue;

    // ']' for next and '[' for prev page
    isBackspace = (c == 127 || c == 8);
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
    if (left || right) {
      int inc;

      // proceeding newlines are handled automatically
      if (right)
        inc = 1;
      // handle preceeding newlines
      else {
        inc = -1;
        while (text[i + inc] == '\n')
          inc--;
      }

      i = wrap(i + inc, pageOffset, pageOffset + typingInfo.pageSize - 1);
      left = right = false;

    } else if (up || down) {
      // yes i stored all the line offsets just for this
      int line = findLine(i, &disp);
      int col = i - disp.lineOffsets[line];

      // next/prev line with the same column
      // for some reason has to move to the 2nd last line when going up
      int inc = down ? 1 : -2;
      int newLine = wrap(line + inc, 0, disp.lines);

      i = disp.lineOffsets[newLine] + col;
      i = wrap(i, pageOffset, pageOffset + typingInfo.pageSize - 1);
      up = down = false;
    }

    // navigate pages
    else if ((isNextPage || isPrevPage) && disp.pages >= 1) {
      // save cursor position
      pagePositions[typingInfo.currPage] = i - pageOffset;

      int inc = isNextPage ? 1 : -1;
      typingInfo.currPage = wrap(typingInfo.currPage + inc, 0, disp.pages);
      getPage(&typingInfo, &disp);

      // restore cursor position
      i = pageOffset = disp.pageOffsets[typingInfo.currPage];
      i += pagePositions[typingInfo.currPage];

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
    else if (isPrintable && i - pageOffset < typingInfo.pageSize) {
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
    while (i < size && text[i] == '\n') {
      input[i] = '\n';
      i++;
    }

    flockfile(stdout);
    updateDisplay(i, &typingInfo, &disp);
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

  // used to guard copy-paste but since that doesnt work anymore, bots maybe?
  return isCheating();
}

// wrap around a range using modulo
// didnt know where else to put this
static int wrap(int a, int min, int max) {
  int range = max - min + 1;
  /*
  1. (a - min) % range ensures a > -range
  2. + range ensures a >= 0
  3. % range ensures 0 <= a < range
  4. + min ensures min <= a < range + min

  since range is inclusive of max, this becomes:
  min <= a <= max
  */
  return min + ((a - min) % range + range) % range;
}
