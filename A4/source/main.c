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
#include <time.h>
#include <timer.h>
#include <unistd.h>
#include <wiringPi.h>

#define VIEWPORT_WIDTH 1280
#define VIEWPORT_HEIGHT 720
#define CELL_WIDTH 32
#define CELL_HEIGHT 32
#define MAP_WIDTH (VIEWPORT_WIDTH / CELL_WIDTH)
// Subtract 1 to make space for the row of gui labels on top
#define MAP_HEIGHT (VIEWPORT_HEIGHT / CELL_HEIGHT - 1)
#define SCORE_CONST 5
#define BUFFER_SIZE 20

unsigned int *gpio;

typedef enum { MV_UP, MV_DOWN, MV_RIGHT, MV_LEFT } Direction;

// Used by objectPositions to determine the type of object occupying a cell
// Cells marked as background are not occupied by an object
enum cellType {
  BACKGROUND,
  PLAYER,
  MOVINGOBSTACLE,
  STATICOBSTACLE,
  POWERUP1,
  POWERUP2,
  POWERUP3
};

struct GameState {
  int gameMap[MAP_HEIGHT][MAP_WIDTH];
  int objectPositions[MAP_HEIGHT][MAP_WIDTH];
  int playerX;
  int playerY;
  Direction playerDirection;
  int speed;
  int lives;
  Timer timeLeft;
  int win;
  int lose;
} state;

int mapX, mapY;

char textBuffer[BUFFER_SIZE];

short int *marioSprites[] = {
    (short int *)mario_up.pixel_data, (short int *)mario_down.pixel_data,
    (short int *)mario_right.pixel_data, (short int *)mario_left.pixel_data};

short int *menuBackground = (short int *)menu_background.pixel_data;
short int *menuTitle = (short int *)menu_title.pixel_data;

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



void drawMap() {
  int cellX, cellY;

  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      cellX = mapX + x * CELL_WIDTH;
      cellY = mapY + y * CELL_HEIGHT;

      if (state.objectPositions[y][x] == BACKGROUND) {
        drawFillRect(cellX, cellY, CELL_WIDTH, CELL_HEIGHT,
                     state.gameMap[y][x]);
      } else {
        drawImage(marioSprites[state.playerDirection], cellX, cellY, CELL_WIDTH,
                  CELL_HEIGHT, TRANSPARENT, state.gameMap[y][x]);
      }
    }
  }
}

void addObstacle(int x) {
  for (int y = 0; y < MAP_HEIGHT; y++) {
    state.gameMap[y][x] = GREY;
  }
}

void initGame() {
  mapX = viewportX;
  mapY = viewportY + CELL_HEIGHT;

  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      state.gameMap[y][x] = GREEN;
      state.objectPositions[y][x] = BACKGROUND;
    }
  }

  addObstacle(4);
  addObstacle(6);

  state.playerX = 3;
  state.playerY = MAP_HEIGHT / 2 - 1;
  state.playerDirection = MV_RIGHT;
  state.timeLeft.secondsAllowed = 3 * 60;  // seconds
  state.lives = 4;
  state.objectPositions[state.playerY][state.playerX] = PLAYER;

  startTimer(state.timeLeft);
}

int calculateScore() {
  return (timerMillisElapsed(state.timeLeft) / 1000 + state.lives) *
         SCORE_CONST;
}

void drawGuiValues() {
  sprintf(textBuffer, "%04d", calculateScore());
  drawText(textBuffer, 4, viewportX + 6 * CELL_WIDTH, viewportY, 0);

  sprintf(textBuffer, "%02d", state.lives);
  drawText(textBuffer, 2, viewportX + (12 + 3.5 + 6) * CELL_WIDTH, viewportY,
           0);

  formatTimeLeft(state.timeLeft, textBuffer);
  drawText(textBuffer, 5, (MAP_WIDTH + 5) * CELL_WIDTH, viewportY, 0);
}

void drawGuiLabels() {
  drawText("score ", 6, viewportX, viewportY, 0);
  drawText("lives ", 6, viewportX + (12 + 3.5) * CELL_WIDTH, viewportY, 0);
  drawText("time ", 5, MAP_WIDTH * CELL_WIDTH, viewportY, 0);
}

int clamp(int val, int min, int max) {
  if (val >= max) return max;
  if (val <= min) return min;
  return val;
}

int randomNumber(int min, int max) { return rand() % (max + 1 - min) + min; }

