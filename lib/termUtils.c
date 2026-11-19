#include "termUtils.h"
#include "ansi.h"
#include "typingUtils.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termcap.h>
#include <termios.h>
#include <unistd.h>

static struct termios old;
static struct WinSize winSize;

static void updateWinSize(void);
static void handleResize(int sig);
static void handleSignal(int sig);

int saveTermState(void) { return tcgetattr(getTtyFD(), &old); }
int restoreTermState(void) { return tcsetattr(getTtyFD(), TCSAFLUSH, &old); }

int disableCanonicalInput(void) {
  // read termios and tcsetattr man pages
  struct termios tc;
  if (tcgetattr(getTtyFD(), &tc) == -1)
    return -1;

  tc.c_cc[VMIN] = 1;
  tc.c_cc[VTIME] = 0;
  tc.c_lflag &= ~(ICANON | ECHO);

  return tcsetattr(getTtyFD(), TCSAFLUSH, &tc);
}

void init(void) {
  // save state before setting input mode
  saveScreen();
  if (saveTermState() == -1)
    exit(EXIT_FAILURE);

  if (disableCanonicalInput() == -1)
    exit(EXIT_FAILURE);

  moveCursor(1, 1);
  hideCursor();
  eraseScreen();

  // update winSize when necessary
  updateWinSize();
  signal(SIGWINCH, handleResize);

  // in case program doesnt exit normally
  signal(SIGTERM, handleSignal);
  signal(SIGSEGV, handleSignal);
  signal(SIGBUS, handleSignal);
  signal(SIGINT, handleSignal);
  signal(SIGTSTP, handleSignal);
}

// can get called asynchronously, so available functions are limited
// see https://www.man7.org/linux/man-pages/man7/signal-safety.7.html
void cleanup(void) {
  // restore terminal and un-hide cursor safely in a signal handle
  restoreScreen();
  showCursor();

  // reenable canonical input mode
  if (restoreTermState() == -1)
    _exit(EXIT_FAILURE);

  // close /dev/tty
  close(getTtyFD());
}

// since we read from stdin at the very start it wont be usable at this point
// so, we read from the current program's "stdin"! (/dev/tty)
int getTtyFD(void) {
  static int fd = -1;

  if (fd == -1)
    fd = open("/dev/tty", O_RDWR);

  return fd;
}

// read input from here instead
FILE *getInputStream(void) {
  static FILE *tty = NULL;

  if (tty == NULL)
    tty = fdopen(getTtyFD(), "r");

  return tty;
}

struct WinSize getWinSize(void) { return winSize; }

static void updateWinSize(void) {
  static char termbuf[1024];
  static char *type = NULL;
  if (type == NULL)
    type = getenv("TERM");

  if (tgetent(termbuf, type) < 0)
    _exit(EXIT_FAILURE);

  winSize.rows = tgetnum("li");
  winSize.cols = tgetnum("co");
}

static void handleResize(int sig) { updateWinSize(); }

static void handleSignal(int sig) {
  cleanup();
  _exit(sig);
}
