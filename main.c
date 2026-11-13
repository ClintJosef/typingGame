#include "setup.h"
#include "stopwatch.h"
#include "textGen.h"
#include "typingUtils.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int TEXT_OFFSET = 4;
const struct TextSize DEFAULT_TSZ = {
    2, // paragraphs
    3, // min sentences per paragraph
    5, // max ''
    4, // min words per sentence
    6  // max ''
};

char *parseArgs(int argc, char *argv[], const struct TextSize *defaultTsz);
void startGame(char *text, int textOffset);

int main(int argc, char *argv[]) {
  srand(time(NULL));

  if (init() == -1)
    return 1;

  char *text = parseArgs(argc, argv, &DEFAULT_TSZ);
  startStopwatch();
  startGame(text, TEXT_OFFSET);

  killStopWatch();
  if (cleanup() == -1)
    return 1;
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
  // move to row 2 since the stopwatch is in row 1
  moveCursor(textOffset, 0);
  displayText(text, true, true);

  size_t size = strlen(text);
  char *input = calloc(size + 1, sizeof(char));

  // each individual char can be in one of three states:
  // UNTYPED, CORRECT, INCORRECT
  // used for char-by-char text coloring
  CharState *charStates = malloc(sizeof(CharState) * size);
  for (size_t i = 0; i < size; i++)
    charStates[i] = UNTYPED;
  // just a tally of the correctly typed and total typed chars
  struct TypingStats userStats = {0, 0};

  int c;
  size_t i = 0;
  bool inRange, nonEmpty, isBackspace, isPrintable, isMatching;
  while (userStats.correct < size) {
    c = getchar();

    // all these flags
    inRange = i < size;
    nonEmpty = i > 0;
    isBackspace = c == 127 || c == 8;
    isPrintable = isprint((unsigned char)c);
    isMatching = c == text[i];

    // backspace
    if (isBackspace && nonEmpty) {
      // handle deleting newlines first
      while (nonEmpty && text[i - 1] == '\n') {
        i--;
        userStats.correct--;
        userStats.typed--;
      }

      // normal deletion
      i--;
      if (charStates[i] == CORRECT) {
        userStats.correct--;
        userStats.typed--;
      }
    }

    // can be printed
    else if (isPrintable && inRange) {
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
    while (text[i] == '\n') {
      userStats.correct++;
      input[i] = '\n';
      i++;
    }

    // always gotta have the null byte
    input[i] = '\0';

    // printing the text to the screen
    moveCursor(textOffset, 0);
    updateStats(&userStats);
    displayText(text, true, true);
    displayInput(i, input, text, charStates);
  }

  // cleanup
  free(input);
  free(charStates);
}
