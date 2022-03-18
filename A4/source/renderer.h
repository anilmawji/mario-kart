#include <framebuffer.h>

FrameBufferInfo fbinfo;
Pixel *pixel;

int centerX;
int centerY;

void clearScreen();

void drawRect(int posX, int posY, int width, int height, int color);

void drawImage(short int *pixelData, int posX, int posY, int width, int height);

void initRenderer(int viewportWidth, int viewportHeight);