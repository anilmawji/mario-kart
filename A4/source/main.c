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
#include "menuassets.h"
#include "objectassets.h"
#include "plants.h"
#include "renderer.h"
#include "timer.h"
#include "utils.h"

#define BUFFER_SIZE 20
#define SCORE_CONST 10
#define PLAYER_DEFAULT_SPEED 1.2
#define PLAYER_START_X 1
#define PLAYER_START_Y (MAP_HEIGHT / 2 - 1)
#define MAX_STATIC_OBSTACLES 100
#define MAX_MOVING_OBSTACLES 50
#define MAX_POWERUPS 10

enum ObjectTypes { PLAYER, MOVING_OBSTACLE, STATIC_OBSTACLE, POWERUP };

struct GameState {
  struct GameMap gameMap;

  struct GameObject movingObstacles[MAX_MOVING_OBSTACLES];
  int numMoving;

  struct GameObject staticObstacles[MAX_STATIC_OBSTACLES];
  int numStatic;

  struct GameObject powerups[MAX_POWERUPS];
  int numValue;

  Timer timeLeft;
  int lives;
  int win;
  int lose;
  int currentLevel;
  float levelStartTime;

  int powerupsAdded;
  float powerupStartTime;
  int powerupInEffect;

} state;

struct Menu mainMenu, gameMenu;
struct GameObject player;

char textBuffer[BUFFER_SIZE];

short* menuBackground = (short*)menu_background.pixel_data;
short* menuTitle = (short*)menu_title.pixel_data;

short* marioSprites[] = {
    (short*)mario_up.pixel_data, (short*)mario_down.pixel_data,
    (short*)mario_right.pixel_data, (short*)mario_left.pixel_data};

short* bowserSprite = (short*)bowser_down.pixel_data;
short* powerupSprite = (short*)powerup.pixel_data;

struct SpriteSheet plantSprites;

void runGameLoop();
void resetGameState();
void viewMainMenu();
void viewGameMenu();

// Eg. A speed of 1 leads to a delay of 1/(5*1) = 1/5 = 0.2
void setPlayerSpeed(float speed) { setButtonDelay(1 / (5 * speed)); }

int calculateScore() {
  return (timerSecondsLeft(&state.timeLeft) + state.lives) * SCORE_CONST;
}

int checkLoss() {
  return state.lives == 0 || timerSecondsLeft(&state.timeLeft) <= 0;
}

int checkLevelWin() { return player.posX >= MAP_WIDTH - 1; }

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
  int numValue = 0;
  int chance;
  int prevX;
  struct GameObject* obj;

  srand(time(0));

  for (int x = 3; x < MAP_WIDTH - 3; x++) {
    chance = rand() % 5;
    if (chance >= 2 && numMoving + 1 < MAX_MOVING_OBSTACLES) {
      addMovingObstacle(&state.movingObstacles[numMoving], x);
      numMoving++;
    } else if (chance >= 1 && numStatic + 1 < MAX_STATIC_OBSTACLES) {
      for (int y = 0; y < MAP_HEIGHT; y++) {
        chance = rand() % 10;
        if (chance <= 3) {
          obj = &state.staticObstacles[numStatic];
          initGameObject(obj, x, y, STATIC_OBSTACLE, (short*)plant.pixel_data,
                         WHITE, MV_DOWN, 0);
          addGameObject(&state.gameMap, obj);
          obj->spriteSheet = &plantSprites;
          obj->updateInterval = 0.4 + (rand() % 4) * 0.1;
          numStatic++;
        } else if (chance <= 4 && prevX != x && x >= 3 &&
                   numValue < MAX_POWERUPS) {
          initGameObject(&state.powerups[numValue], x, y, POWERUP,
                         powerupSprite, WHITE, MV_DOWN, 0);
          numValue++;
          prevX = x;
        }
      }
    }
  }
  state.numMoving = numMoving;
  state.numStatic = numStatic;
  state.numValue = numValue;

  addFinishLine();
}

void respawnPlayer() {
  setGameObjectPos(&state.gameMap, &player, PLAYER_START_X, PLAYER_START_Y);
  state.lives--;
  drawGameMapObject(&state.gameMap, &player);
}

