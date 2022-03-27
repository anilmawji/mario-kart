#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void startTimer(Timer time) { time.start = clock(); }

void stopTimer(Timer time) {
  
}

double timerSecondsElapsed(Timer time) {
  return (double)(clock() - time.start) / CLOCKS_PER_SEC;
}

double timerSecondsLeft(Timer time) {
  return time.secondsAllowed - timerSecondsElapsed(time);
}

double timeDiff(Timer time) { return (clock() - time.start); }

double timerMillisElapsed(Timer time) {
  return (double)(clock() - time.start) * 1000 / CLOCKS_PER_SEC;
}

double timerMillisLeft(Timer time) {
  return time.secondsAllowed * 1000 - timerMillisElapsed(time);
}

int isTimerFinished(Timer time) {
  return timerMillisElapsed(time) >= time.secondsAllowed;
}

void formatTimeLeft(Timer time, char *str) {
  double total = timerMillisLeft(time);
  int mins = total / 1000 / 60;
  int secs = (int)(total / 1000) % 60;

  sprintf(str, "%02d %02d", mins, secs);
}
