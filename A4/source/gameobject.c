#include "gameobject.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "gamemap.h"
#include "renderer.h"
#include "utils.h"

#define TRUE 1
#define FALSE 0

void initGameObject(struct GameObject* obj, int posX, int posY, int id, struct SpriteSheet* sheet, int spriteTileX, int spriteTileY, Direction dir, int speed) {
  obj->width = CELL_WIDTH;
  obj->height = CELL_HEIGHT;
  obj->posX = posX;
  obj->posY = posY;
  obj->spriteSheet = sheet;
  obj->spriteTileX = spriteTileX;
  obj->spriteTileY = spriteTileY;
  obj->id = id;
  obj->dir = dir;
  obj->speed = speed;
  obj->updateInterval = 0.2;
}
