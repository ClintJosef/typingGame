#include "lib/setup.h"
#include "lib/stopwatch.h"
#include "lib/textGen.h"
#include "lib/typingUtils.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int TEXT_OFFSET = 4;
const struct TextSize DEFAULT_TSZ = {
    .paragraphs = 2,
    .minSntn = 3,
    .maxSntn = 5,
    .minWordSntn = 4,
    .maxWordSntn = 6,
};

char *parseArgs(int argc, char *argv[], const struct TextSize *defaultTsz);
void startGame(char *text, int textOffset);

int main(int argc, char *argv[]) {
  init();
  srand(time(NULL));

  char *text = parseArgs(argc, argv, &DEFAULT_TSZ);
  startStopwatch();
  startGame(text, TEXT_OFFSET);

  killStopWatch();
  cleanup();
}

// parse every arg into one string, separated by spaces
char *parseArgs(int argc, char *argv[], const struct TextSize *defaultTsz) {
  // fall back to default text if no args are given
  if (argc < 2)
    // have to do this to avoid "disqualifying" const
    return generateText((struct TextSize *)defaultTsz);

  // initial size
  size_t size = 1; // +1 for null byte
  for (int i = 1; i < argc; i++)
    size += strlen(argv[i]) + 1; // +1 for space
  char *text = calloc(size + 1, sizeof(char));

  // concatenate
  for (int i = 1; i < argc; i++) {
    if (i > 1)
      strcat(text, " ");
    strcat(text, argv[i]);
  }

  return text;
}

// main loop
void startGame(char *text, int textOffset) {
  // display text under the stopwatch and stuff
  moveCursor(textOffset, 0);
  displayText(text, true, true);

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
  CharState *charStates = malloc(sizeof(CharState) * size);
  for (int i = 0; i < size; i++)
    charStates[i] = UNTYPED;

  int c;
  int i = 0;
  bool isBackspace, isPrintable, isMatching;
  while (userStats.correct < size) {
    c = getchar();

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
    while (text[i] == '\n' && i < size) {
      userStats.correct++;
      input[i] = '\n';
      i++;
    }

    // always gotta have the null byte
    input[i] = '\0';

    // printing the text to the screen
    moveCursor(textOffset, 1);
    updateStats(&userStats);
    displayInput(i, size, input, text, charStates);
    fflush(stdout);
  }

  // cleanup
  free(input);
  free(charStates);
}
