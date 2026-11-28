#ifndef TERMUTILS_H
#define TERMUTILS_H

#include <stdio.h>

typedef struct {
  int rows;
  int cols;
} WinSize;

int saveTermState(void);
int restoreTermState(void);
int disableCanonicalInput(void);
void init(void);
void cleanup(void);
int getTtyFD(void);
int pollChar(char *c, int maxBufSize);
FILE *getInputStream(void);
WinSize getWinSize(void);

#endif
