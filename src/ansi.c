#include "ansi.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const char RESET[] = "\033[0m";
const char BOLD[] = "\033[1m";
const char FAINT[] = "\033[2m";
const char ITALIC[] = "\033[3m";
const char UNDERLINE[] = "\033[4m";
const char HIDDEN[] = "\033[?25l";
const char SHOWN[] = "\033[?25h";
const char CUR_HOME[] = "\033[H";
const char SAVE_CUR[] = "\033[s";
const char RESTORE_CUR[] = "\033[u";
const char ERASE_LINE[] = "\033[2K";
const char SAVE_SCREEN[] = "\033[?1049h";
const char RESTORE_SCREEN[] = "\033[?1049l";
const char ERASE_SCREEN[] = "\033[2J";
const char ERASE_CUR_TO_END[] = "\033[0J";

// woah, variadic function
// first arg has to be how many args were passed (minus one)
// note that this doesnt work for plain char's
// this is pretty much just a really bad printf that i thought was cool
void ansiPrint(int modes, ...) {
  va_list args;
  va_start(args, modes);

  for (int i = 0; i < modes; i++) {
    char *mode = va_arg(args, char *);
    printf("%s", mode);
  }

  va_end(args);
}

void moveCursor(int cursorPos, int col) {
  printf("\033[%d;%df", cursorPos, col);
}
void moveToCol(int col) { printf("\033[%dG", col); }
void resetModes(void) { ansiPrint(1, RESET); }
void eraseLine(void) { ansiPrint(1, ERASE_LINE); }

// these can just write to stdout directly, regardless of cursor position
void hideCursor(void) { write(STDOUT_FILENO, HIDDEN, strlen(HIDDEN)); }
void showCursor(void) { write(STDOUT_FILENO, SHOWN, strlen(SHOWN)); }
void eraseScreen(void) {
  write(STDOUT_FILENO, ERASE_SCREEN, strlen(ERASE_SCREEN));
}
void eraseCurToEnd(void) {
  write(STDOUT_FILENO, ERASE_CUR_TO_END, strlen(ERASE_CUR_TO_END));
}
void saveScreen(void) {
  write(STDOUT_FILENO, SAVE_SCREEN, strlen(SAVE_SCREEN));
}
void restoreScreen(void) {
  write(STDOUT_FILENO, RESTORE_SCREEN, strlen(RESTORE_SCREEN));
}

// detect if escape sequence was inputted (arrow keys etc.)
// check if ESC (27) and '[' (91) were pressed first
// this only works if called sequentially
bool isEscaped(char c) {
  static bool escaped = false;
  static bool bracket = false;

  // ESC and '[' pressed -> next call returns true
  if (escaped && c == 91)
    bracket = true;
  else if (bracket) {
    bracket = false;
    return true;
  }

  escaped = c == 27;
  return escaped;
}
