#include "menu.h"

#include <stdio.h>
#include <string.h>

#include "color.h"
#include "config.h"
#include "controller.h"
#include "renderer.h"

void drawMenuButtonOutline(struct MenuButton* btn, int x, int y, int selected) {
  int color = selected ? YELLOW : BLUE;

  drawStrokeRect(x, y, (btn->labelLength + 1) * CELL_WIDTH, 2 * CELL_HEIGHT, 4,
                 color);
}

void drawMenu(struct Menu* menu) {
  struct MenuButton* btn;
  int btnX, btnY;

  for (int i = 0; i < menu->numButtons; i++) {
    btn = &menu->buttons[i];
    btnX = menu->posX + (menu->width - btn->labelLength * CELL_WIDTH) / 2 -
           CELL_WIDTH / 2;
    btnY = menu->posY + menu->paddingY + i * (2 * CELL_HEIGHT + menu->paddingY);

    // Pick outline color based on whether this is the currently selected button
    drawMenuButtonOutline(btn, btnX, btnY, i == menu->selectedButton);
  }
}

void drawInitialMenu(struct Menu* menu, int showBg) {
  if (showBg) {
    // Menu background
    drawFillRectWithStroke(menu->posX, menu->posY, menu->width, menu->height,
                           BLACK, 4, BLUE);
  }

  struct MenuButton* btn;
  int btnX, btnY;

  for (int i = 0; i < menu->numButtons; i++) {
    btn = &menu->buttons[i];
    btnX = menu->posX + (menu->width - btn->labelLength * CELL_WIDTH) / 2 -
           CELL_WIDTH / 2;
    btnY = menu->posY + menu->paddingY + i * (2 * CELL_HEIGHT + menu->paddingY);

    // Button fill
    drawFillRect(btnX, btnY, (btn->labelLength + 1) * CELL_WIDTH,
                 2 * CELL_HEIGHT, BLACK);
    // Button label
    drawText(btn->label, btn->labelLength, btnX + CELL_WIDTH / 2,
             btnY + CELL_HEIGHT / 2, BLACK);
  }
}

void updateMenuButtonSelection(struct Menu* menu) {
  if (isButtonPressed(JOY_PAD_UP)) {
    // Move selection one button up, circle back to bottom if there is no button above
    if (menu->selectedButton == 0) {
      menu->selectedButton = menu->numButtons - 1;
    } else {
      menu->selectedButton = (menu->selectedButton - 1) % (menu->numButtons);
    }
  } else if (isButtonPressed(JOY_PAD_DOWN)) {
    // Move selection one button down, circle back to top if there is no button below
    menu->selectedButton = (menu->selectedButton + 1) % (menu->numButtons);
  }
}

void runMenuButtonEvent(struct Menu* menu, int buttonIndex) {
  // Run the function associated with the button when it was created
  void (*event)() = menu->buttons[buttonIndex].event;
  event();
}

void addMenuButton(struct Menu* menu, char* label, void (*event)()) {
  if (menu->numButtons == MAX_MENU_BTNS) {
    fprintf(stderr, "Failed to add menu button; the menu already has the maximum number of buttons!");
    return;
  }

  struct MenuButton* btn = &menu->buttons[menu->numButtons];
  strcpy(btn->label, label);
  btn->labelLength = strlen(label);
  btn->event = event;

  menu->numButtons++;
  //Increase menu size to make room for the new button
  menu->height += menu->paddingY + 2 * CELL_HEIGHT;
  //Update menu position
  menu->posY = viewportY + (VIEWPORT_HEIGHT - menu->height) / 2;
}

void initMenu(struct Menu* menu) {
  // Initialize fields to their defaults
  menu->numButtons = 0;
  menu->paddingY = 20;
  menu->width = 300;
  menu->height = menu->paddingY;
  menu->posX = viewportX + (VIEWPORT_WIDTH - menu->width) / 2;
  menu->posY = viewportY + (VIEWPORT_HEIGHT - menu->height) / 2;
  menu->selectedButton = 0;
}
