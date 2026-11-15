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
  int correct;
  int typed;
  int len;
};

extern const struct Colors COLORS;

extern const char *RESET;
extern const char *BOLD;
extern const char *FAINT;
extern const char *UNDERLINE;
extern const char *HIDDEN;
extern const char *SHOWN;
extern const char *CUR_HOME;
extern const char *SAVE_CUR;
extern const char *RESTORE_CUR;
extern const char *ERASE_LINE;
extern const char *SAVE_SCREEN;
extern const char *RESTORE_SCREEN;
extern const char *ERASE_SCREEN;

extern int cheatThreshold;

void ansiPrint(int modes, ...);
void moveCursor(int row, int col);
void moveToCol(int col);
void hideCursor(void);
void showCursor(void);
void eraseScreen(void);
void resetModes(void);
void setCheatThreshold(int threshold);
void handleCheat(void);
char *getCharColor(CharState charState);
void displayText(char *text, bool faint, bool restore);
void displayInput(int pos, int size, char input[size], char text[size],
                  CharState charStates[size]);
void displayTypingStats(struct TypingStats *tpStats, double elapsedTime);

#endif
