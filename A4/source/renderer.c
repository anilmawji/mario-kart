#include <ctype.h>
#include <fontassets.h>
#include <renderer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "color.h"

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

struct SpriteSheet fontSheet;

void drawPixel(Pixel* pixel) {
  long int location = (pixel->x + fbinfo.xOffset) * (fbinfo.bitsPerPixel / 8) +
                      (pixel->y + fbinfo.yOffset) * fbinfo.lineLength;
  *((unsigned short int*)(fbinfo.fbptr + location)) = pixel->color;
}

void clearScreen() { memset(fbinfo.fbptr, 0, fbinfo.screenSizeBytes); }

// Full credit goes to
// https://stackoverflow.com/questions/58449462/rgb565-to-grayscale
int16_t greyscale(int color) {
  int16_t red = ((color & 0xF800) >> 11);
  int16_t green = ((color & 0x07E0) >> 5);
  int16_t blue = (color & 0x001F);
  int16_t grayscale = (0.2126 * red) + (0.7152 * green / 2.0) + (0.0722 * blue);

  return (grayscale << 11) + (grayscale << 6) + grayscale;
}

short* generateGreyscaleImage(short* pixelData, int width, int height) {
  int i = 0;
  int len = width * height;
  short* newPixelData = malloc(len * sizeof(short));

  if (newPixelData == NULL) {
    printf("Error: Call to malloc failed\n");
  }
  memcpy(newPixelData, pixelData, len * sizeof(short));

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      newPixelData[i] = greyscale(pixelData[i]);
      i++;
    }
  }
  return newPixelData;
}

// Filled rectangle
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

// Rectangle outline
void drawStrokeRect(int posX, int posY, int width, int height, int strokeSize,
                    int strokeColor) {
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

// Filled rectangle
void drawFillRectWithStroke(int posX, int posY, int width, int height,
                            int bgColor, int strokeSize, int strokeColor) {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (x <= strokeSize || x >= width - strokeSize || y <= strokeSize ||
          y >= height - strokeSize) {
        pixel->color = strokeColor;
      } else {
        pixel->color = bgColor;
      }
      pixel->x = posX + x;
      pixel->y = posY + y;

      drawPixel(pixel);
    }
  }
}

void drawImage(short int* pixelData, int posX, int posY, int width, int height,
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

      // sprintf("%d\n", pixelData[i]);

      drawPixel(pixel);
      i++;
    }
  }
}

void drawOverlayedCroppedImages(short* pixelData, int posX, int posY,
                                int origWidth, int startX, int startY,
                                int width, int height, int oldBgColor,
                                short* bgPixelData, int bgStartX,
                                int bgStartY) {
  int offset = startY * origWidth + startX;
  int bgOffset = bgStartY * origWidth + bgStartX;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (pixelData[offset + x] == oldBgColor) {
        pixel->color = bgPixelData[bgOffset + x];
      } else {
        pixel->color = pixelData[offset + x];
      }
      pixel->x = x + posX;
      pixel->y = y + posY;

      drawPixel(pixel);
    }
    offset += origWidth;
    bgOffset += origWidth;
  }
}

void drawCroppedImage(short* pixelData, int posX, int posY, int origWidth,
                      int startX, int startY, int width, int height,
                      int oldBgColor, int newBgColor) {
  // Find the index of the first pixel in the 1D array
  int offset = startY * origWidth + startX;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      // Recolor background pixels to the desired color
      if (pixelData[offset + x] == oldBgColor) {
        pixel->color = newBgColor;
      } else {
        pixel->color = pixelData[offset + x];
      }
      // Ofset image by the desired position
      pixel->x = x + posX;
      pixel->y = y + posY;

      drawPixel(pixel);
    }
    // Skip to next row of pixels
    offset += origWidth;
  }
}

/**
 * Helper function primarily for debugging purposes
 */
