typedef enum { MV_UP, MV_RIGHT, MV_DOWN, MV_LEFT } Direction;

struct GameObject {
  int id;
  int index;

  int width;
  int height;

  int posX;
  int posY;
  Direction dir;

  struct SpriteSheet* spriteSheet;
  int spriteTileX;
  int spriteTileY;

  int speed;
  float lastUpdateTime;  // Measured in milliseconds
  float updateInterval;
};

struct AnimatedEntity {
  int animationFrame;
};

void initGameObject(struct GameObject* obj, int posX, int posY, int id,
                       struct SpriteSheet* sheet, int spriteTileX,
                       int spriteTileY, Direction dir, int speed);
