/*
 * CPSC 359 Assignment 4: SNES Controller Driver
 * Winter 2022
 *
 * Team members:
 *   Umar Hassan (umar.hassan@ucalgary.ca)
 *   UCID: 30130208
 *
 *   Anil Mawji (anil.mawji@ucalgary.ca)
 *   UCID: 30099809
 */

#include <controller.h>
#include <fcntl.h>
#include <gpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>

// Controller I/O lines
#define CLK 11
#define LAT 9
#define DAT 10

// Register offsets used for pin I/O
#define GPSET0 7   // gpioPtr + 7  (7*4 = 28-bit offset)
#define GPCLR0 10  // gpioPtr + 10 (10*4 = 40-bit offset)
#define GPLEV0 13  // gpioPtr + 13 (13*4 = 52-bit offset)

// Button macros
#define PRESSED 0
#define RELEASED 1

const char* BUTTON_NAMES[NUM_REAL_BUTTONS] = {"B",
                                              "Y",
                                              "Select",
                                              "Start",
                                              "Joy-pad UP",
                                              "Joy-pad DOWN",
                                              "Joy-pad LEFT",
                                              "Joy-pad RIGHT",
                                              "A",
                                              "X",
                                              "Left",
                                              "Right"};

unsigned int* gpio = NULL;

int buttons[NUM_BUTTONS];
// Keeps track of previous button states
int oldButtons[NUM_BUTTONS];
// Keeps track of seconds since a specific button was pressed
time_t lastPress[NUM_BUTTONS];
// Keeps track of seconds since any button was pressed
// Using a separate variable is more efficient than searching through lastPress
time_t lastGlobalPress;

float buttonDelay = 0.1;

const char* getButtonName(int i) { return BUTTON_NAMES[i]; }

// Returns seconds elapsed since a given time
float secondsElapsed(time_t start) {
  return (float)(clock() - start) / CLOCKS_PER_SEC;
}

// Returns seconds elapsed since any button on the controller was pressed
float secondsSinceLastButtonPress() { return secondsElapsed(lastGlobalPress); }

void setButtonDelay(float delay) { buttonDelay = delay; }

// Force a delay between button presses
// Button presses will only be registered every DELAY seconds
int shouldRegisterPress(int i) {
  // Check if the button has been detected as pressed
  // Must be at least DELAY seconds since button i was pressed
  int pressed = secondsElapsed(lastPress[i]) > buttonDelay;

  if (pressed) {
    time_t now = clock();
    // Update the time of last press for button i
    lastPress[i] = now;
    lastGlobalPress = now;
  }

  return pressed;
}

// Check whether a button has been pressed and held down
int isButtonHeld(int i) {
  return buttons[i] == PRESSED && shouldRegisterPress(i);
}

// Check whether a button has been pressed
// Events used with this function are only triggered once
int isButtonPressed(int i) {
  return buttons[i] == PRESSED && oldButtons[i] == RELEASED &&
         shouldRegisterPress(i);
}

int isButtonReleased(int i) { return buttons[i] == RELEASED; }

void INP_GPIO(int p) {
  // Determine the GP function and clear the 3 bits associated with pin p
  *(gpio + (p / 10)) &= ~(7 << ((p % 10) * 3));
}

void OUT_GPIO(int p) {
  // Set the bits in the register to 000 so we can prepare for setting them to
  // 001
  INP_GPIO(p);
  // Determine the GP function and set the 3 bits associated with pin p to 001
  *(gpio + (p / 10)) |= (1 << ((p % 10) * 3));
}

void initSNES() {
  gpio = getGPIOPtr();

  // Sanity check
  if (gpio == NULL) {
    fprintf(stderr,
            "Fatal error: GPIO must be initialized before accessing the "
            "controller! "
            "Aborting...\n");
    exit(1);
  }

  OUT_GPIO(CLK);
  OUT_GPIO(LAT);
  INP_GPIO(DAT);

  time_t now = clock();

  for (int i = 0; i < NUM_BUTTONS; i++) {
    oldButtons[i] = RELEASED;
    buttons[i] = RELEASED;
    lastPress[i] = now;
  }
}

void writeGPIO(int p, int b) {
  if (b) {
    // Write 1 to pin p by changing the pth bit in the set register to 1
    gpio[GPSET0] = 1 << p;
  } else {
    // Write 0 to pin p by changing the pth bit in the clear register to 1
    gpio[GPCLR0] = 1 << p;
  }
}

int readGPIO(int p) {
  // Read the value stored at pin p
  return (gpio[GPLEV0] >> p) & 1;
}

void writeLatch(int b) {
  // Write a bit to the controller latch line
  writeGPIO(LAT, b);
}

void writeClock(int b) {
  // Write a bit to the controller clock line
  writeGPIO(CLK, b);
}

int readData() {
  // Read a bit from the controller data line
  return readGPIO(DAT);
}

void readSNES() {
  // Update button states
  for (int i = 0; i < NUM_BUTTONS; i++) {
    // Save current states
    oldButtons[i] = buttons[i];
    // Reset states
    buttons[i] = RELEASED;
  }

  // Send signal to controller
  writeClock(1);
  writeLatch(1);
  delayMicroseconds(12);
  writeLatch(0);

  for (int i = 0; i < NUM_BUTTONS; i++) {
    delayMicroseconds(6);
    // Falling edge
    writeClock(0);
    delayMicroseconds(6);
    // Read bit i
    buttons[i] = readData();
    // Rising edge
    writeClock(1);
  }
}
