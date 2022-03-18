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
#include <gpio.h>
#include <mariokart.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>
#include <renderer.h>

#define VIEWPORT_WIDTH 1280
#define VIEWPORT_HEIGHT 720
#define CELL_WIDTH 30
#define CELL_HEIGHT 30
#define MAP_WIDTH (VIEWPORT_WIDTH / CELL_WIDTH)
#define MAP_HEIGHT (VIEWPORT_HEIGHT / CELL_HEIGHT)

enum color { RED = 0xF800, BLUE = 0x0000FF, GREY = 0x9493 };

unsigned int *gpio;

struct State {
  int gameMap[MAP_WIDTH][MAP_HEIGHT];
  int playerX;
  int playerY;
  int speed;
  int score;
  int lives;
  time_t timeLeft;
  int win;
  int lose;
  int activePowerup;
  int paused;
} state;

enum cellType { BACKGROUND, PLAYER };
enum powerup { STAR };

short int *marioPtr = (short int *)marioKartImage.pixel_data;

// Eg. A speed of 1 leads to a delay of 1/(5*1) = 1/5 = 0.2
//     A speed of 2 leads to a delay of 1/(5*2) = 1/10 = 0.1
void setPlayerSpeed(float speed) { setButtonDelay(1/(5*speed)); }

void drawMap() {
  int cellX, cellY;

  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      cellX = x * CELL_WIDTH + centerX;
      cellY = y * CELL_HEIGHT + centerY;

      if (x == state.playerX && y == state.playerY) {
        drawImage(marioPtr, cellX, cellY, CELL_WIDTH, CELL_HEIGHT);
      } else {
        drawRect(cellX, cellY, CELL_WIDTH, CELL_HEIGHT, GREY);
      }
    }
  }
}

void printMap() {
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      printf("%d ", state.gameMap[y][x]);
    }
    printf("\n");
  }
  printf("\n\n");
}

void initGame() {
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      state.gameMap[y][x] = BACKGROUND;
    }
  }

  state.playerX = 0;
  state.playerY = 0;
  state.gameMap[state.playerY][state.playerX] = PLAYER;
}

int clamp(int val, int min, int max) {
  if (val > max) return max;
  if (val < min) return min;
  return val;
}

void update() {
  state.gameMap[state.playerY][state.playerX] = BACKGROUND;

  if (isButtonHeld(JOY_PAD_UP)) {
    state.playerY = clamp(state.playerY - 1, 0, MAP_HEIGHT - 1);
  } else if (isButtonHeld(JOY_PAD_DOWN)) {
    state.playerY = clamp(state.playerY + 1, 0, MAP_HEIGHT - 1);
  } else if (isButtonHeld(JOY_PAD_LEFT)) {
    state.playerX = clamp(state.playerX - 1, 0, MAP_WIDTH - 1);
  } else if (isButtonHeld(JOY_PAD_RIGHT)) {
    state.playerX = clamp(state.playerX + 1, 0, MAP_WIDTH - 1);
  }

  // Update player location in map
  state.gameMap[state.playerY][state.playerX] = PLAYER;
}

int main(int argc, char *argv[]) {
  fbinfo = initFbInfo();
  initGPIO();
  initSNES();
  initRenderer(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
  initGame();

  clearScreen();
  setPlayerSpeed(1);

  do {
    drawMap();
    // printMap();
    readSNES();
    update();
  } while (!isButtonPressed(START));

  // Deallocate memory
  free(pixel);
  pixel = NULL;
  munmap(fbinfo.fbptr, fbinfo.screenSizeBytes);

  return 0;
}
