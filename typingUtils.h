#include "escCodes.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef enum { UNTYPED, INCORRECT, CORRECT } CharState;

char *getColorAnsi(CharState charState) {
  char *color;
  switch (charState) {
  case INCORRECT:
    color = COLORS.incorrect;
    break;
  case CORRECT:
    color = COLORS.correct;
    break;
  case UNTYPED:
    color = COLORS.untyped;
    break;
  }

  return color;
}

void displayHlText(size_t size, char input[size], const char text[size],
                   CharState charStates[size]) {
  displayText(text, true, true);
  for (size_t i = 0; i < size; i++) {
    if (isblank(text[i]) != isblank(input[i])) {
      continue;
    }
    char *color = getColorAnsi(charStates[i]);
    printf("%s%c%s", color, input[i], RESET);
  }
}
