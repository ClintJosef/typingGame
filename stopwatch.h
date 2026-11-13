#pragma once

#include "typingUtils.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// timespec just stores a number of seconds (time_t) and nanoseconds (unsigned
// long long?)
static struct timespec start;
// atomic basically just means elapsedTime is thread-safe (but not lock-free)
// at the cost of some extra overhead
static _Atomic(double) elapsedTime;
static const long long SLEEP_INTERVAL_MS = 250;

void startTs();
void elapsedTs(struct timespec *ts);
void killThread(pthread_t thread);
pthread_t startStopwatch();
double pollElapsedSec();

static double tsToDouble(struct timespec *ts);
static void *stopwatchThread(void *arg);

void startTs() {
  // basically just a higher resolution time()
  timespec_get(&start, TIME_UTC);
}

// wall clock time since startTs() was called
void elapsedTs(struct timespec *ts) {
  timespec_get(ts, TIME_UTC);

  ts->tv_sec -= start.tv_sec;
  ts->tv_nsec -= start.tv_nsec;
  if (ts->tv_nsec < 0) {
    ts->tv_sec--;
    ts->tv_nsec += 1e9;
  }
}

void killThread(pthread_t thread) {
  // second arg is the signal (0 to error check)
  pthread_kill(thread, 1);
}

// start the stopwatch in a separate thread
pthread_t startStopwatch() {
  pthread_t thread;
  pthread_create(&thread, NULL, &stopwatchThread, NULL);

  return thread;
}

// will there be a race condition if elapsedTime is read-only everywhere else?
double pollElapsedSec() { return elapsedTime; }

static double tsToDouble(struct timespec *ts) {
  return (double)ts->tv_sec + (double)ts->tv_nsec / 1e9;
}

static void *stopwatchThread(void *arg) {
  // convert miliseconds to nanoseconds
  struct timespec sleepInterval = {0, SLEEP_INTERVAL_MS * 1e6};
  struct timespec elapsed;

  // start the stopwatch
  startTs();

  // run until killThread() is externally called
  while (true) {
    moveCursor(1, 1);
    printf("%sTime taken: %.2lfs", HIDDEN, elapsedTime);
    fflush(stdout);

    // lap every SLEEP_INTERVAL_MS ms
    nanosleep(&sleepInterval, NULL);
    elapsedTs(&elapsed);
    elapsedTime = tsToDouble(&elapsed);
  }
}