int updateGameObject(struct GameObject* obj) {
  // Using the current level as a factor for update speed is what controls the
  // difficulty as you progress
  if ((clock() - obj->lastUpdateTime) / CLOCKS_PER_SEC >
      obj->updateInterval * (2 / (float)state.currentLevel)) {
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
        setGameObjectPos(&state.gameMap, obj, obj->posX, 0);
      } else {
        setGameObjectPos(&state.gameMap, obj, newX, newY);
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

void updateStaticObstacles() {
  struct GameObject* obj;

  for (int i = 0; i < state.numStatic; i++) {
    obj = &state.staticObstacles[i];

    if ((clock() - obj->lastUpdateTime) / CLOCKS_PER_SEC >
        obj->updateInterval) {
      drawAnimatedGameObject(&state.gameMap, obj);
      obj->lastUpdateTime = clock();
    }
  }
}

void updatePlayer() {
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
    int objId = state.gameMap.objectMap[newY][newX];

    if (objId == MOVING_OBSTACLE || objId == STATIC_OBSTACLE) {
      respawnPlayer();
    } else {
      if (objId == POWERUP) {
        // Remove value pack from map
        // Memory is cleared out from gameMap->objects when the level ends
        state.gameMap.objectMap[newY][newX] = -1;

        // Pick random effect
        int effect = rand() % 10;
        if (effect <= -1) {
          state.lives++;
        } else if (effect <= -1) {
          state.timeLeft.secondsAllowed += 300;
        } else {
          setPlayerSpeed(PLAYER_DEFAULT_SPEED * 2.5);
          state.powerupStartTime = clock();
          state.powerupInEffect = TRUE;
        }
      }
      player.sprite = marioSprites[player.dir];
      setGameObjectPos(&state.gameMap, &player, newX, newY);
      drawGameMapObject(&state.gameMap, &player);
    }
  }
}

void drawGuiValues(int score) {
  sprintf(textBuffer, "%04d", score);
  drawText(textBuffer, 4, viewportX + 6 * CELL_WIDTH, viewportY, 0);

  sprintf(textBuffer, "%02d", state.lives);
  drawText(textBuffer, 2, viewportX + (12 + 3.5 + 6) * CELL_WIDTH, viewportY,
           0);

  formatTimeLeft(&state.timeLeft, textBuffer);
  drawText(textBuffer, 5, (MAP_WIDTH + 5) * CELL_WIDTH, viewportY, 0);
}

void drawGuiLabels() {
  // Clear label background
  drawFillRect(viewportX, viewportY, VIEWPORT_WIDTH,
               CELL_HEIGHT + VIEWPORT_HEIGHT % CELL_HEIGHT, BLACK);

  drawText("score ", 6, viewportX, viewportY, 0);
  drawText("lives ", 6, viewportX + (12 + 3.5) * CELL_WIDTH, viewportY, 0);
  drawText("time ", 5, MAP_WIDTH * CELL_WIDTH, viewportY, 0);
}

void resetGameState() {
  // Remove all obstacles and powerups
  memset(&state.movingObstacles, 0, sizeof(state.movingObstacles));
  memset(&state.staticObstacles, 0, sizeof(state.staticObstacles));
  memset(&state.powerups, 0, sizeof(state.powerups));

  clearGameMap(&state.gameMap, MAP_WIDTH, MAP_HEIGHT, GREEN);

  // Set fields to defaults
  state.lives = 4;
  state.win = FALSE;
  state.lose = FALSE;
  state.currentLevel = 1;
  state.powerupInEffect = FALSE;
  state.powerupsAdded = FALSE;
}

void drawGameFinishedScreen(char* text, int length, int finalScore) {
  drawFillRectWithStroke(viewportX + (VIEWPORT_WIDTH - 20 * CELL_WIDTH) / 2,
                         viewportY + (VIEWPORT_HEIGHT - 8 * CELL_HEIGHT) / 2,
                         20 * CELL_WIDTH, 8 * CELL_HEIGHT, BLACK, 8, BLUE);

  drawText(text, length, viewportX + (VIEWPORT_WIDTH - length * CELL_WIDTH) / 2,
           viewportY + (VIEWPORT_HEIGHT - 4 * CELL_HEIGHT) / 2, BLACK);

  sprintf(textBuffer, "final score %04d", finalScore);
  drawText(textBuffer, 16, viewportX + (VIEWPORT_WIDTH - 16 * CELL_WIDTH) / 2,
           viewportY + (VIEWPORT_HEIGHT + CELL_HEIGHT) / 2, BLACK);
}

void endGame(int finalScore) {
  if (state.win) {
    drawGameFinishedScreen("you win", 7, finalScore);
  } else if (state.lose) {
    drawGameFinishedScreen("game over", 9, finalScore);
  }
  // Give player enough time to see results before registering button presses
  sleep(2);
  while (!isAnyButtonPressed()) {
    readSNES();
  }
  viewMainMenu();
}

void runGameLoop() {
  drawInitialGameMap(&state.gameMap);
  drawGameMapObject(&state.gameMap, &player);

  int score;

  while (!state.win && !state.lose) {
    // printGameMap(&state.gameMap);
    readSNES();
    updatePlayer();
    updateMovingObstacles();
    updateStaticObstacles();
    score = calculateScore();
    drawGuiValues(score);

    if (isButtonPressed(START)) {
      // Pause game before viewing game menu
      pauseTimer(&state.timeLeft);
      viewGameMenu();
      // Game menu has been closed, resume game
      resumeTimer(&state.timeLeft);
      runGameLoop();
    }

    // Add value packs to the map 10 seconds after the game starts
    if (!state.powerupsAdded &&
        (double)(clock() - state.levelStartTime) / CLOCKS_PER_SEC >= 10) {
      for (int i = 0; i < state.numValue; i++) {
        addGameObject(&state.gameMap, &state.powerups[i]);
        drawGameMapObject(&state.gameMap, &state.powerups[i]);
      }
      state.powerupsAdded = TRUE;
    }

    if (state.powerupInEffect &&
        (double)(clock() - state.powerupStartTime) / CLOCKS_PER_SEC >= 6) {
      setPlayerSpeed(PLAYER_DEFAULT_SPEED);
      state.powerupInEffect = FALSE;
    }

    if (checkLevelWin()) {
      if (state.currentLevel == 4) {
        state.win = TRUE;
      } else {
        state.currentLevel++;

        clearGameMap(&state.gameMap, MAP_WIDTH, MAP_HEIGHT, GREEN);
        generateRandomMap();
        drawInitialGameMap(&state.gameMap);

        setGameObjectPos(&state.gameMap, &player, PLAYER_START_X, PLAYER_START_Y);
        drawGameMapObject(&state.gameMap, &player);

        state.powerupsAdded = FALSE;
        state.levelStartTime = clock();
      }
    } else if (checkLoss()) {
      state.lose = TRUE;
    }
  }
  endGame(score);
}

void quitGame() {
  clearScreen();
  exit(0);
}

void startGame() {
  resetGameState();
  generateRandomMap();

  drawGuiLabels();
  startTimer(&state.timeLeft);
  state.levelStartTime = clock();
  setGameObjectPos(&state.gameMap, &player, PLAYER_START_X, PLAYER_START_Y);
  runGameLoop();
}

void viewMainMenu() {
  clearScreen();
  // Draw background
  drawFillRect(viewportX, viewportY, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, GREY);
  // Draw title
  drawImage(menuTitle, viewportX + (VIEWPORT_WIDTH - menu_title.width) / 2,
            viewportY + 100, menu_title.width, menu_title.height, BLACK, GREEN);
  // Draw buttons
  mainMenu.selectedButton = 0;
  drawInitialMenu(&mainMenu, TRUE);

  while (!isButtonPressed(A)) {
    readSNES();
    updateMenuButtonSelection(&mainMenu);
    drawMenu(&mainMenu);
  }
  runMenuButtonEvent(&mainMenu, mainMenu.selectedButton);
}

void viewGameMenu() {
  // Draw buttons
  gameMenu.selectedButton = 0;
  drawInitialMenu(&gameMenu, TRUE);

  while (!isButtonPressed(A)) {
    readSNES();
    updateMenuButtonSelection(&gameMenu);
    drawMenu(&gameMenu);

    if (isButtonPressed(START)) {
      // Exit game menu
      return;
    }
  }
  runMenuButtonEvent(&gameMenu, gameMenu.selectedButton);
}

void initGame() {
  initGameMap(&state.gameMap, viewportX, viewportY + CELL_HEIGHT);

  // Init main menu buttons
  initMenu(&mainMenu);
  addMenuButton(&mainMenu, "start", &startGame);
  addMenuButton(&mainMenu, "quit", &quitGame);
  mainMenu.posY = viewportY + 350;

  // Init game menu buttons
  initMenu(&gameMenu);
  addMenuButton(&gameMenu, "restart", &startGame);
  addMenuButton(&gameMenu, "quit", &viewMainMenu);

  // Init player
  initGameObject(&player, PLAYER_START_X, PLAYER_START_Y, PLAYER,
                 marioSprites[MV_RIGHT], TRANSPARENT, MV_RIGHT, 1.5);
  addGameObject(&state.gameMap, &player);

  initSpriteSheet(&plantSprites, (short*)plant.pixel_data, plant.width,
                  plant.height, 1, 2, 32, 32, 6, 5, WHITE);

  // If the size of the screen isn't perfectly divisible by the cell height,
  // move the map down by the extra space
  state.gameMap.posY += VIEWPORT_HEIGHT % CELL_HEIGHT;
  state.timeLeft.secondsAllowed = 3 * 60;  // seconds
}

int main(int argc, char* argv[]) {
  // Initialize all game features
  // These functions are only called once
  initRenderer(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
  initGPIO();
  initSNES();
  initGame();

  viewMainMenu();

  // Deallocate memory
  cleanUpRenderer();

  return 0;
}