void generateRandomMap() {
  initGame();

  state.playerY = 2;
  state.playerX = 0;
  state.objectPositions[state.playerY][state.playerX] = PLAYER;

  int allowedMovable = 100;
  int allowedStatic = 50;
  int y = 1;
  while (y < MAP_HEIGHT) {

    int x = 1;
    int laneWidth = 0;
    while (x < MAP_WIDTH - 4) {
      int cellVal = randomNumber(0, 5);
      if (cellVal == 1 && allowedMovable > 0) {
        state.objectPositions[y][x] = MOVINGOBSTACLE;
        allowedMovable -= 1;
      }
      addObstacle(x);
      x++;
      if (laneWidth == 6) {
        int xCopy = x;
        x += 3;
        while (xCopy < x) {
          cellVal = randomNumber(0, 5);
          if (cellVal == 1 && allowedStatic > 0) {
            state.objectPositions[y][xCopy] = STATICOBSTACLE;
            allowedStatic -= 1;
          }

          xCopy++;
        }
        laneWidth = 0;
      }
      laneWidth++;
    }

    y += 2;
  }

  // y = 0;
  // while (y < MAP_HEIGHT) {
  //   int x = 8;
  //   while (x < MAP_WIDTH - 2) {
  //     int cellVal = randomNumber(0, 5);
  //     if (cellVal == 1 && allowedStatic > 0) {
  //       state.objectPositions[y][x] = STATICOBSTACLE;
  //       allowedStatic -= 1;
  //     }
  //     x += 6;
  //   }
  //   y++;
  // }
}

void moveMovable(int yStart) {
  int playerCollision = 0;
  for (int y = yStart; y < MAP_HEIGHT; y += 2) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      if (state.objectPositions[y][x] == MOVINGOBSTACLE) {
        if (state.objectPositions[(y + 1) % MAP_HEIGHT][x] == PLAYER) {
          playerCollision = 1;
          break;
        }
        state.objectPositions[(y + 1) % MAP_HEIGHT][x] = MOVINGOBSTACLE;
        state.objectPositions[y][x] = BACKGROUND;
      }
      if (playerCollision) {
        break;
      }
    }
  }
  if (playerCollision) {
    state.lives = state.lives - 1;
    generateRandomMap();
  }
}


void updatePlayer() {
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
  if (state.objectPositions[state.playerY][state.playerX] == MOVINGOBSTACLE ||state.objectPositions[state.playerY][state.playerX] == STATICOBSTACLE) {
    state.lives = state.lives - 1;
    generateRandomMap();
  } else {
    // Update player location in map
    state.objectPositions[state.playerY][state.playerX] = PLAYER;
  }
}

// Eg. A speed of 1 leads to a delay of 1/(5*1) = 1/5 = 0.2
void setPlayerSpeed(float speed) { setButtonDelay(1 / (5 * speed)); }

//TODO
void drawMenuButton(int buttonIndex, char* text) {

}

void drawMenuScreen() {
  // drawImage(menuBackground, 0, 0, 1280, 640, -1, RED);

  int titleX;

  drawImage(menuTitle, viewportX + (VIEWPORT_WIDTH - menu_title.width) / 2,
            mapY, menu_title.width, menu_title.height, WHITE, BLACK);

  int startBtnX = viewportX + (VIEWPORT_WIDTH - 5 * CELL_WIDTH) / 2;
  int startBtnY = viewportY + (VIEWPORT_HEIGHT - CELL_HEIGHT) / 2;
  
  int quitBtnX = viewportX + (VIEWPORT_WIDTH - 4 * CELL_WIDTH) / 2;
  int quitBtnY =
      viewportY + (VIEWPORT_HEIGHT - CELL_HEIGHT) / 2 + 3 * CELL_HEIGHT;

  drawText("start", 5, startBtnX, startBtnY, 0);
  drawText("quit", 4, quitBtnX, quitBtnY, 0);

  drawStrokeRect(startBtnX - CELL_WIDTH / 2, startBtnY - CELL_HEIGHT / 2,
                 (5 + 1) * CELL_WIDTH, 2 * CELL_HEIGHT, 4, GREY, BLUE);

  drawStrokeRect(quitBtnX - CELL_WIDTH / 2, quitBtnY - CELL_HEIGHT / 2,
                 (4 + 1) * CELL_WIDTH, 2 * CELL_HEIGHT, 4, GREY, BLUE);
}



int main(int argc, char *argv[]) {
   srand(time(NULL));
  fbinfo = initFbInfo();
  initGPIO();
  initSNES();
  initRenderer(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
  initGame();

  clearScreen();
  // drawGuiLabels();
  setPlayerSpeed(1.5);

  double time = clock();
  generateRandomMap();
  int yStart = 1;

  while (!isButtonPressed(START)) {
    // printMap();
    drawGuiValues();
    drawMap();
    // drawMenuScreen();

    if ((double)(clock() - time) / CLOCKS_PER_SEC > 1) {
      // update method
      moveMovable(yStart);
      yStart = 1 - yStart;
      time = clock();
    }

    readSNES();
    updatePlayer();
  }

  // Deallocate memory
  cleanUpRenderer();
  munmap(fbinfo.fbptr, fbinfo.screenSizeBytes);

  return 0;
}
