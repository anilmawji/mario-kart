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

#include "color.h"
#include "config.h"
#include "controller.h"
#include "gameassets.h"
#include "gamemap.h"
#include "gameobject.h"
#include "gpio.h"
#include "menu.h"
#include "menuassets.h"
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
#define MAX_POWERUPS 8

#define POWERUP_SPEED (PLAYER_DEFAULT_SPEED * 2)
#define POWERUP_TIME_ADDED 30
#define POWERUP_SPAWN_TIME 10

// Tile ids correspond to their location in the spritesheet
// For a tile with an id of k, the tile must be the kth tile in the spritesheet
enum TILE_IDS {
  GRASS = 22,
  ROAD = 23,
  FINISH_LINE = 34,
  PLANT_BIG = 31,
  PLANT_SMALL = 32
};

// Values must be at least as large as the number of tiles in the spritesheet
// to avoid coflicting with background tile ids
// Beyond that, the values are arbitrary
enum OBJECT_IDS {
  PLAYER = 111,
  MOVING_OBSTACLE = 222,
  STATIC_OBSTACLE = 333,
  POWERUP = 444
};

struct GameState {
  struct GameMap gameMap;

  struct GameObject movingObstacles[MAX_MOVING_OBSTACLES];
  int numMoving;

  struct GameObject staticObstacles[MAX_STATIC_OBSTACLES];
  int numStatic;

  struct GameObject powerups[MAX_POWERUPS];
  int numPowerups;

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
short* gameSprites = (short*)game_sprites.pixel_data;
short* gameSpritesGreyscale;

struct SpriteSheet gameSpriteSheet;

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
  // Draw road
  for (int y = 0; y < MAP_HEIGHT; y++) {
    state.gameMap.backgroundMap[y][x] = ROAD;
  }

  //Select random tile row to get a random kart sprite
  int character = 1 + rand() % (gameSpriteSheet.rows - 1);

  // Pick random move direction (up or down)
  if (rand() % 2 == 0) {
    initGameObject(obj, x, 0, MOVING_OBSTACLE, &gameSpriteSheet, 0, character,
                   MV_DOWN, 2);
  } else {
    initGameObject(obj, x, MAP_HEIGHT - 1, MOVING_OBSTACLE, &gameSpriteSheet, 0,
                   character, MV_UP, 2);
  }
  addGameObject(&state.gameMap, obj);

  obj->updateInterval = 0.05 + (rand() % 3) * 0.1;
  obj->lastUpdateTime = clock();
}

void generateInitialGameMap() {
  clearGameMap(&state.gameMap, MAP_WIDTH, MAP_HEIGHT, GRASS);

  int bgType;

  // Add some plants to the map to make it pretty
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      bgType = rand() % 12;

      if (bgType <= 2) {
        state.gameMap.backgroundMap[y][x] = PLANT_BIG;
      } else if (bgType == 3) {
        state.gameMap.backgroundMap[y][x] = PLANT_SMALL;
      }
    }
  }
}

