#include "menu.h"

#include <stdio.h>
#include <string.h>

#include "color.h"
#include "config.h"
#include "controller.h"
#include "renderer.h"

void drawMenuButtonOutline(struct MenuButton* btn, int x, int y, int selected) {
  if (selected) {
    drawStrokeRect(x - CELL_WIDTH / 2, y - CELL_HEIGHT / 2,
                   (btn->labelLength + 1) * CELL_WIDTH, 2 * CELL_HEIGHT, 4,
                   YELLOW);
  } else {
    drawStrokeRect(x - CELL_WIDTH / 2, y - CELL_HEIGHT / 2,
                   (btn->labelLength + 1) * CELL_WIDTH, 2 * CELL_HEIGHT, 4,
                   BLUE);
  }
}

void drawMenu(struct Menu* menu) {
  struct MenuButton* btn;
  int btnX, btnY;

  for (int i = 0; i < menu->numButtons; i++) {
    btn = &menu->buttons[i];
    btnX = menu->posX + (menu->width - btn->labelLength * CELL_WIDTH) / 2;
    btnY = menu->posY + 2 * i * (CELL_HEIGHT + menu->paddingY) +
           2 * menu->paddingY;

    drawMenuButtonOutline(btn, btnX, btnY, i == menu->selectedButton);
  }
}

void drawInitialMenu(struct Menu* menu, int showBg) {
  if (showBg) {
    drawFillRect(menu->posX, menu->posY, menu->width, menu->height, BLACK);
  }

  struct MenuButton* btn;
  int btnX, btnY;

  for (int i = 0; i < menu->numButtons; i++) {
    btn = &menu->buttons[i];
    btnX = menu->posX + (menu->width - btn->labelLength * CELL_WIDTH) / 2;
    btnY = menu->posY + 2 * i * (CELL_HEIGHT + menu->paddingY) +
           2 * menu->paddingY;

    drawFillRect(btnX - CELL_WIDTH / 2, btnY - CELL_HEIGHT / 2,
                 (btn->labelLength + 1) * CELL_WIDTH, 2 * CELL_HEIGHT, BLACK);
    drawText(btn->label, btn->labelLength, btnX, btnY, BLACK);

    drawMenuButtonOutline(btn, btnX, btnY, i == menu->selectedButton);
  }
}

void updateMenuButtonSelection(struct Menu* menu) {
  if (isButtonPressed(JOY_PAD_UP)) {
    if (menu->selectedButton == 0) {
      menu->selectedButton = menu->numButtons - 1;
    } else {
      menu->selectedButton = (menu->selectedButton - 1) % (menu->numButtons);
    }
  } else if (isButtonPressed(JOY_PAD_DOWN)) {
    menu->selectedButton = (menu->selectedButton + 1) % (menu->numButtons);
  }
}

void runMenuButtonEvent(struct Menu* menu, int buttonIndex) {
  void (*event)() = menu->buttons[buttonIndex].event;
  event();
}

void addMenuButton(struct Menu* menu, char* label, void (*event)()) {
  struct MenuButton* btn = &menu->buttons[menu->numButtons++];
  strcpy(btn->label, label);
  btn->labelLength = strlen(label);
  btn->event = event;

  menu->height = menu->numButtons * 2 * (CELL_HEIGHT + menu->paddingY);
  menu->posY = viewportY + (VIEWPORT_HEIGHT - menu->height) / 2;
}

void initMenu(struct Menu* menu) {
  menu->numButtons = 0;
  menu->paddingY = 10;
  menu->width = 300;
  menu->height = 2 * menu->paddingY;
  menu->posX = viewportX + (VIEWPORT_WIDTH - menu->width) / 2;
  menu->posY = viewportY + (VIEWPORT_HEIGHT - menu->height) / 2;
  menu->selectedButton = 0;
}

/*
void updateButtonSelection() {
  if (isButtonPressed(JOY_PAD_UP) && menuSelection == QUIT_BTN) {
    menuSelection = START_BTN;
  } else if (isButtonPressed(JOY_PAD_DOWN) && menuSelection == START_BTN) {
    menuSelection = QUIT_BTN;
  }
}
*/

// int viewportX;
// int viewportY;

// int menuWidth;
// int menuHeight;

// int startBtnX;
// int startBtnY;

// int quitBtnX;
// int quitBtnY;

/*
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
*/
