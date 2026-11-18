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

int main(int argc, char *argv[]) {
  int textOffset = 4;
  long long swSleepMs = 500;
  int cheatThreshold = 1000;
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
  int size = strlen(text);

  struct WinSize winSize = getWinSize();

  // user input
  char *input = calloc(size + 1, sizeof(char));
  struct UserStats userStats = {.typed = 0, .correct = 0, .textLen = size};

  // used for char-by-char text coloring
  CharState *charStates = calloc(size, sizeof(CharState));
  struct TypingInfo typingInfo = {.textLen = size,
                                  .userInput = input,
                                  .text = text,
                                  .charStates = charStates};

  // make sure stdout is safe to write to for this (main) thread
  flockfile(stdout);

  // initial text (all untyped)
  moveCursor(textOffset, 1);
  displayInput(0, textOffset, &typingInfo, &winSize);

  // now allow other threads (stopwatch) to write to it
  funlockfile(stdout);

  int c;
  int i = 0;
  bool isBackspace, isPrintable, isMatching;
  while (userStats.correct < size) {
    // read from the provided input stream since stdin was "used up"
    c = fgetc(getInputStream());

    if (c == EOF)
      break;

    isBackspace = c == 127 || c == 8;
    isPrintable = isprint((unsigned char)c);
    isMatching = i < size && c == text[i];

    // backspace
    if (isBackspace && i > 0) {
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
    while (i < size && text[i] == '\n') {
      userStats.correct++;
      userStats.typed++;
      input[i] = '\n';
      i++;
    }

    // always gotta have the null byte
    input[i] = '\0';

    updateStats(&userStats);

    flockfile(stdout);
    moveCursor(textOffset, 1);
    displayInput(i, textOffset, &typingInfo, &winSize);
    funlockfile(stdout);
  }

  printf("\nDone!");
  sleep(1);

  // cleanup
  free(input);
  free(charStates);
}
