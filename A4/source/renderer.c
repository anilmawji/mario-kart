#include <renderer.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stdio.h>

void drawPixel(Pixel *pixel) {
  long int location = (pixel->x + fbinfo.xOffset) * (fbinfo.bitsPerPixel / 8) +
                      (pixel->y + fbinfo.yOffset) * fbinfo.lineLength;
  *((unsigned short int *)(fbinfo.fbptr + location)) = pixel->color;
}

void clearScreen() { memset(fbinfo.fbptr, 0, fbinfo.screenSizeBytes); }

void drawRect(int posX, int posY, int width, int height, int color) {
  pixel->color = color;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      pixel->x = posX + x;
      pixel->y = posY + y;

      drawPixel(pixel);
    }
  }
}

void drawImage(short int *pixelData, int posX, int posY, int width,
               int height, int backgroundColor) {
  int i = 0;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      pixel->x = x + posX;
      pixel->y = y + posY;

      if (pixelData[i] == 0) {
        pixel->color = backgroundColor;
      } else {
        pixel->color = pixelData[i];
      }

      drawPixel(pixel);
      i++;
    }
  }
}

void initRenderer(int viewportWidth, int viewportHeight) {
  centerX = (fbinfo.screenWidth - viewportWidth) / 2;
  centerY = (fbinfo.screenHeight - viewportHeight) / 2;
  pixel = malloc(sizeof(Pixel));
}