void drawSpriteSheet(struct SpriteSheet* sheet, int posX, int posY) {
  drawImage(sheet->pixelData, posX, posY, sheet->width, sheet->height,
            sheet->backgroundColor, sheet->backgroundColor);
}

void drawSpriteTile(struct SpriteSheet* sheet, int posX, int posY, int tileX,
                    int tileY, int newBgColor) {
  int startX = (tileX + 1) * sheet->paddingX + tileX * sheet->tileWidth;
  int startY = (tileY + 1) * sheet->paddingY + tileY * sheet->tileHeight;

  drawCroppedImage(sheet->pixelData, posX, posY, sheet->width, startX, startY,
                   sheet->tileWidth, sheet->tileHeight, sheet->backgroundColor,
                   newBgColor);
}

void drawOverlayedSpriteTiles(struct SpriteSheet* sheet, int posX,
                                     int posY, int tileX, int tileY,
                                     struct SpriteSheet* bgSheet, int bgTileX, int bgTileY) {
  int startX = (tileX + 1) * sheet->paddingX + tileX * sheet->tileWidth;
  int startY = (tileY + 1) * sheet->paddingY + tileY * sheet->tileHeight;

  int bgStartX = (bgTileX + 1) * sheet->paddingX + bgTileX * sheet->tileWidth;
  int bgStartY = (bgTileY + 1) * sheet->paddingY + bgTileY * sheet->tileHeight;

  drawOverlayedCroppedImages(sheet->pixelData, posX, posY, sheet->width,
                                    startX, startY, sheet->tileWidth,
                                    sheet->tileHeight, sheet->backgroundColor,
                                    bgSheet->pixelData, bgStartX, bgStartY);
}

void drawText(char* text, int length, int posX, int posY, int bgColor) {
  int ch;

  for (int i = 0; i < length; i++) {
    if (text[i] != ' ') {
      ch = toupper(text[i]);
      drawSpriteTile(&fontSheet, posX, posY, fontMap[ch][0], fontMap[ch][1],
                     bgColor);
    }
    posX += fontSheet.tileWidth;
  }
}

int getSpriteTileX(struct SpriteSheet* sheet, int tileNum) {
  return tileNum % sheet->cols;
}

int getSpriteTileY(struct SpriteSheet* sheet, int tileNum) {
  return tileNum / sheet->cols;
}

// Returns the number of tiles in the spritesheet
int getNumTiles(struct SpriteSheet* sheet) { return sheet->rows * sheet->cols; }

void initFontMap() {
  initSpriteSheet(&fontSheet, (short*)font_sprite.pixel_data, 720, 380, 3, 13,
                  32, 32, 4, 4, -14824);

  // Precompute the location of each symbol in the sprite sheet
  for (int i = 0; i < NUM_SYMBOLS; i++) {
    int ch = symbols[i];
    fontMap[ch][0] = getSpriteTileX(&fontSheet, i);
    fontMap[ch][1] = getSpriteTileY(&fontSheet, i);
  }
}

void initSpriteSheet(struct SpriteSheet* sheet, short* pixelData, int width,
                     int height, int rows, int cols, int tileWidth,
                     int tileHeight, int paddingX, int paddingY, int bgColor) {
  sheet->pixelData = pixelData;
  sheet->width = width;
  sheet->height = height;
  sheet->rows = rows;
  sheet->cols = cols;
  sheet->tileWidth = tileWidth;
  sheet->tileHeight = tileHeight;
  sheet->paddingX = paddingX;
  sheet->paddingY = paddingY;
  sheet->backgroundColor = bgColor;
}

void initRenderer(int viewportWidth, int viewportHeight) {
  fbinfo = initFbInfo();
  viewportX = (fbinfo.screenWidth - viewportWidth) / 2;
  viewportY = (fbinfo.screenHeight - viewportHeight) / 2;
  pixel = malloc(sizeof(Pixel));

  initFontMap();
}

void cleanUpRenderer() {
  munmap(fbinfo.fbptr, fbinfo.screenSizeBytes);

  free(pixel);
  pixel = NULL;
}
