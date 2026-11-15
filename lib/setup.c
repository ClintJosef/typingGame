#include "setup.h"
#include "typingUtils.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static struct termios OLD;

static void handleSignal(int sig);

int saveTermState(void) { return tcgetattr(0, &OLD); }
int restoreTermState(void) { return tcsetattr(0, TCSAFLUSH, &OLD); }

int disableCanonicalInput(void) {
  // read termios and tcsetattr man pages
  struct termios tc;
  if (tcgetattr(0, &tc) == -1)
    return -1;

  tc.c_cc[VMIN] = 1;
  tc.c_cc[VTIME] = 0;
  tc.c_lflag &= ~(ICANON | ECHO);

  return tcsetattr(0, TCSAFLUSH, &tc);
}

void init(void) {
  if (saveTermState() == -1)
    exit(EXIT_FAILURE);
  if (disableCanonicalInput() == -1)
    exit(EXIT_FAILURE);

  moveCursor(1, 1);
  hideCursor();
  eraseScreen();

  // in case program doesnt exit normally
  signal(SIGTERM, handleSignal);
  signal(SIGINT, handleSignal);
}

// can get called asynchronously, so available functions are limited
// see https://www.man7.org/linux/man-pages/man7/signal-safety.7.html
void cleanup(void) {
  int n = restoreTermState();

  // clear terminal and un-hide cursor safely in a signal handle
  tcflush(STDOUT_FILENO, TCOFLUSH);
  write(STDOUT_FILENO, SHOWN, strlen(SHOWN));

  // also async-signal-safe (but not just "exit"?)
  _exit(n != -1 ? EXIT_SUCCESS : EXIT_FAILURE);
}

static void handleSignal(int sig) { cleanup(); }
