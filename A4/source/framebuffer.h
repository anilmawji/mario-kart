typedef struct {
  char *fbptr;            // Framebuffer pointer
  int xOffset;
  int yOffset;
  int bitsPerPixel;
  int lineLength;
  float screenSizeBytes;  // Screen size in bytes
  int screenWidth;
  int screenHeight;
  int fbfd;
} FrameBufferInfo;

typedef struct {
  int color;
  int x, y;
} Pixel;

FrameBufferInfo initFbInfo(void);
