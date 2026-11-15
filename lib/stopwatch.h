#ifndef STOPWATCH_H
#define STOPWATCH_H

#include "typingUtils.h"
#include <time.h>

void startTs(void);
void elapsedTs(struct timespec *ts);
void killStopWatch(void);
void updateStats(struct TypingStats *userStats);
void startStopwatch(long long sleepMs);
double pollElapsedSec(void);

#endif
