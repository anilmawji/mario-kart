#include "config.h"

#define MAP_WIDTH (VIEWPORT_WIDTH / CELL_WIDTH)
// Subtract 1 to make space for the row of gui labels on top
#define MAP_HEIGHT (VIEWPORT_HEIGHT / CELL_HEIGHT - 1)
#define MAX_OBJECTS 200

struct GameMap {
  int posX;
  int posY;

  int backgroundMap[MAP_HEIGHT][MAP_WIDTH];
  int objectMap[MAP_HEIGHT][MAP_WIDTH];

  struct GameObject* objects[MAX_OBJECTS];
  int numObjects;
};

void clearGameMap(struct GameMap* map, int maxX, int maxY, int bgColor);

void initGameMap(struct GameMap* map, int posX, int posY, int defaultColor);

void addGameObject(struct GameMap* map, struct GameObject* obj);

void removeGameObject(struct GameMap* map, struct GameObject* obj);

void updateGameMap(struct GameMap* map);

void updateGameMapObject(struct GameMap* map, struct GameObject* obj);

void drawGameMapObject(struct GameMap* map, struct GameObject* obj);

void printGameMap(struct GameMap* map);

void printObjectMap(struct GameMap* map);

int getCellColor(int cellId);

void drawGameMap(struct GameMap* map);

void drawInitialGameMap(struct GameMap* map);
