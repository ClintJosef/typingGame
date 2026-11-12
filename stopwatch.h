#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// timespec just stores a number of seconds (time_t) and nanoseconds (unsigned
// long long?)
static struct timespec start;
static const long long SLEEP_INTERVAL_MS = 250;

void startTs();
void killThread(pthread_t thread);
pthread_t startStopwatch();
struct timespec elapsedTs();

static double tsToDouble(struct timespec *ts);
static void *stopwatchThread(void *arg);

void startTs() {
  // basically just a higher resolution time()
  timespec_get(&start, TIME_UTC);
}

// start the stopwatch in a separate thread
pthread_t startStopwatch() {
  pthread_t thread;
  pthread_create(&thread, NULL, &stopwatchThread, NULL);

  return thread;
}

// wall clock time since startTs() was called
struct timespec elapsedTs() {
  struct timespec elapsed;
  timespec_get(&elapsed, TIME_UTC);

  elapsed.tv_sec -= start.tv_sec;
  elapsed.tv_nsec -= start.tv_nsec;
  if (elapsed.tv_nsec < 0) {
    elapsed.tv_sec--;
    elapsed.tv_nsec += 1e9;
  }

  return elapsed;
}

static double tsToDouble(struct timespec *ts) {
  return (double)ts->tv_sec + (double)ts->tv_nsec / 1e9;
}

static void *stopwatchThread(void *arg) {
  struct timespec sleepInterval = {0, SLEEP_INTERVAL_MS * 1e6};

  startTs();
  while (true) {
    // hide cursor, move to pos (1, 1) in the terminal, and print the time
    struct timespec elapsed = elapsedTs();
    printf("\e[?25l\e[1;1fTime taken: %.2lfs\n", tsToDouble(&elapsed));

    nanosleep(&sleepInterval, NULL);
  }
}

void killThread(pthread_t thread) {
  // second arg is the signal (0 to error check)
  pthread_kill(thread, 0);
}
