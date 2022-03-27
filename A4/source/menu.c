#include <string.h>
#include "menuassets.h"
#include "renderer.h"
#include "color.h"
#include "controller.h"
#include "config.h"
#include "menu.h"

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
  drawText(button.label, button.labelLength, button.posX, button.posY, button.bgColor);

  if (selectedButton.posX == button.posX && selectedButton.posY == button.posY) {
    drawStrokeRect(button.posX - CELL_WIDTH / 2, button.posY - CELL_HEIGHT / 2,
                   (button.labelLength + 1) * CELL_WIDTH, 2 * CELL_HEIGHT, 4, RED);
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

// TODO
void drawMenuButton(int buttonIndex, char* text) {}

void drawMenuScreen() {
  // drawImage(menuBackground, 0, 0, 1280, 640, -1, RED);

  drawImage(menuTitle, viewportX + (VIEWPORT_WIDTH - menu_title.width) / 2,
            viewportY + CELL_HEIGHT, menu_title.width, menu_title.height, WHITE,
            GREEN);

  int startBtnX = viewportX + (VIEWPORT_WIDTH - 5 * CELL_WIDTH) / 2;
  int startBtnY = viewportY + (VIEWPORT_HEIGHT - CELL_HEIGHT) / 2;

  int quitBtnX = viewportX + (VIEWPORT_WIDTH - 4 * CELL_WIDTH) / 2;
  int quitBtnY =
      viewportY + (VIEWPORT_HEIGHT - CELL_HEIGHT) / 2 + 3 * CELL_HEIGHT;

  drawText("start", 5, startBtnX, startBtnY, GREEN);
  drawText("quit", 4, quitBtnX, quitBtnY, GREEN);

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

  drawText("Made by Anil Mawji and Umar Hassan", 34,
           viewportX + (VIEWPORT_WIDTH - 34 * CELL_WIDTH) / 2,
           viewportY + VIEWPORT_HEIGHT - 2 * CELL_HEIGHT, GREEN);
}

void updateMenuScreen() {
  if (isButtonPressed(JOY_PAD_UP) && menuSelection == QUIT_BTN) {
    menuSelection = START_BTN;
  } else if (isButtonPressed(JOY_PAD_DOWN) && menuSelection == START_BTN) {
    menuSelection = QUIT_BTN;
  }
}

void initMenuScreen() { menuSelection = START_BTN; }
