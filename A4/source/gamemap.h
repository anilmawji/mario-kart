#include "config.h"

#define MAP_WIDTH (VIEWPORT_WIDTH / CELL_WIDTH)
// Subtract 1 to make space for the row of gui labels on top
#define MAP_HEIGHT (VIEWPORT_HEIGHT / CELL_HEIGHT - 1)
#define MAX_OBJECTS 200
// Used to represent the absence of an object in the objectMap
#define NO_OBJECT -1

struct GameMap {
  int posX;
  int posY;

  struct SpriteSheet* spriteSheet;

  int backgroundMap[MAP_HEIGHT][MAP_WIDTH];
  int objectMap[MAP_HEIGHT][MAP_WIDTH];

  struct GameObject* objects[MAX_OBJECTS];
  int numObjects;
};

void initGameMap(struct GameMap* map, int posX, int posY, struct SpriteSheet* sheet);

void clearGameMap(struct GameMap* map, int maxX, int maxY, int bgColor);

void addGameObject(struct GameMap* map, struct GameObject* obj);

void removeGameObject(struct GameMap* map, struct GameObject* obj);

void setGameObjectPos(struct GameMap* map, struct GameObject* obj, int posX,
                      int posY);

void printGameMap(struct GameMap* map);

void printObjectMap(struct GameMap* map);

void drawAnimatedGameObject(struct GameMap* map, struct GameObject* obj);

void drawGameMapObject(struct GameMap* map, struct GameObject* obj);

void drawInitialGameMap(struct GameMap* map);
