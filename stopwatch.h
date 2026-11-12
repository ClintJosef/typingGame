#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static double start;
static const long long SLEEP_INTERVAL_MS = 250;

void startStopwatch();
void *stopwatchThread(void *arg);
void startStopwatchThread();
double timeElapsed();

static double tsToDouble(struct timespec *ts);

void startStopwatch() {
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  start = tsToDouble(&ts);
}

void *stopwatchThread(void *arg) {
  struct timespec ts = {0, SLEEP_INTERVAL_MS * 1e6};
  startStopwatch();
  while (true) {
    // hide cursor, move to pos (1, 1) in the terminal, and print the time
    printf("\e[?25l\e[1;1fTime taken: %.2lfs\n", timeElapsed());
    nanosleep(&ts, NULL);
  }
}

void startStopwatchThread() {
  pthread_t thread;
  pthread_create(&thread, NULL, &stopwatchThread, NULL);
}

double timeElapsed() {
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  double lap = tsToDouble(&ts);

  return lap - start;
}

static double tsToDouble(struct timespec *ts) {
  return (double)ts->tv_sec + (double)ts->tv_nsec / 1e9;
}
