#include "gamemap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "color.h"
#include "config.h"
#include "gameobject.h"
#include "renderer.h"

#define TRUE 1
#define FALSE 0

void initGameMap(struct GameMap* map, int posX, int posY,
                 struct SpriteSheet* sheet) {
  map->posX = posX;
  map->posY = posY;
  map->spriteSheet = sheet;
  map->numObjects = 0;
}

void clearGameMap(struct GameMap* map, int maxX, int maxY, int bgColor) {
  for (int y = 0; y < maxY; y++) {
    for (int x = 0; x < maxX; x++) {
      map->backgroundMap[y][x] = bgColor;
      map->objectMap[y][x] = NO_OBJECT;
    }
  }

  memset(&map->objects, 0, sizeof(map->objects));
  map->numObjects = 0;
}

void addGameObject(struct GameMap* map, struct GameObject* obj) {
  // Useful for detecting potential memory leaks
  // All objects should be deleted and generated from scratch when a new level
  // is reached
  if (map->numObjects + 1 > MAX_OBJECTS) {
    fprintf(stderr, "Failed to add game object; maximum reached\n");
    return;
  }
  obj->index = map->numObjects;
  map->objects[obj->index] = obj;
  map->objectMap[obj->posY][obj->posX] = obj->id;
  map->numObjects++;
}

void removeGameObject(struct GameMap* map, struct GameObject* obj) {
  map->objectMap[obj->posY][obj->posX] = NO_OBJECT;
  map->objects[obj->index] = NULL;
}

void drawBackgroundTile(struct GameMap* map, int posX, int posY) {
  int cellX = map->posX + posX * CELL_WIDTH;
  int cellY = map->posY + posY * CELL_HEIGHT;

  int bgTileId = map->backgroundMap[posY][posX];
  int bgTileX = getSpriteTileX(map->spriteSheet, bgTileId);
  int bgTileY = getSpriteTileY(map->spriteSheet, bgTileId);

  drawSpriteTile(map->spriteSheet, cellX, cellY, bgTileX, bgTileY, TRANSPARENT);
}

void drawGameMapObject(struct GameMap* map, struct GameObject* obj) {
  int cellX = map->posX + obj->posX * CELL_WIDTH;
  int cellY = map->posY + obj->posY * CELL_HEIGHT;

  int bgTileId = map->backgroundMap[obj->posY][obj->posX];
  int bgTileX = getSpriteTileX(map->spriteSheet, bgTileId);
  int bgTileY = getSpriteTileY(map->spriteSheet, bgTileId);

  drawOverlayedSpriteTiles(obj->spriteSheet, cellX, cellY,
                                  obj->spriteTileX + obj->dir, obj->spriteTileY,
                                  map->spriteSheet, bgTileX, bgTileY);
}

// Updates the location of the object in the object map
void setGameObjectPos(struct GameMap* map, struct GameObject* obj, int posX,
                      int posY) {
  drawBackgroundTile(map, obj->posX, obj->posY);

  map->objectMap[obj->posY][obj->posX] = NO_OBJECT;
  obj->posX = posX;
  obj->posY = posY;
  map->objectMap[obj->posY][obj->posX] = obj->id;
}

void printGameMap(struct GameMap* map) {
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      if (map->objectMap[y][x] == NO_OBJECT) {
        switch (map->backgroundMap[y][x]) {
          case GREEN:
            printf("g ");
            break;
          case GREY:
            printf("r ");
            break;
        }
      } else {
        printf("O ");
      }
    }
    printf("\n");
  }
  printf("\n\n");
}

void printObjectMap(struct GameMap* map) {
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      if (map->objectMap[y][x] == NO_OBJECT) {
        printf("_ ");
      } else {
        printf("O ");
      }
    }
    printf("\n");
  }
  printf("\n\n");
}

/*
void drawAnimatedGameObject(struct GameMap* map, struct GameObject* obj) {
  int cellX = map->posX + obj->posX * CELL_WIDTH;
  int cellY = map->posY + obj->posY * CELL_HEIGHT;

  //Increment the animation frame
  obj->animationFrame++;
  if (obj->animationFrame > obj->spriteSheet->cols) {
    obj->animationFrame = 0;
  }

  drawSpriteTile(obj->spriteSheet, cellX, cellY, obj->animationFrame, 0,
                 map->backgroundMap[obj->posY][obj->posX]);
}
*/

void drawInitialGameMap(struct GameMap* map) {
  int maxTileId = getNumTiles(map->spriteSheet);

  // Draw background tiles
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      if (map->objectMap[y][x] <= maxTileId) {
        drawBackgroundTile(map, x, y);
      }
    }
  }

  // Draw object tiles
  for (int i = 0; i < map->numObjects; i++) {
    drawGameMapObject(map, map->objects[i]);
  }
}
