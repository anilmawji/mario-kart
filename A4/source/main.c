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
#include <mariokart.h>
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

// Location of center of screen in pixels
int centerX;
int centerY;

short int *marioPtr = (short int *)marioKartImage.pixel_data;

void drawPixel(Pixel *pixel) {
  long int location = (pixel->x + fbinfo.xOffset) * (fbinfo.bitsPerPixel / 8) +
                      (pixel->y + fbinfo.yOffset) * fbinfo.lineLength;
  *((unsigned short int *)(fbinfo.fbptr + location)) = pixel->color;
}

void clearScreen() { memset(fbinfo.fbptr, 0, fbinfo.screenSizeBytes); }

void setSpeed(int speed) { setButtonDelay(speed / 10); }

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

void drawImage(short int *imagePixels, int posX, int posY, int width,
               int height) {
  int i = 0;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      pixel->color = imagePixels[i];
      pixel->x = x + posX;
      pixel->y = y + posY;

      drawPixel(pixel);
      i++;
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
        // drawCell(cellX, cellY, RED);
        drawImage(marioPtr, cellX, cellY, CELL_WIDTH, CELL_HEIGHT);
      } else {
        drawCell(cellX, cellY, GREY);
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
  centerX = (fbinfo.screenWidth - SCREEN_WIDTH) / 2;
  centerY = (fbinfo.screenHeight - SCREEN_HEIGHT) / 2;
  pixel = malloc(sizeof(Pixel));

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
    setSpeed(4);
  } else if (isButtonHeld(JOY_PAD_DOWN)) {
    state.playerY = clamp(state.playerY + 1, 0, MAP_HEIGHT - 1);
    setSpeed(4);
  } else if (isButtonHeld(JOY_PAD_LEFT)) {
    state.playerX = clamp(state.playerX - 1, 0, MAP_WIDTH - 1);
    setSpeed(2);
  } else if (isButtonHeld(JOY_PAD_RIGHT)) {
    state.playerX = clamp(state.playerX + 1, 0, MAP_WIDTH - 1);
    setSpeed(2);
  }

  // Update player location in map
  state.gameMap[state.playerY][state.playerX] = PLAYER;
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
