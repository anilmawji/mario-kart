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

void initGameObject(struct GameObject* obj, int posX, int posY, int id,
                    short* sprite, int spriteBgColor, Direction dir,
                    int speed) {
  obj->width = CELL_WIDTH;
  obj->height = CELL_HEIGHT;
  obj->posX = posX;
  obj->posY = posY;
  obj->prevPosX = 0;
  obj->prevPosY = 0;
  obj->id = id;
  obj->sprite = sprite;
  obj->spriteBgColor = spriteBgColor;
  obj->dir = dir;
  obj->speed = speed;
  obj->spriteSheet = NULL;
  obj->animationFrame = 0;
  obj->updateInterval = 0.2;
}

void setGameObjectPos(struct GameObject* obj, int posX, int posY) {
  obj->prevPosX = obj->posX;
  obj->prevPosY = obj->posY;
  obj->posX = posX;
  obj->posY = posY;
}
