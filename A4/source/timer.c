#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void startTimer(Timer* timer) { timer->startTime = clock(); }

void pauseTimer(Timer* timer) { timer->pausedTime = clock(); }

void resumeTimer(Timer* timer) {
  double pauseDuration = clock() - timer->pausedTime;

  timer->startTime += pauseDuration;
}

double timerSecondsElapsed(Timer* timer) {
  return (double)(clock() - timer->startTime) / CLOCKS_PER_SEC;
}

double timerSecondsLeft(Timer* timer) {
  return timer->secondsAllowed - timerSecondsElapsed(timer);
}

double timeDiff(Timer* timer) { return (clock() - timer->startTime); }

double timerMillisElapsed(Timer* timer) {
  return (double)(clock() - timer->startTime) * 1000 / CLOCKS_PER_SEC;
}

double timerMillisLeft(Timer* timer) {
  return timer->secondsAllowed * 1000 - timerMillisElapsed(timer);
}

int isTimerFinished(Timer* timer) {
  return timerMillisElapsed(timer) >= timer->secondsAllowed;
}

void formatTimeLeft(Timer* timer, char *str) {
  double total = timerMillisLeft(timer);
  int mins = total / 1000 / 60;
  int secs = (int)(total / 1000) % 60;

  sprintf(str, "%02d %02d", mins, secs);
}
