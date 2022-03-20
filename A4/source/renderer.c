#include <ctype.h>
#include <font.h>
#include <renderer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define NUM_SYMBOLS 36

typedef enum {
  TEXT_COLOR = -33,
  STROKE_COLOR = 863,
  BG_COLOR = -14824
} DefaultFontColors;

char symbols[NUM_SYMBOLS] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                             'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                             'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1',
                             '2', '3', '4', '5', '6', '7', '8', '9', '0'};
int fontMap[91][2];

SpriteSheet fontSheet;

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

void drawImage(short int *pixelData, int posX, int posY, int width, int height,
               int oldBgColor, int newBgColor) {
  int i = 0;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      pixel->x = posX + x;
      pixel->y = posY + y;

      if (pixelData[i] == oldBgColor) {
        pixel->color = newBgColor;
      } else {
        pixel->color = pixelData[i];
      }

      drawPixel(pixel);
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
                int offsetX, int offsetY, int newBgColor) {
  int startY = offsetY * height + (1 + offsetY) * sheet.paddingY;
  int startX = offsetX * width + (1 + offsetX) * sheet.paddingX;
  int endY = startY + height - 1;
  int endX = startX + width - 1;
  int i = 0;

  for (int y = 0; y < sheet.height; y++) {
    for (int x = 0; x < sheet.width; x++) {
      if (y <= endY && y >= startY && x <= endX && x >= startX) {
        if (sheet.pixelData[i] == sheet.backgroundColor) {
          pixel->color = newBgColor;
        } else {
          pixel->color = sheet.pixelData[i];
        }
        pixel->x = posX + x - startX;
        pixel->y = posY + y - startY;
        drawPixel(pixel);
      }
      i++;
    }
  }
}

void drawText(char *text, int length, int posX, int posY, int bgColor) {
  int ch;

  for (int i = 0; i < length; i++) {
    if (text[i] != ' ') {
      ch = toupper(text[i]);
      drawSprite(fontSheet, posX, posY, 32, 32, fontMap[ch][0], fontMap[ch][1],
                bgColor);
    }
    posX += 32;
  }
}

void initFontMap() {
  fontSheet.pixelData = (short int *)font_sprite.pixel_data;
  fontSheet.width = 720;
  fontSheet.height = 380;
  fontSheet.rows = 3;
  fontSheet.cols = 13;
  fontSheet.paddingX = 4;
  fontSheet.paddingY = 4;
  fontSheet.backgroundColor = -14824;

  int ch;

  //Precompute the location of each symbol in the sprite sheet
  for (int i = 0; i < NUM_SYMBOLS; i++) {
    ch = symbols[i];
    fontMap[ch][0] = i % fontSheet.cols;  // offsetX
    fontMap[ch][1] = i / fontSheet.cols;  // offsetY

    //printf("%c\t%d\t%d\n", ch, fontMap[ch][0], fontMap[ch][1]);
  }
}

void initRenderer(int viewportWidth, int viewportHeight) {
  centerX = (fbinfo.screenWidth - viewportWidth) / 2;
  centerY = (fbinfo.screenHeight - viewportHeight) / 2;
  pixel = malloc(sizeof(Pixel));

  initFontMap();
}

void cleanUp() { free(pixel); }
