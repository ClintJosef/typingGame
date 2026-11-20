#ifndef TERMUTILS_H
#define TERMUTILS_H

#include <stdio.h>

struct WinSize {
  int rows;
  int cols;
};

int saveTermState(void);
int restoreTermState(void);
int disableCanonicalInput(void);
void init(void);
void cleanup(void);
int getTtyFD(void);
int pollChar(int timeout, char *c);
FILE *getInputStream(void);
struct WinSize getWinSize(void);

#endif
