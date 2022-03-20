/*
 * CPSC 359 Assignment 4: Mario Kart Game
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
#include <font.h>
#include <gpio.h>
#include <mario.h>
#include <menuassets.h>
#include <renderer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <timer.h>
#include <unistd.h>
#include <wiringPi.h>

#define VIEWPORT_WIDTH 1280
#define VIEWPORT_HEIGHT 720
#define CELL_WIDTH 32
#define CELL_HEIGHT 32
#define MAP_WIDTH (VIEWPORT_WIDTH / CELL_WIDTH)
#define MAP_HEIGHT (VIEWPORT_HEIGHT / CELL_HEIGHT - 1)
#define SCORE_CONST 5
#define BUFFER_SIZE 20

unsigned int *gpio;

typedef enum { MV_UP, MV_DOWN, MV_RIGHT, MV_LEFT } Direction;
enum cellType { BACKGROUND, PLAYER };

struct GameState {
  int gameMap[MAP_WIDTH][MAP_HEIGHT];
  int objectPositions[MAP_WIDTH][MAP_HEIGHT];
  int playerX;
  int playerY;
  Direction playerDirection;
  int speed;
  int lives;
  Timer timeLeft;
  int win;
  int lose;
} state;

char textBuffer[BUFFER_SIZE];

short int *marioSprites[] = {
    (short int *)mario_up.pixel_data, (short int *)mario_down.pixel_data,
    (short int *)mario_right.pixel_data, (short int *)mario_left.pixel_data};

short int *menuBackground = (short int *)menu_background.pixel_data;
short int *menuTitle = (short int *)menu_title.pixel_data;

// Eg. A speed of 1 leads to a delay of 1/(5*1) = 1/5 = 0.2
void setPlayerSpeed(float speed) { setButtonDelay(1 / (5 * speed)); }

void drawMap() {
  int cellX, cellY;

  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      cellX = x * CELL_WIDTH + centerX;
      cellY = (y + 1) * CELL_HEIGHT + centerY;

      if (x == state.playerX && y == state.playerY) {
        drawImage(marioSprites[state.playerDirection], cellX, cellY, CELL_WIDTH,
                  CELL_HEIGHT, TRANSPARENT, state.gameMap[y][x]);
      } else {
        drawRect(cellX, cellY, CELL_WIDTH, CELL_HEIGHT, state.gameMap[y][x]);
      }
    }
  }
}

void printMap() {
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      if (state.objectPositions[y][x] == BACKGROUND) {
        printf("%d ", state.gameMap[y][x]);
      } else {
        printf("%d ", state.objectPositions[y][x]);
      }
    }
    printf("\n");
  }
  printf("\n\n");
}

int calculateScore() {
  return (timerMillisElapsed(state.timeLeft) / 1000 + state.lives) *
         SCORE_CONST;
}

void initGame() {
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      if (x <= 8 && x >= 5) {
        state.gameMap[y][x] = GREY;
      } else {
        state.gameMap[y][x] = GREEN;
      }
      state.objectPositions[y][x] = BACKGROUND;
    }
  }

  state.playerX = 0;
  state.playerY = 0;
  state.playerDirection = MV_DOWN;
  state.timeLeft.secondsAllowed = 3 * 60;  // seconds
  state.lives = 4;
  state.objectPositions[state.playerY][state.playerX] = PLAYER;

  startTimer(state.timeLeft);
}

int clamp(int val, int min, int max) {
  if (val > max) return max;
  if (val < min) return min;
  return val;
}

void update() {
  state.objectPositions[state.playerY][state.playerX] = BACKGROUND;

  if (isButtonHeld(JOY_PAD_UP)) {
    state.playerY = clamp(state.playerY - 1, 0, MAP_HEIGHT - 1);
    state.playerDirection = MV_UP;
  } else if (isButtonHeld(JOY_PAD_DOWN)) {
    state.playerY = clamp(state.playerY + 1, 0, MAP_HEIGHT - 1);
    state.playerDirection = MV_DOWN;
  } else if (isButtonHeld(JOY_PAD_LEFT)) {
    state.playerX = clamp(state.playerX - 1, 0, MAP_WIDTH - 1);
    state.playerDirection = MV_LEFT;
  } else if (isButtonHeld(JOY_PAD_RIGHT)) {
    state.playerX = clamp(state.playerX + 1, 0, MAP_WIDTH - 1);
    state.playerDirection = MV_RIGHT;
  }

  // Update player location in map
  state.objectPositions[state.playerY][state.playerX] = PLAYER;
}

void drawGUI() {
  sprintf(textBuffer, "%04d", calculateScore());
  drawText(textBuffer, 4, centerX + 6 * CELL_WIDTH, centerY, 0);

  sprintf(textBuffer, "%02d", state.lives);
  drawText(textBuffer, 2, centerX + (12 + 6) * CELL_WIDTH, centerY, 0);

  formatTimeLeft(state.timeLeft, textBuffer);
  drawText(textBuffer, 5, centerX + (12 * 2 + 3) * CELL_WIDTH, centerY, 0);
}

void initGUI() {
  drawText("score ", 6, centerX, centerY, 0);
  drawText("lives ", 6, centerX + 12 * CELL_WIDTH, centerY, 0);
  drawText("time ", 5, centerX + (12 * 2 - 2) * CELL_WIDTH, centerY, 0);
}

void drawMenuScreen() { drawImage(menuBackground, 0, 0, 1280, 640, -1, RED); }

int main(int argc, char *argv[]) {
  fbinfo = initFbInfo();
  initGPIO();
  initSNES();
  initRenderer(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
  initGame();

  clearScreen();
  initGUI();
  setPlayerSpeed(1.5);

  do {
    // printMap();
    drawGUI();
    drawMap();
    // drawMenuScreen();
    readSNES();
    update();
  } while (!isButtonPressed(START));

  // Deallocate memory
  cleanUpRenderer();
  munmap(fbinfo.fbptr, fbinfo.screenSizeBytes);

  return 0;
}
