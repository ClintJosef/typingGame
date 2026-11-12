#include "setup.h"
#include "stopwatch.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *TEXT = "the quick brown fox\n\njumps over the lazy dog";

char *parseArgs(int argc, char *argv[], const char *defaultText);
void startGame(const char *text);
void display(size_t size, char input[size], const char text[size],
             CharState charStates[size]);

int main(int argc, char *argv[]) {
  if (init() == -1)
    return 1;

  char *text = parseArgs(argc, argv, TEXT);
  pthread_t stopwatch = startStopwatch();
  startGame(text);

  killThread(stopwatch);
  if (cleanup() == -1)
    return 1;
}

// parse every arg into one string, separated by spaces
char *parseArgs(int argc, char *argv[], const char *defaultText) {
  // fall back to default text if no args are given
  if (argc < 2)
    // have to do this to avoid "disqualifying" const
    return (char *)defaultText;

  // initial size
  size_t size = 1; // +1 for null byte
  for (int i = 1; i < argc; i++)
    size += strlen(argv[i]) + 1; // +1 for space
  char *text = calloc(size + 1, sizeof(char));

  for (int i = 1; i < argc; i++) {
    if (i > 1)
      strcat(text, " ");
    strcat(text, argv[i]);
  }

  return text;
}

// main loop
void startGame(const char *text) {
  // move to row 2 since the stopwatch is in row 1
  moveCursor(2, 0);
  displayText(text, true, true);

  size_t size = strlen(text);
  char *input = calloc(size + 1, sizeof(char));

  CharState *charStates = malloc(sizeof(CharState) * size);
  for (size_t i = 0; i < size; i++)
    charStates[i] = UNTYPED;
  size_t correct = 0;

  int c;
  size_t i = 0;
  bool inRange, nonEmpty, isBackspace, isPrintable, isMatching;
  while (correct < size) {
    c = getchar();

    // all these flags
    inRange = i < size;
    nonEmpty = i > 0;
    isBackspace = c == 127 || c == 8;
    isPrintable = isprint((unsigned char)c);
    isMatching = c == text[i];

    // backspace
    if (isBackspace && nonEmpty) {
      // handle removing newlines first
      while (nonEmpty && text[i - 1] == '\n') {
        i--;
        correct--;
      }

      // normal deletion
      i--;
      if (charStates[i] == CORRECT)
        correct--;
    }

    // can be printed
    else if (isPrintable && inRange) {
      // match chars
      if (isMatching) {
        charStates[i] = CORRECT;
        correct++;
      } else
        charStates[i] = INCORRECT;
      input[i] = c;
      i++;
    }

    // handle adding newlines
    while (text[i] == '\n') {
      correct++;
      input[i] = '\n';
      i++;
    }

    // always gotta have the null byte
    input[i] = '\0';

    display(i, input, text, charStates);
  }

  // cleanup
  free(input);
  free(charStates);
}

void display(size_t size, char input[size], const char text[size],
             CharState charStates[size]) {
  moveCursor(2, 0);
  displayText(text, true, true);
  displayHlText(size, input, text, charStates);
}
