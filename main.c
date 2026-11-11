#include "escCodes.h"
#include "setup.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *TEXT = "the quick brown fox jumps over the lazy dog";
const size_t TEXT_SIZE = sizeof(&TEXT) / sizeof(char);

void startGame(const char *text, size_t size);
void resetLine(const char *text, const char *input);

int main() {
  if (init() == -1)
    return 1;

  startGame(TEXT, TEXT_SIZE);

  if (cleanup() == -1)
    return 1;
}

void startGame(const char *text, size_t size) {
  displayText(text, true, true);

  char *input = calloc(size + 1, sizeof(char));
  unsigned c;
  size_t i = 0;
  while (strcmp(text, input) != 0) {
    c = getchar();

    // backspace
    if ((c == 127 || c == 8) && i > 0) {
      i--;
      input[i] = '\0';
    }
    // can be printed
    else if (isprint(c)) {
      input[i++] = c;
      input[i] = '\0';
    }
    resetLine(text, input);
  }
}

void resetLine(const char *text, const char *input) {
  clearLine();
  moveToCol(0);
  displayText(text, true, true);
  displayText(input, false, false);
}
