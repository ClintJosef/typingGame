#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char RESET[] = "\e[0m";
const char BOLD[] = "\e[1m";
const char FAINT[] = "\e[2m";
const char HIDDEN[] = "\e[?25l";
const char SHOWN[] = "\e[?25h";
const char CUR_HOME[] = "\e[H";
const char SAVE_CUR[] = "\e[s";
const char RESTORE_CUR[] = "\e[u";
const char ERASE_LINE[] = "\e[2K";
const char SAVE_SCREEN[] = "\e[?47h";
const char RESTORE_SCREEN[] = "\e[?47l";
const char ERASE_SCREEN[] = "\e[2J";

typedef enum { UNTYPED, INCORRECT, CORRECT } CharState;

struct Colors {
  char *untyped;
  char *incorrect;
  char *correct;
};

const struct Colors COLORS = {"\e[2m", "\e[31m", "\e[38;5;76m"};

void moveCursor(int row, int col);
void moveToCol(int col);
void hideCursor();
void showCursor();
void eraseScreen();
void resetModes();
char *getCharColor(CharState charState);
void displayText(const char *text, bool faint, bool restore);
void displayHlText(size_t size, char input[size], const char text[size],
                   CharState charStates[size]);

void moveCursor(int row, int col) { printf("\e[%d;%df", row, col); }
void moveToCol(int col) { printf("\e[%dG", col); }
void hideCursor() { printf("%s", HIDDEN); }
void showCursor() { printf("%s", SHOWN); }
void eraseScreen() { printf("%s", ERASE_SCREEN); }
void resetModes() { printf("%s", RESET); }

char *getCharColor(CharState charState) {
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

// pretty self explanatory
void displayText(const char *text, bool faint, bool restore) {
  const char *mod = faint ? FAINT : "";
  const char *res = restore ? RESTORE_CUR : "";

  printf("%s%s%s%s%s", SAVE_CUR, mod, text, RESET, res);
}

// colorizes each character based their status
void displayHlText(size_t size, char input[size], const char text[size],
                   CharState charStates[size]) {
  for (size_t i = 0; i < size; i++) {
    // don't let space characters hide the text underneath
    bool bothSpaces = isblank(input[i]) == isblank(text[i]);
    if (!bothSpaces && isblank(input[i]))
      printf("%s%c%s", FAINT, text[i], RESET);

    else {
      char *color = getCharColor(charStates[i]);
      printf("%s%c%s", color, input[i], RESET);
    }
  }
}
