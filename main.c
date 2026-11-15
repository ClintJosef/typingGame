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

char *parseArgs(int argc, char *argv[], struct TextSize *defaultTsz);
int parseIntOrExit(char *str);
void usage(bool invalid);
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

// parse every arg into one string, separated by spaces
char *parseArgs(int argc, char *argv[], struct TextSize *defaultTsz) {
  // fall back to default text if no args are given
  if (argc < 2)
    // have to do this to avoid "disqualifying" const
    return generateText((struct TextSize *)defaultTsz);

  // copy of the default
  struct TextSize tsz = *defaultTsz;

  // all the options
  // the number is just its "id" returned by getopt_long_only etc.
  // last option has to be all 0s for some reason
  struct option longOpts[] = {
      {"pg", optional_argument, NULL, 1},
      {"smin", optional_argument, NULL, 2},
      {"smax", optional_argument, NULL, 3},
      {"wmin", optional_argument, NULL, 4},
      {"wmax", optional_argument, NULL, 5},
      {"help", no_argument, NULL, 'h'},
      {"h", no_argument, NULL, 'h'},
      {0, 0, 0, 0},
  };

  int opt;
  // loop through every option given
  while ((opt = getopt_long_only(argc, argv, "", longOpts, NULL)) != -1) {
    switch (opt) {
    case 1:
      tsz.paragraphs = parseIntOrExit(optarg);
      break;
    case 2:
      tsz.minSntn = parseIntOrExit(optarg);
      break;
    case 3:
      tsz.maxSntn = parseIntOrExit(optarg);
      break;
    case 4:
      tsz.minWordSntn = parseIntOrExit(optarg);
      break;
    case 5:
      tsz.maxWordSntn = parseIntOrExit(optarg);
      break;
    case 'h':
      usage(false);
      break;
    case '?':
    default:
      usage(true);
    }
  }

  return generateText(&tsz);
}

// exit program if user passed in invalid numbers as arguments
int parseIntOrExit(char *str) {
  // points to the next invalid character in the str
  // if none, points to the end of the string ('\0')
  // if it couldnt convert anything, this just ends up as the original string
  char *endPtr;

  // actually returns a long but theyre both 32 bit on my system so ig its fine
  int num = strtol(str, &endPtr, 10);

  if (endPtr == str || *endPtr != '\0')
    usage(true);

  return num;
}

// help message
void usage(bool invalid) {
  static const char MSG[] =
      "Usage: typingGame [-pg -smin -smax -wmin -wmax -h -help]\n\n"
      "Options:\n"
      "    -pg \t:no. of paragraphs\t\t\t(default: 2)\n"
      "    -smin \t:minimum no. of sentences per paragraph\t(default: 3)\n"
      "    -smax \t:maximum no. of sentences per paragraph\t(default: 5)\n"
      "    -wmin \t:minimum no. of words per sentence\t(default: 4)\n"
      "    -wmax \t:maximum no. of words per sentence\t(default: 6)\n"
      "    -h/-help \t:prints this help message\n\n"
      "Each arg must be passed either as '-arg=value' or '--arg=value'";

  if (invalid)
    fprintf(stderr, "Invalid args.\n\n");
  fprintf(stderr, "%s", MSG);

  exit(1);
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
