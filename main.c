#include "escCodes.h"
#include "setup.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *TEXT = "the quick brown fox jumps over the lazy dog";
const size_t TEXT_SIZE = sizeof(TEXT) - 1;

char *parseArgs(int argc, char *argv[]);
void startGame(const char *text);
void resetLine(const char *text, const char *input);

int main(int argc, char *argv[]) {
  if (init() == -1)
    return 1;

  // fall back to default text if no args are given
  const char *text = argc > 1 ? parseArgs(argc, argv) : TEXT;
  startGame(text);

  if (cleanup() == -1)
    return 1;
}

// parse every arg into one string, separated by spaces
char *parseArgs(int argc, char *argv[]) {
  // initial size
  size_t size = 1; // +1 for null byte
  for (int i = 1; i < argc; i++)
    size += strlen(argv[i]) + 1; // +1 for space
  char *text = malloc(sizeof(char) * size);

  for (int i = 1; i < argc; i++) {
    if (i > 1)
      strcat(text, " ");
    strcat(text, argv[i]);
  }

  return text;
}

void startGame(const char *text) {
  displayText(text, true, true);

  size_t size = strlen(text);
  char *input = calloc(size + 1, sizeof(char));

  int c;
  size_t i = 0;
  while (strcmp(text, input) != 0) {
    c = getchar();

    // backspace
    if ((c == 127 || c == 8) && i > 0) {
      i--;
      input[i] = '\0';
    }
    // can be printed
    else if (isprint(c) && i < size) {
      input[i++] = c;
      input[i] = '\0';
    }
    resetLine(text, input);
  }

  free(input);
}

void resetLine(const char *text, const char *input) {
  clearLine();
  moveToCol(0);
  displayText(text, true, true);
  displayText(input, false, false);
}
