// could probably come up with a better name but this displays a stopwatch and
// the user stats from a separate thread

#pragma once

#include "typingUtils.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// update frequency of stopwatch thread
static const long long SLEEP_INTERVAL_MS = 250;

// timespec just stores a number of seconds (time_t) and nanoseconds (unsigned
// long long?)
static struct timespec start;

// atomic basically means these are thread-safe (but not lock-free)
// at the cost of some extra overhead
// for structs its still recommended to access them indirectly via atomic_load()
static _Atomic(double) elapsedTime;
static _Atomic(bool) running;
static _Atomic(struct TypingStats) stats;

void startTs();
void elapsedTs(struct timespec *ts);
void killStopWatch();
void updateStats(struct TypingStats *userStats);
void startStopwatch();
double pollElapsedSec();

static void *stopwatchThread(void *arg);
static double tsToDouble(struct timespec *ts);

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

// call for stopwatch thread to kill itself
void killStopWatch() { running = false; }

// start the stopwatch in a separate thread
void startStopwatch() {
  running = true;

  pthread_t thread;
  pthread_create(&thread, NULL, &stopwatchThread, NULL);
}

// update stats from other threads
void updateStats(struct TypingStats *userStats) { stats = *userStats; }

double pollElapsedSec() { return elapsedTime; }

// actual thread
static void *stopwatchThread(void *arg) {
  // convert miliseconds to nanoseconds
  struct timespec sleepInterval = {0, SLEEP_INTERVAL_MS * 1e6};
  struct timespec elapsed;

  // copy of atomic struct
  struct TypingStats currStats;

  // start the stopwatch
  startTs();

  // run until killStopWatch is called
  while (running) {
    // thread-safe way to access a struct
    currStats = atomic_load(&stats);

    elapsedTs(&elapsed);
    elapsedTime = tsToDouble(&elapsed);

    // set cursor position and terminal mode
    moveCursor(1, 1);
    ansiPrint(1, HIDDEN);

    printf("%s\rTime taken: %.2lfs\n", ERASE_LINE, elapsedTime);
    displayTypingStats(&currStats, elapsedTime);
    fflush(stdout);

    // lap every SLEEP_INTERVAL_MS ms
    nanosleep(&sleepInterval, NULL);
  }

  pthread_exit(NULL);
}

static double tsToDouble(struct timespec *ts) {
  return (double)ts->tv_sec + (double)ts->tv_nsec / 1e9;
}
