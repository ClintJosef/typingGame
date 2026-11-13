#ifndef STOPWATCH_H
#define STOPWATCH_H

#include "typingUtils.h"
#include <time.h>

void startTs(void);
void elapsedTs(struct timespec *ts);
void killStopWatch(void);
void updateStats(struct TypingStats *userStats);
void startStopwatch(void);
double pollElapsedSec(void);

#endif
