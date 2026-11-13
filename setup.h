#pragma once // oh my god this is so nice it just prevents circular imports

#include "typingUtils.h"
#include <signal.h>
#include <stdio.h>
#include <termios.h>

static struct termios OLD;

int saveTermState();
int restoreTermState();
int enableCanonicalInputProcessing();
void init();
void cleanup();

int saveTermState() { return tcgetattr(0, &OLD); }
int restoreTermState() { return tcsetattr(0, TCSAFLUSH, &OLD); }

int enableCanonicalInputProcessing() {
  // read termios and tcsetattr man pages
  struct termios tc;
  if (tcgetattr(0, &tc) == -1)
    return 1;

  tc.c_cc[VMIN] = 1;
  tc.c_cc[VTIME] = 0;
  tc.c_lflag &= ~ICANON;
  tc.c_lflag &= ~ECHO;

  return tcsetattr(0, TCSAFLUSH, &tc);
}

void init() {
  if (saveTermState() == -1)
    exit(1);
  if (enableCanonicalInputProcessing() == -1)
    exit(1);

  moveCursor(1, 1);
  hideCursor();
  eraseScreen();

  // in case program doesnt exit normally
  signal(SIGTERM, cleanup);
  signal(SIGINT, cleanup);
}

void cleanup() {
  int n = restoreTermState();
  resetModes();
  showCursor();

  exit(n != -1);
}
