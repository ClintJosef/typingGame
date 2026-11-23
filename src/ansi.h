#ifndef ANSI_H
#define ANSI_H

#include <stdbool.h>

extern const char RESET[];
extern const char BOLD[];
extern const char FAINT[];
extern const char UNDERLINE[];
extern const char ITALIC[];
extern const char HIDDEN[];
extern const char SHOWN[];
extern const char CUR_HOME[];
extern const char SAVE_CUR[];
extern const char RESTORE_CUR[];
extern const char ERASE_LINE[];
extern const char SAVE_SCREEN[];
extern const char RESTORE_SCREEN[];
extern const char ERASE_SCREEN[];
extern const char ERASE_CUR_TO_END[];

void ansiPrint(int modes, ...);
void moveCursor(int row, int col);
void moveToCol(int col);
void hideCursor(void);
void showCursor(void);
void saveScreen(void);
void restoreScreen(void);
void eraseScreen(void);
void eraseCurToEnd(void);
void eraseLine(void);
void resetModes(void);
bool isEscaped(char c);

#endif
