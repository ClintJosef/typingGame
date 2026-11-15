#include "typingUtils.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *RESET = "\033[0m";
const char *BOLD = "\033[1m";
const char *FAINT = "\033[2m";
const char *UNDERLINE = "\033[4m";
const char *HIDDEN = "\033[?25l";
const char *SHOWN = "\033[?25h";
const char *CUR_HOME = "\033[H";
const char *SAVE_CUR = "\033[s";
const char *RESTORE_CUR = "\033[u";
const char *ERASE_LINE = "\033[2K";
const char *SAVE_SCREEN = "\033[?47h";
const char *RESTORE_SCREEN = "\033[?47l";
const char *ERASE_SCREEN = "\033[2J";

const struct Colors COLORS = {
    .untyped = "\033[2m", .incorrect = "\033[31m", .correct = "\033[38;5;76m"};

// woah, variadic function
// first arg has to be how many args were passed (minus one)
// note that this doesnt work for plain char's
// this is pretty much just a really bad printf that i thought was cool
void ansiPrint(int modes, ...) {
  va_list args;
  va_start(args, modes);

  for (int i = 0; i < modes; i++)
    printf("%s", va_arg(args, char *));

  va_end(args);
}

void moveCursor(int row, int col) { printf("\033[%d;%df", row, col); }
void moveToCol(int col) { printf("\033[%dG", col); }
void hideCursor(void) { ansiPrint(1, HIDDEN); }
void showCursor(void) { ansiPrint(1, SHOWN); }
void eraseScreen(void) { ansiPrint(1, ERASE_SCREEN); }
void resetModes(void) { ansiPrint(1, RESET); }

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
void displayInput(size_t pos, size_t size, char input[size], char text[size],
                  CharState charStates[size]) {
  for (size_t i = 0; i < size; i++) {
    bool atCursor = i == pos;
    bool untyped = charStates[i] == UNTYPED;
    bool covered = isblank(input[i]) && !isblank(text[i]);

    // custom cursor
    if (atCursor)
      printf("%s%s%c", BOLD, UNDERLINE, text[pos]);

    // colorize user input (green/red)
    else if (!untyped && !covered)
      printf("%s%c", getCharColor(charStates[i]), input[i]);

    // show faint text if user inputted a space/tab over it
    // or if its not typed yet
    else
      printf("%s%c", FAINT, text[i]);

    resetModes();
  }
}

void displayTypingStats(struct TypingStats *tpStats, double elapsedTime) {
  // pretty useful
  static double prevTime = 0;
  static double prevTyped = 0;

  size_t correct = tpStats->correct;
  size_t typed = tpStats->typed;
  int len = tpStats->len;

  // for delta wpm
  double dt = elapsedTime - prevTime;
  double dTyped = typed - prevTyped;

  // arbitrary 5-letter words per minute
  double wpm = dTyped / 5 / dt * 60;
  double avgWpm = (double)typed / 5 / elapsedTime * 60;

  double accuracy = (double)correct / typed * 100;
  double progress = (double)correct / len * 100;

  // values are saved for the next function call
  prevTime = elapsedTime;
  prevTyped = typed;

  printf("\rWpm: %.1lf | Avg Wpm: %.1lf | Accuracy: %.1lf | Progress: %.1lf",
         wpm, avgWpm, accuracy, progress);
}
