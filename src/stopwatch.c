#include "stopwatch.h"
#include "ansi.h"
#include "typingUtils.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static pthread_t swThread;

// timespec just stores a number of seconds (time_t) and nanoseconds (unsigned
// long long?)
static struct timespec start;

// atomic basically means these are thread-safe (but not lock-free)
// at the cost of some extra overhead
// for structs its still recommended to access them via atomic_load()
static _Atomic(bool) running;
static _Atomic(bool) stopped;
static _Atomic(UserStats) stats;

static void *stopwatchThread(void *arg);

void startTs(void) {
  // basically just a higher resolution time()
  // also ignores changes to the system clock
  clock_gettime(CLOCK_MONOTONIC, &start);
}

// wall clock time since startTs() was called
void elapsedTs(struct timespec *ts) {
  clock_gettime(CLOCK_MONOTONIC, ts);

  ts->tv_sec -= start.tv_sec;
  ts->tv_nsec -= start.tv_nsec;
  if (ts->tv_nsec < 0) {
    ts->tv_sec--;
    ts->tv_nsec += 1e9;
  }
}

// call for stopwatch thread to kill itself
void killStopWatch(void) {
  // make sure this is only called once
  if (stopped)
    return;

  running = false;
  pthread_join(swThread, NULL);
  stopped = true;
}

// start the stopwatch in a separate thread
void startStopwatch(long long tickMs, int statsTicks) {
  running = true;
  stopped = false;

  SWThreadArgs *swArgs = malloc(sizeof(SWThreadArgs));
  swArgs->tickMs = tickMs;
  swArgs->statsTicks = statsTicks;

  pthread_create(&swThread, NULL, stopwatchThread, swArgs);
}

// update stats from other threads
void updateStats(const UserStats *userStats) {
  stats = *userStats;
}

// convert miliseconds to timespec
void msToTs(long long ms, struct timespec *ts) {
  time_t seconds = ms / 1000;
  unsigned long long nseconds = (ms % 1000) * 1e6;

  ts->tv_sec = seconds;
  ts->tv_nsec = nseconds;
}

// convert timespec to time in seconds
double tsToDouble(const struct timespec *ts) {
  return (double)ts->tv_sec + (double)ts->tv_nsec / 1e9;
}

// actual thread
static void *stopwatchThread(void *arg) {
  static char timeFmt[] = "\rTime taken: %.1lfs\n";

  SWThreadArgs *swArgs = (SWThreadArgs *)arg;

  struct timespec sleepTime;
  msToTs(swArgs->tickMs, &sleepTime);

  struct timespec elapsed;
  double elapsedTime;

  UserStats userStats;
  TypingStats tStats;

  // start the stopwatch
  startTs();

  int ticks = 0;

  // run until killStopWatch is called
  while (running) {
    elapsedTs(&elapsed);
    elapsedTime = tsToDouble(&elapsed);

    // make sure stdout is safe to write to for this (stopwatch) thread
    flockfile(stdout);

    // first lines should be the stats
    moveCursor(1, 1);
    eraseLine();

    printf(timeFmt, elapsedTime);
    displayTypingStats(&tStats);
    fflush(stdout);

    // now allow other threads (main) to write to it
    funlockfile(stdout);

    // sleep for a tick
    nanosleep(&sleepTime, NULL);
    ticks++;

    // time to recalculate
    if (ticks >= swArgs->statsTicks) {
      userStats = atomic_load(&stats);
      calculateTypingStats(&userStats, &tStats, elapsedTime);
      ticks = 0;
    }
  }

  // cleanup
  free(swArgs);

  return NULL;
}
