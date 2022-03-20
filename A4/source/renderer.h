#include <framebuffer.h>

FrameBufferInfo fbinfo;
Pixel *pixel;

int centerX;
int centerY;

typedef enum {
  TRANSPARENT = 0x0,
  RED = 0xf800,
  BLUE = 0x0000ff,
  GREY = 0x7bef,
  GREEN = 0x5e05
} Color;

typedef struct {
  short int *pixelData;
  int width;
  int height;
  int rows;
  int cols;
  int paddingX;
  int paddingY;
  int backgroundColor;
} SpriteSheet;

void clearScreen();

void drawRect(int posX, int posY, int width, int height, int color);

void drawImage(short int *pixelData, int posX, int posY, int width, int height,
               int oldBgColor, int newBgColor);

void drawSpriteSheet(SpriteSheet sheet, int posX, int posY);

void drawSprite(SpriteSheet sheet, int posX, int posY, int width, int height,
                int offsetY, int frameIndex, int newBgColor);

void drawText(char *text, int length, int posX, int posY, int bgColor);

void initRenderer(int viewportWidth, int viewportHeight);

void cleanUp();