#ifndef STOPWATCH_H
#define STOPWATCH_H

#include "typingUtils.h"
#include <time.h>

struct SWThreadArgs {
  long long tickMs;
  int statsTicks; // how many ticks before stats are recalculated
};

void startTs(void);
void elapsedTs(struct timespec *ts);
void killStopWatch(void);
void updateStats(const struct UserStats *userStats);
void startStopwatch(long long sleepMs, int statsTicks);
void msToTs(long long ms, struct timespec *ts);
double tsToDouble(const struct timespec *ts);

#endif
