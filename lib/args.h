#ifndef ARGS_H
#define ARGS_H

#include "stdbool.h"
#include "textGen.h"

char *parseArgs(int argc, char *argv[], struct TextSize *defaultTsz);
char *readStdin(void);
int parseIntOrExit(char *str);
void usage(bool invalid);

#endif
