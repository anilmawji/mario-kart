typedef enum { MV_UP, MV_DOWN, MV_RIGHT, MV_LEFT } Direction;

struct GameObject {
  int id;
  int index;
  int currentCellId;

  int width;
  int height;

  int posX;
  int posY;
  int prevPosX;  // Track the previous position to help with rendering
  int prevPosY;

  short* sprite;
  int spriteBgColor;
  Direction dir;

  int speed;
  float lastUpdateTime;  // Measured in milliseconds
  float updateInterval;
};

void initGameObject(struct GameObject* obj, int posX, int posY, int id,
                    short* sprite, int spriteBgColor, Direction dir, int speed);

void setGameObjectPos(struct GameObject* obj, int posX, int posY);

