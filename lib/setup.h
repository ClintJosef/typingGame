#ifndef SETUP_H
#define SETUP_H

int saveTermState(void);
int restoreTermState(void);
int disableCanonicalInput(void);
void init(void);
void cleanup(void);

#endif
