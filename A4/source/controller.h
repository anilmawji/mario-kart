#define NUM_BUTTONS 16
#define NUM_REAL_BUTTONS 12

typedef enum {
  B,
  Y,
  SELECT,
  START,
  JOY_PAD_UP,
  JOY_PAD_DOWN,
  JOY_PAD_LEFT,
  JOY_PAD_RIGHT,
  A,
  X,
  LEFT,
  RIGHT
} ControllerButton;

void initSNES();

void readSNES();

const char* getButtonName(int i);

int isButtonHeld(int i);

int isButtonPressed(int i);

int isButtonReleased(int i);

int isAnyButtonPressed();

float secondsSinceLastButtonPress();

void setButtonDelay(float delay);
