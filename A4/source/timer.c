#include <time.h>
#include <timer.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void startTimer(Timer time) {
  
  time.start = clock(); 
  
  }

double timerMillisElapsed(Timer time) {
  return (double)(clock() - time.start) * 1000 / CLOCKS_PER_SEC;
}

double timeDiff(Timer time){
  return (clock() - time.start);
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
  //int mils = 0;

  sprintf(str, "%02d %02d", mins, secs);
}
