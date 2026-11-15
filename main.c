#include "lib/setup.h"
#include "lib/stopwatch.h"
#include "lib/textGen.h"
#include "lib/typingUtils.h"
#include <ctype.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage(void);
char *parseArgs(int argc, char *argv[], struct TextSize *defaultTsz);
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

  init();
  srand(time(NULL));

  char *text = parseArgs(argc, argv, &defaultTsz);
  setCheatThreshold(cheatThreshold);
  startStopwatch(swSleepMs);
  startGame(text, textOffset);

  killStopWatch();
  cleanup();
}

void usage(void) {
  char *msg =
      "Invalid args.\nUsage: typingGame [--paragraphs --minSntn --maxSntn "
      "--minWords "
      "--maxWords]\nDefaults: --paragraphs=2 --minSntn=3 --maxSntn=5 "
      "--minWord=4 --maxWord=6";
  printf("%s", msg);
  fflush(stdout);
  kill(0, SIGTERM);
}

// parse every arg into one string, separated by spaces
char *parseArgs(int argc, char *argv[], struct TextSize *defaultTsz) {
  // fall back to default text if no args are given
  if (argc < 2)
    // have to do this to avoid "disqualifying" const
    return generateText((struct TextSize *)defaultTsz);

  // copy of the default
  struct TextSize tsz = *defaultTsz;

  // all the options (name, has_args, flags, val)
  // last element has to be all 0s for some reason
  struct option longOpts[] = {
      {"paragraphs", optional_argument, NULL, 1},
      {"minSntn", optional_argument, NULL, 2},
      {"maxSntn", optional_argument, NULL, 3},
      {"minWords", optional_argument, NULL, 4},
      {"maxWords", optional_argument, NULL, 5},
      {0, 0, 0, 0},
  };

  int opt;
  int optIndex = 0;
  // idk what to abbreviate the above opts to so just require their full names
  while ((opt = getopt_long_only(argc, argv, "", longOpts, &optIndex)) != -1) {
    switch (opt) {
    case 1:
      tsz.paragraphs = atoi(optarg);
      break;
    case 2:
      tsz.minSntn = atoi(optarg);
      break;
    case 3:
      tsz.maxSntn = atoi(optarg);
      break;
    case 4:
      tsz.minWordSntn = atoi(optarg);
      break;
    case 5:
      tsz.maxWordSntn = atoi(optarg);
      break;
    default:
      usage();
    }
  }

  return generateText(&tsz);
}

// main loop
void startGame(char *text, int textOffset) {
  // display text under the stopwatch and stuff
  moveCursor(textOffset, 0);
  displayText(text);

  int size = strlen(text);
  // user input
  char *input = calloc(size + 1, sizeof(char));

  // gotta use ints instead of size_t for the TypingStats so its atomic and
  // lock-free, meaning the stopwatch thread can safely access userStats
  // these strings probably wont be long enough to warrant size_t anyway
  struct TypingStats userStats = {.typed = 0, .correct = 0, .len = size};

  // each individual char can be in one of three states:
  // UNTYPED, CORRECT, INCORRECT
  // used for char-by-char text coloring
  CharState *charStates = calloc(size, sizeof(CharState));

  int c;
  int i = 0;
  bool isBackspace, isPrintable, isMatching;
  while (userStats.correct < size) {
    c = getchar();

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

    // make sure stdout is safe to write to for this (main) thread
    flockfile(stdout);

    // printing the text to the screen
    moveCursor(textOffset, 1);
    updateStats(&userStats);
    displayInput(i, size, input, text, charStates);
    fflush(stdout);

    // now allow other threads (stopwatch) to write to it
    funlockfile(stdout);
  }

  printf("\nDone!");
  sleep(1);

  // cleanup
  free(input);
  free(charStates);
}
