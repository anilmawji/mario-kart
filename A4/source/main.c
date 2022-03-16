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
#include <framebuffer.h>
#include <gpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define CELL_WIDTH 30
#define CELL_HEIGHT 30
#define MAP_WIDTH (SCREEN_WIDTH / CELL_WIDTH)
#define MAP_HEIGHT (SCREEN_HEIGHT / CELL_HEIGHT)

FrameBufferInfo fbinfo;
Pixel *pixel;

enum color {
  RED = 0xF800,
  BLUE = 0x0000FF,
  GREY = 0x9493
};

unsigned int *gpio;

struct State {
  int playerX;
  int playerY;
  // int gameMap[MAP_WIDTH][MAP_HEIGHT];
} state;

enum cellType { BACKGROUND, PLAYER };
int gameMap[MAP_WIDTH][MAP_HEIGHT];

// Location of center of screen in pixels
int centerX;
int centerY;

void drawPixel(Pixel *pixel) {
  long int location = (pixel->x + fbinfo.xOffset) * (fbinfo.bitsPerPixel / 8) +
                      (pixel->y + fbinfo.yOffset) * fbinfo.lineLength;
  *((unsigned short int *)(fbinfo.fbptr + location)) = pixel->color;
}

void clearScreen() { memset(fbinfo.fbptr, 0, fbinfo.screenSizeBytes); }

void drawCell(int cellX, int cellY, int color) {
  pixel->color = color;

  for (int y = 0; y < CELL_HEIGHT; y++) {
    for (int x = 0; x < CELL_WIDTH; x++) {
      pixel->x = cellX + x;
      pixel->y = cellY + y;

      drawPixel(pixel);
    }
  }
}

void drawMap() {
  int cellX, cellY;

  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      cellX = x * CELL_WIDTH + centerX;
      cellY = y * CELL_HEIGHT + centerY;

      if (x == state.playerX && y == state.playerY) {
        drawCell(cellX, cellY, RED);
      } else {
        drawCell(cellX, cellY, GREY);
      }
    }
  }
}

void printMap() {
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      printf("%d ", gameMap[y][x]);
    }
    printf("\n");
  }
  printf("\n\n");
}

void initGame() {
  centerX = (fbinfo.screenWidth - SCREEN_WIDTH) / 2;
  centerY = (fbinfo.screenHeight - SCREEN_HEIGHT) / 2;
  pixel = malloc(sizeof(Pixel));

  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      gameMap[y][x] = BACKGROUND;
    }
  }

  state.playerX = 0;
  state.playerY = 0;
  gameMap[state.playerY][state.playerX] = PLAYER;
}

int clamp(int val, int min, int max) {
  if (val > max) return max;
  if (val < min) return min;
  return val;
}

void update() {
  gameMap[state.playerY][state.playerX] = BACKGROUND;

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
  gameMap[state.playerY][state.playerX] = PLAYER;
}

int main(int argc, char *argv[]) {
  fbinfo = initFbInfo();
  initGPIO();
  initSNES();
  initGame();

  clearScreen();

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
