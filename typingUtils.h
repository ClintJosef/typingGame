#pragma once

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char RESET[] = "\e[0m";
const char BOLD[] = "\e[1m";
const char FAINT[] = "\e[2m";
const char UNDERLINE[] = "\e[4m";
const char BLINKING[] = "\e[5m";
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

void ansiPrint(int modes, ...);
void moveCursor(int row, int col);
void moveToCol(int col);
void hideCursor();
void showCursor();
void eraseScreen();
void resetModes();
char *getCharColor(CharState charState);
void displayText(char *text, bool faint, bool restore);
void displayInput(size_t size, char input[size], char text[size],
                  CharState charStates[size]);
static char *charToStr(char c);

// woah, variadic function
// first arg has to be how many args were passed (minus one)
// note that this doesnt work for plain char's
// this is pretty much just a really bad printf that i thought was cool
void ansiPrint(int modes, ...) {
  va_list args;
  va_start(args, modes);

  for (int i = 0; i < modes; i++) {
    printf("%s", va_arg(args, char *));
  }

  va_end(args);
}

void moveCursor(int row, int col) { printf("\e[%d;%df", row, col); }
void moveToCol(int col) { printf("\e[%dG", col); }
void hideCursor() { ansiPrint(1, HIDDEN); }
void showCursor() { ansiPrint(1, SHOWN); }
void eraseScreen() { ansiPrint(1, ERASE_SCREEN); }
void resetModes() { ansiPrint(1, RESET); }

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

void displayText(char *text, bool faint, bool restore) {
  const char *mod = faint ? FAINT : "";
  const char *res = restore ? RESTORE_CUR : "";

  ansiPrint(5, SAVE_CUR, mod, text, RESET, res);
}

// colorizes each character based their status
void displayInput(size_t size, char input[size], char text[size],
                  CharState charStates[size]) {
  size_t i = 0;
  for (; i < size; i++) {
    // if a space is over a char, show that char instead (but faint)
    if (!isblank(text[i]) && isblank(input[i]))
      printf("%s%c", FAINT, text[i]);

    else
      printf("%s%c", getCharColor(charStates[i]), input[i]);

    resetModes();
  }

  // custom cursor basically
  printf("%s%s%c%s", BOLD, UNDERLINE, text[i], RESET);
}
