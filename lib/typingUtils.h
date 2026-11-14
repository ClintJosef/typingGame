#ifndef TYPINGUTILS_H
#define TYPINGUTILS_H

#include <stdbool.h>
#include <stdlib.h>

typedef enum { UNTYPED, INCORRECT, CORRECT } CharState;

struct Colors {
  char *untyped;
  char *incorrect;
  char *correct;
};

struct TypingStats {
  size_t correct;
  size_t typed;
};

const struct Colors COLORS;

const char *RESET;
const char *BOLD;
const char *FAINT;
const char *UNDERLINE;
const char *HIDDEN;
const char *SHOWN;
const char *CUR_HOME;
const char *SAVE_CUR;
const char *RESTORE_CUR;
const char *ERASE_LINE;
const char *SAVE_SCREEN;
const char *RESTORE_SCREEN;
const char *ERASE_SCREEN;

void ansiPrint(int modes, ...);
void moveCursor(int row, int col);
void moveToCol(int col);
void hideCursor(void);
void showCursor(void);
void eraseScreen(void);
void resetModes(void);
char *getCharColor(CharState charState);
void displayText(char *text, bool faint, bool restore);
void displayInput(size_t size, char input[size], char text[size],
                  CharState charStates[size]);
void displayTypingStats(struct TypingStats *tpStats, double elapsedTime);

#endif
