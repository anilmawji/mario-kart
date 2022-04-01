typedef struct {
  double startTime;
  double pausedTime;
  double passed;
  int secondsAllowed;  // How long timer should last in seconds
} Timer;

void startTimer(Timer* time);

void pauseTimer(Timer* time);

void resumeTimer(Timer* time);

double timerMillisElapsed(Timer* time);

double timerMillisLeft(Timer* time);

void formatTimeLeft(Timer* time, char *str);

int isTimerFinished(Timer* time);

double timerSecondsElapsed(Timer* time);

double timerSecondsLeft(Timer* time);

double timeDiff(Timer* time);