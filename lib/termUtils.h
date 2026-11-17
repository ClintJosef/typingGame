#ifndef TERMUTILS_H
#define TERMUTILS_H

#include <stdio.h>

int saveTermState(void);
int restoreTermState(void);
int disableCanonicalInput(void);
void init(void);
void cleanup(void);
int getTtyFD(void);
FILE *getInputStream(void);

#endif
