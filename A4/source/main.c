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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>

#include "bowserassets.h"
#include "color.h"
#include "config.h"
#include "controller.h"
#include "gamemap.h"
#include "gameobject.h"
#include "gpio.h"
#include "marioassets.h"
#include "menu.h"
#include "renderer.h"
#include "timer.h"
#include "utils.h"

#define BUFFER_SIZE 20
#define SCORE_CONST 10
#define PLAYER_START_X 1
#define PLAYER_START_Y (MAP_HEIGHT / 2 - 1)
#define MAX_STATIC_OBSTACLES 100
#define MAX_MOVING_OBSTACLES 50

enum ObjectTypes { PLAYER, MOVING_OBSTACLE, STATIC_OBSTACLE };

struct GameState {
  struct GameMap gameMap;

  struct GameObject movingObstacles[MAX_MOVING_OBSTACLES];
  int numMoving;

  struct GameObject staticObstacles[MAX_STATIC_OBSTACLES];
  int numStatic;

  Timer timeLeft;
  int lives;
  int win;
  int lose;

  float powerUpShowTime;
  int currentLevel;

} state;

struct GameObject player;

char textBuffer[BUFFER_SIZE];

short* marioSprites[] = {
    (short*)mario_up.pixel_data, (short*)mario_down.pixel_data,
    (short*)mario_right.pixel_data, (short*)mario_left.pixel_data};

short* bowserSprite = (short*)bowser_down.pixel_data;

// Eg. A speed of 1 leads to a delay of 1/(5*1) = 1/5 = 0.2
void setPlayerSpeed(float speed) { setButtonDelay(1 / (5 * speed)); }

void addMovingObstacle(struct GameObject* obj, int x) {
  for (int y = 0; y < MAP_HEIGHT; y++) {
    state.gameMap.backgroundMap[y][x] = GREY;
  }

  initGameObject(obj, x, 0, MOVING_OBSTACLE, bowserSprite, TRANSPARENT, MV_DOWN,
                 1.5);
  addGameObject(&state.gameMap, obj);

  obj->updateInterval = 0.05 + (rand() % 3) * 0.1;
  obj->lastUpdateTime = clock();
}

void addStaticObstacle(struct GameObject* obj, int x, int y) {
  initGameObject(obj, x, y, MOVING_OBSTACLE, bowserSprite, TRANSPARENT, MV_DOWN,
                 0);
  addGameObject(&state.gameMap, obj);
}

void addFinishLine() {
  int black = TRUE;

  for (int x = MAP_WIDTH - 2; x < MAP_WIDTH; x++) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
      if (black) {
        state.gameMap.backgroundMap[y][x] = BLACK;
        black = FALSE;
      } else {
        state.gameMap.backgroundMap[y][x] = WHITE;
        black = TRUE;
      }
    }
  }
}

void generateRandomMap() {
  int numMoving = 0;
  int numStatic = 0;
  int chance;

  srand(time(0));

  for (int x = 3; x < MAP_WIDTH - 3; x++) {
    chance = rand() % 4;
    if (chance >= 1 && numMoving + 1 < MAX_MOVING_OBSTACLES) {
      addMovingObstacle(&state.movingObstacles[numMoving], x);
      numMoving++;
    } else if (numStatic + 1 < MAX_STATIC_OBSTACLES) {
      for (int y = 0; y < MAP_HEIGHT; y++) {
        chance = rand() % 4;
        if (chance == 1) {
          addStaticObstacle(&state.staticObstacles[numStatic], x, y);
          numStatic++;
        }
      }
    }
  }
  state.numMoving = numMoving;
  state.numStatic = numStatic;

  addFinishLine();
}

void respawnPlayer() {
  player.prevPosX = player.posX;
  player.prevPosY = player.posY;
  state.gameMap.objectMap[player.posY][player.posX] = -1;
  player.posX = PLAYER_START_X;
  player.posY = PLAYER_START_Y;
  state.gameMap.objectMap[player.posY][player.posX] = player.index;
  state.lives--;

  drawGameMapObject(&state.gameMap, &player);
}

int updateGameObject(struct GameObject* obj) {
  if ((clock() - obj->lastUpdateTime) / CLOCKS_PER_SEC > obj->updateInterval) {
    int newX = obj->posX;
    int newY = obj->posY;

    switch (obj->dir) {
      case MV_UP:
        newY = clampUtil(obj->posY - 1, 0, MAP_HEIGHT - 1);
        break;
      case MV_DOWN:
        newY = clampUtil(obj->posY + 1, 0, MAP_HEIGHT - 1);
        break;
      case MV_LEFT:
        newX = clampUtil(obj->posX - 1, 0, MAP_WIDTH - 1);
        break;
      case MV_RIGHT:
        newX = clampUtil(obj->posX - 1, 0, MAP_WIDTH - 1);
        break;
    }
    obj->lastUpdateTime = clock();

    if (newX != obj->posX || newY != obj->posY) {
      // Loop back to top of screen
      if (newY == MAP_HEIGHT - 1) {
        obj->updateInterval = 0.1 + (rand() % 3) * 0.1;
        obj->prevPosY = obj->posY;
        state.gameMap.objectMap[obj->prevPosY][obj->prevPosX] = -1;
        obj->posY = 0;
        state.gameMap.objectMap[obj->posY][obj->posX] = obj->index;
      } else {
        obj->prevPosX = obj->posX;
        obj->prevPosY = obj->posY;
        state.gameMap.objectMap[obj->prevPosY][obj->prevPosX] = -1;
        obj->posX = newX;
        obj->posY = newY;
        state.gameMap.objectMap[obj->posY][obj->posX] = obj->index;
      }
      return TRUE;
    }
  }
  return FALSE;
}

