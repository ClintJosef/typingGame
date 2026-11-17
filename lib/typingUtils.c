#include "typingUtils.h"
#include "ansi.h"
#include "termUtils.h"
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const struct Colors COLORS = {
    .untyped = "\033[2m", .incorrect = "\033[31m", .correct = "\033[38;5;76m"};

// max wpm limit before it thinks user is cheating (can be set)
int cheatThreshold = INT32_MAX;

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

void setCheatThreshold(int threshold) { cheatThreshold = threshold; }
void handleCheat(void) {
  // clear screen
  eraseScreen();
  moveCursor(1, 1);

  // the line
  printf("You dirty cheater.");
  fflush(stdout);
  sleep(2);

  // clear again and signal to restore term
  eraseScreen();
  moveCursor(1, 1);
  killpg(0, SIGTERM);
}

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

// colorizes each character based their status
// does not write past the screen
void displayInput(int pos, int textOffset, const struct TypingInfo *data,
                  const struct WinSize *winSize) {
  int cursorPos = 0;
  bool atCursor, untyped, covered;

  for (int i = 0; i < data->textLen; i++) {
    // a row is winSize.cols long.
    // if the newline char is in the middle of the line, it still advances to
    // the next row, but only to the first column
    // so just add winSize.cols minus the cursor's current pos in the line
    if (data->text[i] == '\n')
      cursorPos += winSize->cols - (cursorPos % winSize->cols);
    else
      cursorPos++;

    // cutoff text before running out of terminal space
    if (cursorPos / winSize->cols >= winSize->rows - textOffset)
      break;

    // rest of the loop (actually displaying)

    atCursor = i == pos;
    untyped = data->charStates[i] == UNTYPED;
    covered = isblank(data->userInput[i]) && !isblank(data->text[i]);

    // custom cursor
    if (atCursor)
      printf("%s%s%c", BOLD, UNDERLINE, data->text[pos]);

    // colorize user userInput (green/red)
    else if (!untyped && !covered)
      printf("%s%c", getCharColor(data->charStates[i]), data->userInput[i]);

    // show faint text if user inputted a space/tab over it
    // or if its not typed yet
    else
      printf("%s%c", FAINT, data->text[i]);

    resetModes();
  }
}

void displayUserStats(const struct UserStats *userStats, double elapsedTime) {
  // pretty useful
  static double prevTime = 0;
  static double prevTyped = 0;

  // for delta wpm
  double dt = elapsedTime - prevTime;
  double dTyped = userStats->typed - prevTyped;

  // arbitrary 5-letter words per minute
  double wpm = (double)dTyped / 5 / dt * 60;
  double avgWpm = (double)userStats->typed / 5 / elapsedTime * 60;

  double accuracy = (double)userStats->correct / userStats->typed * 100;
  double progress = (double)userStats->correct / userStats->textLen * 100;

  // values are saved for the next function call
  prevTime = elapsedTime;
  prevTyped = userStats->typed;

  // exit if user is "typing" too fast
  if (wpm >= cheatThreshold) {
    handleCheat();
    return;
  }

  printf("%sWpm: %.0lf, Avg Wpm: %.0lf, Accuracy: %.0lf%%, Progress: %.0lf%%",
         ERASE_LINE, wpm, avgWpm, accuracy, progress);
}
