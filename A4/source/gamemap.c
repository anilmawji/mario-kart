#include "gamemap.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "color.h"
#include "config.h"
#include "gameobject.h"
#include "renderer.h"

#define TRUE 1
#define FALSE 0

void clearGameMap(struct GameMap* map, int maxX, int maxY, int bgColor) {
  for (int y = 0; y < maxY; y++) {
    for (int x = 0; x < maxX; x++) {
      map->backgroundMap[y][x] = bgColor;
      map->objectMap[y][x] = -1;
    }
  }
  for (int i = 1; i < map->numObjects; i++) {
    map->objects[i] = NULL;
  }
  map->numObjects = 0;
}

void initGameMap(struct GameMap* map, int posX, int posY, int defaultColor) {
  map->posX = posX;
  map->posY = posY;
  map->numObjects = 0;
}

void addGameObject(struct GameMap* map, struct GameObject* obj) {
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
  map->objectMap[obj->posY][obj->posX] = -1;
  map->objects[obj->index] = NULL;
}

void printGameMap(struct GameMap* map) {
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      if (map->objectMap[y][x] == -1) {
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
      if (map->objectMap[y][x] == -1) {
        printf("_ ");
      } else {
        printf("O ");
      }
    }
    printf("\n");
  }
  printf("\n\n");
}

// TODO: Split into 2 functions, erase and draw
void drawGameMapObject(struct GameMap* map, struct GameObject* obj) {
  int cellX = map->posX + obj->prevPosX * CELL_WIDTH;
  int cellY = map->posY + obj->prevPosY * CELL_HEIGHT;

  // Erase the object from the old position
  drawFillRect(cellX, cellY, CELL_WIDTH, CELL_HEIGHT,
               map->backgroundMap[obj->prevPosY][obj->prevPosX]);

  cellX = map->posX + obj->posX * CELL_WIDTH;
  cellY = map->posY + obj->posY * CELL_HEIGHT;

  // Draw the object in the new position
  drawImage(obj->sprite, cellX, cellY, CELL_WIDTH, CELL_HEIGHT,
            obj->spriteBgColor, map->backgroundMap[obj->posY][obj->posX]);
}

void drawAnimatedGameObject(struct GameMap* map, struct GameObject* obj) {
  int cellX = map->posX + obj->posX * CELL_WIDTH;
  int cellY = map->posY + obj->posY * CELL_HEIGHT;

  obj->animationFrame++;
  if (obj->animationFrame > obj->spriteSheet->cols) {
    obj->animationFrame = 0;
  }

  drawSpriteTile(obj->spriteSheet, cellX, cellY, obj->animationFrame, 0,
                 map->backgroundMap[obj->posY][obj->posX]);
}

void drawInitialGameMap(struct GameMap* map) {
  int cellX;
  int cellY;

  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      if (map->objectMap[y][x] == -1) {
        cellX = map->posX + x * CELL_WIDTH;
        cellY = map->posY + y * CELL_HEIGHT;

        drawFillRect(cellX, cellY, CELL_WIDTH, CELL_HEIGHT,
                     map->backgroundMap[y][x]);
      }
    }
  }

  struct GameObject* obj;

  for (int i = 0; i < map->numObjects; i++) {
    obj = map->objects[i];

    if (obj->spriteSheet == NULL) {
      cellX = map->posX + obj->posX * CELL_WIDTH;
      cellY = map->posY + obj->posY * CELL_HEIGHT;

      drawImage(obj->sprite, cellX, cellY, CELL_WIDTH, CELL_HEIGHT,
                obj->spriteBgColor, map->backgroundMap[obj->posY][obj->posX]);
    } else {
      drawAnimatedGameObject(map, obj);
    }
  }
}
