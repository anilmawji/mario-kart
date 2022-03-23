#include <ctype.h>
#include <font.h>
#include <renderer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define NUM_SYMBOLS 36
#define MAX_ASCII_VAL 90

// Currently unused
// Can be used to modify text colors in the future
typedef enum {
  TEXT_COLOR = -33,
  STROKE_COLOR = 863,
  BG_COLOR = -14824
} DefaultFontColors;

char symbols[NUM_SYMBOLS] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                             'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                             'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1',
                             '2', '3', '4', '5', '6', '7', '8', '9', '0'};
int fontMap[MAX_ASCII_VAL + 1][2];

SpriteSheet fontSheet;

void drawPixel(Pixel *pixel) {
  long int location = (pixel->x + fbinfo.xOffset) * (fbinfo.bitsPerPixel / 8) +
                      (pixel->y + fbinfo.yOffset) * fbinfo.lineLength;
  *((unsigned short int *)(fbinfo.fbptr + location)) = pixel->color;
}

void clearScreen() { memset(fbinfo.fbptr, 0, fbinfo.screenSizeBytes); }

//Filled rectangle
void drawFillRect(int posX, int posY, int width, int height, int bgcolor) {
  pixel->color = bgcolor;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      pixel->x = posX + x;
      pixel->y = posY + y;

      drawPixel(pixel);
    }
  }
}

//Rectangle outline
void drawStrokeRect(int posX, int posY, int width, int height, int strokeSize, int strokeColor) {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (x <= strokeSize || x >= width - strokeSize || y <= strokeSize ||
          y >= height - strokeSize) {
        pixel->color = strokeColor;
        pixel->x = posX + x;
        pixel->y = posY + y;

        drawPixel(pixel);
      }
    }
  }
}

void drawImage(short int *pixelData, int posX, int posY, int width, int height,
               int oldBgColor, int newBgColor) {
  int i = 0;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (pixelData[i] == oldBgColor) {
        pixel->color = newBgColor;
      } else {
        pixel->color = pixelData[i];
      }
      pixel->x = posX + x;
      pixel->y = posY + y;

      drawPixel(pixel);
      i++;
    }
  }
}

void drawCroppedImage(short int *pixelData, int posX, int posY, int oldWidth,
                      int oldHeight, int startX, int startY, int endX, int endY,
                      int oldBgColor, int newBgColor) {
  int i = 0;

  for (int y = 0; y < oldHeight; y++) {
    for (int x = 0; x < oldWidth; x++) {
      if (y <= endY && y >= startY && x <= endX && x >= startX) {
        if (pixelData[i] == oldBgColor) {
          pixel->color = newBgColor;
        } else {
          pixel->color = pixelData[i];
        }
        pixel->x = posX + x - startX;
        pixel->y = posY + y - startY;

        drawPixel(pixel);
      }
      i++;
    }
  }
}

/**
 * Helper function primarily for debugging purposes
 */
void drawSpriteSheet(SpriteSheet sheet, int posX, int posY) {
  drawImage(sheet.pixelData, posX, posY, sheet.width, sheet.height,
            sheet.backgroundColor, sheet.backgroundColor);
}

void drawSprite(SpriteSheet sheet, int posX, int posY, int width, int height,
                int startX, int startY, int newBgColor) {
  drawCroppedImage(sheet.pixelData, posX, posY, sheet.width, sheet.height,
                   startX, startY, startX + width - 1, startY + height - 1,
                   sheet.backgroundColor, newBgColor);
}

void drawSpriteTile(SpriteSheet sheet, int posX, int posY, int tileX, int tileY,
                    int newBgColor) {
  int startX = tileX * sheet.tileWidth + (1 + tileX) * sheet.paddingX;
  int startY = tileY * sheet.tileHeight + (1 + tileY) * sheet.paddingY;
  int endX = startX + sheet.tileWidth - 1;
  int endY = startY + sheet.tileHeight - 1;

  drawCroppedImage(sheet.pixelData, posX, posY, sheet.width, sheet.height,
                   startX, startY, endX, endY, sheet.backgroundColor,
                   newBgColor);
}

void drawText(char *text, int length, int posX, int posY, int bgColor) {
  int ch;

  for (int i = 0; i < length; i++) {
    if (text[i] != ' ') {
      ch = toupper(text[i]);
      drawSpriteTile(fontSheet, posX, posY, fontMap[ch][0], fontMap[ch][1],
                     bgColor);
    }
    posX += fontSheet.tileWidth;
  }
}

void initFontMap() {
  fontSheet.pixelData = (short int *)font_sprite.pixel_data;
  fontSheet.width = 720;
  fontSheet.height = 380;
  fontSheet.rows = 3;
  fontSheet.cols = 13;
  fontSheet.tileWidth = 32;
  fontSheet.tileHeight = 32;
  fontSheet.paddingX = 4;
  fontSheet.paddingY = 4;
  fontSheet.backgroundColor = -14824;

  int ch;

  // Precompute the location of each symbol in the sprite sheet
  for (int i = 0; i < NUM_SYMBOLS; i++) {
    ch = symbols[i];
    fontMap[ch][0] = i % fontSheet.cols;  // tileX
    fontMap[ch][1] = i / fontSheet.cols;  // tileY

    // printf("%c\t%d\t%d\n", ch, fontMap[ch][0], fontMap[ch][1]);
  }
}

void initRenderer(int viewportWidth, int viewportHeight) {
  viewportX = (fbinfo.screenWidth - viewportWidth) / 2;
  viewportY = (fbinfo.screenHeight - viewportHeight) / 2;
  pixel = malloc(sizeof(Pixel));

  initFontMap();
}

void cleanUpRenderer() {
  free(pixel);
  pixel = NULL;
}