void generateRandomMap() {
  int numMoving = 0;
  int numStatic = 0;
  int numPowerups = 0;
  int objType;
  int prevX;
  struct GameObject* obj;

  srand(time(0));

  for (int x = 3; x < MAP_WIDTH - 3; x++) {
    objType = rand() % 5;
    if (objType >= 2 && numMoving + 1 < MAX_MOVING_OBSTACLES) {
      addMovingObstacle(&state.movingObstacles[numMoving], x);
      numMoving++;

    } else if (objType >= 1 && numStatic + 1 < MAX_STATIC_OBSTACLES) {
      for (int y = 0; y < MAP_HEIGHT; y++) {
        objType = rand() % 10;
        if (objType <= 3) {
          obj = &state.staticObstacles[numStatic];

          initGameObject(obj, x, y, STATIC_OBSTACLE, &gameSpriteSheet, 5, 4,
                         MV_UP, 2);
          addGameObject(&state.gameMap, obj);

          obj->updateInterval = 0.4 + (rand() % 4) * 0.1;

          numStatic++;
        } else if (objType <= 4 && prevX != x && x >= 3 &&
                   numPowerups < MAX_POWERUPS) {
          obj = &state.powerups[numPowerups];

          initGameObject(obj, x, y, POWERUP, &gameSpriteSheet, 6, 3, MV_UP, 2);

          numPowerups++;
          prevX = x;
        }
      }
    }
  }
  state.numMoving = numMoving;
  state.numStatic = numStatic;
  state.numPowerups = numPowerups;

  // Add finish line
  for (int x = MAP_WIDTH - 2; x < MAP_WIDTH - 1; x++) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
      state.gameMap.backgroundMap[y][x] = FINISH_LINE;
    }
  }
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
        newY = obj->posY - 1;
        break;
      case MV_DOWN:
        newY = obj->posY + 1;
        break;
      case MV_LEFT:
        newX = obj->posX - 1;
        break;
      case MV_RIGHT:
        newX = obj->posX - 1;
        break;
    }
    obj->lastUpdateTime = clock();

    // Only update object position if it changed
    if (newX != obj->posX || newY != obj->posY) {
      if (obj->dir == MV_DOWN && newY > MAP_HEIGHT - 1) {
        // Random chance of switching character
        obj->spriteTileY = 1 + rand() % (gameSpriteSheet.rows - 1);
        // Pick random moving object speed
        obj->updateInterval = 0.1 + (rand() % 3) * 0.1;
        if (rand() % 2 == 0) {
          // Loop back to top of screen
          setGameObjectPos(&state.gameMap, obj, obj->posX, 0);
        } else {
          // Random chance of switching direction
          obj->dir = MV_UP;
          setGameObjectPos(&state.gameMap, obj, obj->posX, MAP_HEIGHT - 1);
        }
      } else if (obj->dir == MV_UP && newY < 0) {
        // Random chance of switching character
        obj->spriteTileY = 1 + rand() % (gameSpriteSheet.rows - 1);
        // Pick random moving object speed
        obj->updateInterval = 0.1 + (rand() % 3) * 0.1;
        if (rand() % 2 == 0) {
          // Loop back to bottom of screen
          setGameObjectPos(&state.gameMap, obj, obj->posX, MAP_HEIGHT - 1);
        } else {
          // Random chance of switching direction
          obj->dir = MV_DOWN;
          setGameObjectPos(&state.gameMap, obj, obj->posX, 0);
        }
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

/*
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
*/

void activatePowerup() {
  // Pick random powerup
  int type = rand() % 10;

  if (type <= 3) {
    state.lives++;
  } else if (type <= 6) {
    state.timeLeft.secondsAllowed += POWERUP_TIME_ADDED;
  } else {
    setPlayerSpeed(POWERUP_SPEED);
    state.powerupStartTime = clock();
    state.powerupInEffect = TRUE;
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

  // Only update player position if it changed
  if (newX != player.posX || newY != player.posY) {
    int objId = state.gameMap.objectMap[newY][newX];

    if (objId == MOVING_OBSTACLE || objId == STATIC_OBSTACLE) {
      // Player collided with an obstacle
      respawnPlayer();
    } else {
      if (objId == POWERUP) {
        // Remove value pack from map
        // Memory is cleared out from gameMap->objects when the level ends
        state.gameMap.objectMap[newY][newX] = NO_OBJECT;
        activatePowerup();
      }
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

  generateInitialGameMap();

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

void useGreyscaleFilter() {
  state.gameMap.spriteSheet->pixelData = gameSpritesGreyscale;

  for (int i = 0; i < state.gameMap.numObjects; i++) {
    state.gameMap.objects[i]->spriteSheet->pixelData = gameSpritesGreyscale;
  }
}

void removeFilter() {
  state.gameMap.spriteSheet->pixelData = gameSprites;

  for (int i = 0; i < state.gameMap.numObjects; i++) {
    state.gameMap.objects[i]->spriteSheet->pixelData = gameSprites;
  }
}

void endGame(int finalScore) {
  if (state.win) {
    drawGameFinishedScreen("you win", 7, finalScore);
  } else if (state.lose) {
    useGreyscaleFilter();
    // Redraw make to reflect filter changes
    drawInitialGameMap(&state.gameMap);
    drawGameMapObject(&state.gameMap, &player);

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
  removeFilter();
  drawInitialGameMap(&state.gameMap);
  drawGameMapObject(&state.gameMap, &player);

  int score;

  while (!state.win && !state.lose) {
    // double start = clock(); //rough performance test
    //   printGameMap(&state.gameMap);
    readSNES();
    updatePlayer();
    updateMovingObstacles();
    // updateStaticObstacles();
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
        (double)(clock() - state.levelStartTime) / CLOCKS_PER_SEC >=
            POWERUP_SPAWN_TIME) {
      for (int i = 0; i < state.numPowerups; i++) {
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

        generateInitialGameMap();
        generateRandomMap();
        drawInitialGameMap(&state.gameMap);

        setGameObjectPos(&state.gameMap, &player, PLAYER_START_X,
                         PLAYER_START_Y);
        drawGameMapObject(&state.gameMap, &player);

        state.powerupsAdded = FALSE;
        state.levelStartTime = clock();
      }
    } else if (checkLoss()) {
      state.lose = TRUE;
    }
    // double dur = clock() - start;
    // printf("%.4f\n", dur / CLOCKS_PER_SEC);
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
  setGameObjectPos(&state.gameMap, &player, PLAYER_START_X, PLAYER_START_Y);
  startTimer(&state.timeLeft);
  state.levelStartTime = clock();
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
  drawInitialMenu(&mainMenu, TRUE);

  while (!isButtonPressed(A)) {
    readSNES();
    updateMenuButtonSelection(&mainMenu);
    drawMenu(&mainMenu);
  }
  runMenuButtonEvent(&mainMenu, mainMenu.selectedButton);
}

void viewGameMenu() {
  useGreyscaleFilter();
  drawInitialGameMap(&state.gameMap);
  drawGameMapObject(&state.gameMap, &player);

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
  initGameMap(&state.gameMap, viewportX, viewportY + CELL_HEIGHT,
              &gameSpriteSheet);

  // Init main menu
  initMenu(&mainMenu);
  addMenuButton(&mainMenu, "start", &startGame);
  addMenuButton(&mainMenu, "quit", &quitGame);
  mainMenu.posY = viewportY + 350;

  // Init game menu
  initMenu(&gameMenu);
  addMenuButton(&gameMenu, "restart", &startGame);
  addMenuButton(&gameMenu, "quit", &viewMainMenu);

  // Init game sprites
  initSpriteSheet(&gameSpriteSheet, gameSprites, game_sprites.width,
                  game_sprites.height, 7, 9, 32, 32, 4, 4, SPRITE_BG_COLOR);

  // Init player
  initGameObject(&player, PLAYER_START_X, PLAYER_START_Y, PLAYER,
                 &gameSpriteSheet, 0, 0, MV_RIGHT, 2);
  addGameObject(&state.gameMap, &player);

  // Init greyscale filter
  gameSpritesGreyscale = generateGreyscaleImage(
      gameSprites, game_sprites.width, game_sprites.height, SPRITE_BG_COLOR);

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
