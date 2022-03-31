#include "framebuffer.h"

FrameBufferInfo fbinfo;
Pixel *pixel;

int viewportX;
int viewportY;

struct SpriteSheet {
  short int *pixelData;
  int width;
  int height;
  int rows;
  int cols;
  int tileWidth;
  int tileHeight;
  int paddingX;
  int paddingY;
  int backgroundColor;
};

void clearScreen();

void drawFillRect(int posX, int posY, int width, int height, int color);

void drawStrokeRect(int posX, int posY, int width, int height, int strokeSize,
                    int strokeColor);

void drawFillRectWithStroke(int posX, int posY, int width, int height,
                            int bgColor, int strokeSize, int strokeColor);

void drawImage(short int *pixelData, int posX, int posY, int width, int height,
               int oldBgColor, int newBgColor);

void drawCroppedImage(short int *pixelData, int posX, int posY, int oldWidth,
                      int oldHeight, int startX, int startY, int endX, int endY,
                      int oldBgColor, int newBgColor);

void drawSpriteSheet(struct SpriteSheet *sheet, int posX, int posY);

void drawSprite(struct SpriteSheet *sheet, int posX, int posY, int width,
                int height, int startX, int startY, int newBgColor);

void drawSpriteTile(struct SpriteSheet *sheet, int posX, int posY, int tileX,
                    int tileY, int newBgColor);

void initSpriteSheet(struct SpriteSheet *sheet, short *pixelData, int width,
                     int height, int rows, int cols, int tileWidth,
                     int tileHeight, int paddingX, int paddingY, int bgColor);

void drawText(char *text, int length, int posX, int posY, int bgColor);

void initRenderer(int viewportWidth, int viewportHeight);

void cleanUpRenderer();
