#include "menu.h"

#include <string.h>

#include "color.h"
#include "config.h"
#include "controller.h"
#include "menuassets.h"
#include "renderer.h"

/*
typedef struct {
    char* label;
    int labelLength;
    void* event;
    int posX;
    int posY;
    int bgColor;
} MenuButton;

MenuButton selectedButton;

void menu_draw_button(MenuButton button, int highlighted) {
  drawText(button.label, button.labelLength, button.posX, button.posY,
button.bgColor);

  if (selectedButton.posX == button.posX && selectedButton.posY == button.posY)
{ drawStrokeRect(button.posX - CELL_WIDTH / 2, button.posY - CELL_HEIGHT / 2,
                   (button.labelLength + 1) * CELL_WIDTH, 2 * CELL_HEIGHT, 4,
RED);
  }
}

void drawMenuScreen() {

}

void updateButtonSelection() {
  if (isButtonPressed(JOY_PAD_UP) && menuSelection == QUIT_BTN) {
    menuSelection = START_BTN;
  } else if (isButtonPressed(JOY_PAD_DOWN) && menuSelection == START_BTN) {
    menuSelection = QUIT_BTN;
  }
}
*/

short* menuBackground = (short*)menu_background.pixel_data;
short* menuTitle = (short*)menu_title.pixel_data;

int viewportX;
int viewportY;

int menuWidth;
int menuHeight;

int startBtnX;
int startBtnY;

int quitBtnX;
int quitBtnY;

// TODO
void drawMenuButton(int buttonIndex, char* text) {}

void drawInitialMenuScreen() {
  // drawImage(menuBackground, 0, 0, 1280, 640, -1, RED);

  drawFillRect(viewportX, viewportY, menuWidth, menuHeight, GREEN);

  drawImage(menuTitle, viewportX + (menuWidth - menu_title.width) / 2,
            viewportY + CELL_HEIGHT, menu_title.width, menu_title.height, WHITE,
            GREEN);

  drawText("start", 5, startBtnX, startBtnY, GREEN);
  drawText("quit", 4, quitBtnX, quitBtnY, GREEN);

  drawText("Made by Anil Mawji and Umar Hassan", 34,
           viewportX + (menuWidth - 34 * CELL_WIDTH) / 2,
           viewportY + menuHeight - 2 * CELL_HEIGHT, GREEN);
}

void updateMenuScreen() {
  if (isButtonPressed(JOY_PAD_UP) && menuSelection == QUIT_BTN) {
    menuSelection = START_BTN;
  } else if (isButtonPressed(JOY_PAD_DOWN) && menuSelection == START_BTN) {
    menuSelection = QUIT_BTN;
  }

  if (menuSelection == START_BTN) {
    drawStrokeRect(startBtnX - CELL_WIDTH / 2, startBtnY - CELL_HEIGHT / 2,
                   (5 + 1) * CELL_WIDTH, 2 * CELL_HEIGHT, 4, BLUE);
    drawStrokeRect(quitBtnX - CELL_WIDTH / 2, quitBtnY - CELL_HEIGHT / 2,
                   (4 + 1) * CELL_WIDTH, 2 * CELL_HEIGHT, 4, GREEN);
  } else if (menuSelection == QUIT_BTN) {
    drawStrokeRect(quitBtnX - CELL_WIDTH / 2, quitBtnY - CELL_HEIGHT / 2,
                   (4 + 1) * CELL_WIDTH, 2 * CELL_HEIGHT, 4, BLUE);
    drawStrokeRect(startBtnX - CELL_WIDTH / 2, startBtnY - CELL_HEIGHT / 2,
                   (5 + 1) * CELL_WIDTH, 2 * CELL_HEIGHT, 4, GREEN);
  }
}

void initMenuScreen(int x, int y, int width, int height) {
  menuSelection = START_BTN;
  viewportX = x;
  viewportY = y;
  menuWidth = width;
  menuHeight = height;

  startBtnX = viewportX + (menuWidth - 5 * CELL_WIDTH) / 2;
  startBtnY = viewportY + (menuHeight - CELL_HEIGHT) / 2;

  quitBtnX = viewportX + (menuWidth - 4 * CELL_WIDTH) / 2;
  quitBtnY = viewportY + (menuHeight - CELL_HEIGHT) / 2 + 3 * CELL_HEIGHT;
}
