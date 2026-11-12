#include "typingUtils.h"
#include <stdio.h>
#include <termios.h>

static struct termios OLD;

int saveTermState();
int restoreTermState();
int enableCanonicalInputProcessing();
int init();
int cleanup();

int saveTermState() { return tcgetattr(0, &OLD); }
int restoreTermState() { return tcsetattr(0, TCSAFLUSH, &OLD); }

int enableCanonicalInputProcessing() {
  // read termios and tcsetattr man pages
  struct termios tc;
  if (tcgetattr(0, &tc) == -1)
    return -1;

  tc.c_cc[VMIN] = 1;
  tc.c_cc[VTIME] = 0;
  tc.c_lflag &= ~ICANON;
  tc.c_lflag &= ~ECHO;

  return tcsetattr(0, TCSAFLUSH, &tc);
}

int init() {
  if (saveTermState() == -1)
    return -1;
  if (enableCanonicalInputProcessing() == -1)
    return -1;
  moveCursor(0, 0);
  eraseScreen();
  return 0;
}

int cleanup() {
  int n = restoreTermState();
  resetModes();
  showCursor();
  return n;
}
