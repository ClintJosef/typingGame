#include "stopwatch.h"
#include "ansi.h"
#include "typingUtils.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
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
static _Atomic(struct UserStats) stats;

static void *stopwatchThread(void *arg);
static double tsToDouble(struct timespec *ts);

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
  running = false;
  pthread_join(swThread, NULL);
}

// start the stopwatch in a separate thread
void startStopwatch(long long sleepMs) {
  running = true;

  pthread_create(&swThread, NULL, &stopwatchThread, (void *)sleepMs);
}

// update stats from other threads
void updateStats(struct UserStats *userStats) { stats = *userStats; }

// actual thread
static void *stopwatchThread(void *arg) {
  long long sleepMs = (long long)arg;
  // convert miliseconds to nanoseconds
  struct timespec sleepInterval = {.tv_sec = 0, .tv_nsec = sleepMs * 1e6};
  struct timespec elapsed;
  double elapsedTime;

  // copy of atomic struct
  struct UserStats currStats;

  // start the stopwatch
  startTs();

  // run until killStopWatch is called
  while (running) {
    // thread-safe way to access a struct
    currStats = atomic_load(&stats);

    elapsedTs(&elapsed);
    elapsedTime = tsToDouble(&elapsed);

    // make sure stdout is safe to write to for this (stopwatch) thread
    flockfile(stdout);

    // first lines should be the stats
    moveCursor(1, 1);
    hideCursor();

    printf("%sTime taken: %.1lfs\n", ERASE_LINE, elapsedTime);
    displayUserStats(&currStats, elapsedTime);
    fflush(stdout);

    // now allow other threads (main) to write to it
    funlockfile(stdout);

    // lap every SLEEP_INTERVAL_MS ms
    nanosleep(&sleepInterval, NULL);
  }

  return NULL;
}

static double tsToDouble(struct timespec *ts) {
  return (double)ts->tv_sec + (double)ts->tv_nsec / 1e9;
}
