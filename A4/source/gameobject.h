typedef enum { MV_UP, MV_DOWN, MV_RIGHT, MV_LEFT } Direction;

struct GameObject {
  int id;
  int index;

  int width;
  int height;

  int posX;
  int posY;

  short* sprite;
  int spriteBgColor;
  Direction dir;

  int speed;
  float lastUpdateTime;  // Measured in milliseconds
  float updateInterval;

  struct SpriteSheet* spriteSheet;
  int animationFrame;
};

struct AnimatedEntity {

};

void initGameObject(struct GameObject* obj, int posX, int posY, int id,
                    short* sprite, int spriteBgColor, Direction dir, int speed);