void updateMovingObstacles() {
  struct GameObject* obj;

  for (int i = 0; i < state.numMoving; i++) {
    obj = &state.movingObstacles[i];

    if (updateGameObject(obj)) {
      if (obj->posX == player.posX && obj->posY == player.posY) {
        respawnPlayer();
      }
      drawGameMapObject(&state.gameMap, obj);
    }
  }
}

int checkLoss() {
  return state.lives == 0 || timerSecondsLeft(state.timeLeft) <= 0;
}

int checkLevelWin() { return player.posX >= MAP_WIDTH - 1; }

int updatePlayer() {
  int newX = player.posX;
  int newY = player.posY;

  if (isButtonHeld(JOY_PAD_UP)) {
    player.dir = MV_UP;
    newY = clampUtil(player.posY - 1, 0, MAP_HEIGHT - 1);
  } else if (isButtonHeld(JOY_PAD_DOWN)) {
    player.dir = MV_DOWN;
    newY = clampUtil(player.posY + 1, 0, MAP_HEIGHT - 1);
  } else if (isButtonHeld(JOY_PAD_LEFT)) {
    player.dir = MV_LEFT;
    newX = clampUtil(player.posX - 1, 0, MAP_WIDTH - 1);
  } else if (isButtonHeld(JOY_PAD_RIGHT)) {
    player.dir = MV_RIGHT;
    newX = clampUtil(player.posX + 1, 0, MAP_WIDTH - 1);
  }

  if (newX != player.posX || newY != player.posY) {
    if (state.gameMap.objectMap[newY][newX] != -1) {
      respawnPlayer();
    } else {
      player.sprite = marioSprites[player.dir];
      player.prevPosX = player.posX;
      player.prevPosY = player.posY;
      state.gameMap.objectMap[player.prevPosY][player.prevPosX] = -1;
      player.posX = newX;
      player.posY = newY;
      state.gameMap.objectMap[player.posY][player.posX] = player.index;

      drawGameMapObject(&state.gameMap, &player);
    }
    return TRUE;
  }
  return FALSE;
}

int calculateScore() {
  return (timerSecondsLeft(state.timeLeft) + state.lives) * SCORE_CONST;
}

void drawGuiValues(int score) {
  sprintf(textBuffer, "%04d", score);
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

void drawGameFinishedScreen(char* text, int length, int finalScore) {
  drawFillRect(viewportX + (VIEWPORT_WIDTH - 20 * CELL_WIDTH) / 2,
               viewportY + (VIEWPORT_HEIGHT - 8 * CELL_HEIGHT) / 2,
               20 * CELL_WIDTH, 8 * CELL_HEIGHT, BLACK);

  drawText(text, length, viewportX + (VIEWPORT_WIDTH - length * CELL_WIDTH) / 2,
           viewportY + (VIEWPORT_HEIGHT - 4 * CELL_HEIGHT) / 2, BLACK);

  sprintf(textBuffer, "final score %04d", finalScore);
  drawText(textBuffer, 16, viewportX + (VIEWPORT_WIDTH - 16 * CELL_WIDTH) / 2,
           viewportY + (VIEWPORT_HEIGHT + CELL_HEIGHT) / 2, BLACK);
}

void runGame() {
  clearScreen();
  drawGuiLabels();
  drawInitialGameMap(&state.gameMap);
  startTimer(state.timeLeft);

  int score;

  while (!isButtonPressed(START) && !state.win && !state.lose) {
    // printGameMap(&state.gameMap);
    readSNES();
    updatePlayer();
    updateMovingObstacles();
    score = calculateScore();
    drawGuiValues(score);

    if (checkLevelWin()) {
      if (state.currentLevel == 4) {
        state.win = TRUE;
      } else {
        state.currentLevel++;

        clearGameMap(&state.gameMap, MAP_WIDTH, MAP_HEIGHT, GREEN);
        generateRandomMap(&state.gameMap);
        drawInitialGameMap(&state.gameMap);

        setGameObjectPos(&player, PLAYER_START_X, PLAYER_START_Y);
        drawGameMapObject(&state.gameMap, &player);
      }
    } else if (checkLoss()) {
      state.lose = TRUE;
    }
  }
  if (state.win) {
    drawGameFinishedScreen("you win", 7, score);
  } else if (state.lose) {
    drawGameFinishedScreen("game over", 9, score);
  }
}

void viewMenu() {
  clearScreen();
  drawInitialMenuScreen();

  while (!isButtonPressed(A)) {
    readSNES();
    updateMenuScreen();
  }
  // User pressed A
  if (menuSelection == START_BTN) {
    runGame();
  } else if (menuSelection == QUIT_BTN) {
    clearScreen();
    exit(0);
  }
}

void initGame() {
  // Fill game map with grass tiles by default
  initGameMap(&state.gameMap, viewportX, viewportY + CELL_HEIGHT, GREEN);

  initGameObject(&player, PLAYER_START_X, PLAYER_START_Y, PLAYER,
                 marioSprites[MV_RIGHT], TRANSPARENT, MV_RIGHT, 1.5);
  addGameObject(&state.gameMap, &player);

  generateRandomMap();

  state.timeLeft.secondsAllowed = 3 * 60;  // seconds
  state.lives = 4;
  state.currentLevel = 1;
}

int main(int argc, char* argv[]) {
  initRenderer(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
  initGPIO();
  initSNES();
  initMenuScreen(viewportX, viewportY, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
  initGame();

  viewMenu();

  // Deallocate memory
  cleanUpRenderer();

  return 0;
}
