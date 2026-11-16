#ifndef TYPINGUTILS_H
#define TYPINGUTILS_H

#include <stdbool.h>
#include <stdlib.h>

// UNTYPED = 0 for calloc (just to be safe)
typedef enum { UNTYPED = 0, INCORRECT, CORRECT } CharState;

struct Colors {
  char *untyped;
  char *incorrect;
  char *correct;
};

// 3 size_t's are apparently too big to be 'safely' atomic
// which is important because eventually the stopwatch and main thread
// will be sharing one of these
struct TypingStats {
  int correct;
  int typed;
  int len;
};

extern const struct Colors COLORS;

extern int cheatThreshold;

void ansiPrint(int modes, ...);
void moveCursor(int row, int col);
void moveToCol(int col);
void hideCursor(void);
void showCursor(void);
void saveScreen(void);
void restoreScreen(void);
void eraseScreen(void);
void resetModes(void);
void setCheatThreshold(int threshold);
void handleCheat(void);
char *getCharColor(CharState charState);
void displayText(char *text);
void displayInput(int pos, int size, char input[size], char text[size],
                  CharState charStates[size]);
void displayTypingStats(struct TypingStats *tpStats, double elapsedTime);

#endif
