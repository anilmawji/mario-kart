/*
 * CPSC 359 Assignment 4
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>

#define MAP_WIDTH 20
#define MAP_HEIGHT 20
#define CELL_WIDTH 30
#define CELL_HEIGHT 30

unsigned int *gpio;

FrameBufferInfo fbinfo;
Pixel *pixel;

enum GAME_OBJECT { BACKGROUND, PLAYER };

struct State {
  int playerX;
  int playerY;
  // int gameMap[MAP_WIDTH][MAP_HEIGHT];
} state;

int gameMap[MAP_WIDTH][MAP_HEIGHT];

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

  for (int py = 0; py < CELL_HEIGHT; py++) {
    for (int px = 0; px < CELL_WIDTH; px++) {
      pixel->x = cellX + px;
      pixel->y = cellY + py;

      drawPixel(pixel);
    }
  }
}

void drawMap() {
  for (int cellY = 0; cellY < CELL_HEIGHT * MAP_HEIGHT; cellY += CELL_HEIGHT) {
    for (int cellX = 0; cellX < CELL_WIDTH * MAP_WIDTH; cellX += CELL_WIDTH) {
      int color = 0xF800;
      // temp
      if (cellX / CELL_WIDTH == state.playerX &&
          cellY / CELL_HEIGHT == state.playerY) {
        color = 0x0000FF;
      }
      drawCell(cellX + centerX, cellY + centerY, color);
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
  state.playerX = 0;
  state.playerY = 0;

  centerX = (fbinfo.screenWidth - 1280) / 2;
  centerY = (fbinfo.screenHeight - 720) / 2;
  pixel = malloc(sizeof(Pixel));

  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      gameMap[y][x] = 0;
    }
  }
  gameMap[state.playerY][state.playerX] = 1;
}

void update() {
  gameMap[state.playerY][state.playerX] = 0;

  if (isTimedPress(JOY_PAD_UP)) {
    state.playerY--;
  } else if (isTimedPress(JOY_PAD_DOWN)) {
    state.playerY++;
  } else if (isTimedPress(JOY_PAD_LEFT)) {
    state.playerX--;
  } else if (isTimedPress(JOY_PAD_RIGHT)) {
    state.playerX++;
  }

  // Update player location in map
  gameMap[state.playerY][state.playerX] = 1;
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
    update();
    readSNES();
  } while (!isButtonPressed(START));

  // Deallocate memory
  free(pixel);
  pixel = NULL;
  munmap(fbinfo.fbptr, fbinfo.screenSizeBytes);

  return 0;
}
