#include <framebuffer.h>

FrameBufferInfo fbinfo;
Pixel *pixel;

int viewportX;
int viewportY;

typedef enum {
  TRANSPARENT = 0x0,
  BLACK = 0x0,
  WHITE = -1,  // idfk how but it works and 0xffff doesn't
  RED = 0xf800,
  BLUE = 0x00ff,
  GREY = 0x7bef,
  GREEN = 0x5e05
} Color;

typedef struct {
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
} SpriteSheet;

void clearScreen();

void drawFillRect(int posX, int posY, int width, int height, int color);

void drawStrokeRect(int posX, int posY, int width, int height, int strokeSize,
                    int bgcolor, int strokeColor);

void drawImage(short int *pixelData, int posX, int posY, int width, int height,
               int oldBgColor, int newBgColor);

void drawCroppedImage(short int *pixelData, int posX, int posY, int oldWidth,
                      int oldHeight, int startX, int startY, int endX, int endY,
                      int oldBgColor, int newBgColor);

void drawSpriteSheet(SpriteSheet sheet, int posX, int posY);

void drawSprite(SpriteSheet sheet, int posX, int posY, int width, int height,
                int startX, int startY, int newBgColor);

void drawSpriteTile(SpriteSheet sheet, int posX, int posY, int tileX, int tileY,
                    int newBgColor);

void drawText(char *text, int length, int posX, int posY, int bgColor);

void initRenderer(int viewportWidth, int viewportHeight);

void cleanUpRenderer();
