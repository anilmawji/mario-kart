typedef struct {
  double start;
  double passed;
  int secondsAllowed;  // How long timer should last in seconds
} Timer;

void startTimer(Timer time);

double timerMillisElapsed(Timer time);

double timerMillisLeft(Timer time);

void formatTimeLeft(Timer time, char *str);

int isTimerFinished(Timer time);

double timeDiff(Timer time);