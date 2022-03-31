#define MAX_LABEL_LEN 10
#define MAX_MENU_BTNS 10

struct MenuButton {
  char label[MAX_LABEL_LEN];
  int labelLength;
  void(*event);
};

struct Menu {
  struct MenuButton buttons[MAX_MENU_BTNS];
  int numButtons;
  int width;
  int height;
  int posX;
  int posY;
  int paddingY;
  int selectedButton;
};

void initMenu(struct Menu* menu);

void drawMenu(struct Menu* menu);

void drawInitialMenu(struct Menu* menu, int showBg);

void updateMenuButtonSelection(struct Menu* menu);

void addMenuButton(struct Menu* menu, char* label, void (*event)());

void runMenuButtonEvent(struct Menu* menu, int buttonIndex);
