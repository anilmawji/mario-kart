/*
 * Modified from
 * http://cep.xray.aps.anl.gov/software/qt4-x11-4.2.2/qtopiacore-testingframebuffer.html
 * for the purpose of assignment 4 CPSC359 P19
 */

#include <fcntl.h>
#include <framebuffer.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

FrameBufferInfo initFbInfo(void) {
  int fbfd = 0;
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;
  long int screenSize = 0;
  void *fbptr = 0;

  // Open the file for reading and writing
  fbfd = open("/dev/fb0", O_RDWR);
  if (fbfd == -1) {
    perror("Error: cannot open framebuffer device");
    exit(1);
  }
  printf("The framebuffer device was opened successfully.\n");

  // Get fixed screen information
  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
    perror("Error reading fixed information");
    exit(2);
  }

  // Get variable screen information
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
    perror("Error reading variable information");
    exit(3);
  }

  // Figure out the size of the screen in bytes
  screenSize =
      vinfo.xres_virtual * vinfo.yres_virtual * vinfo.bits_per_pixel / 8;
  printf("Hello: %d", vinfo.bits_per_pixel);

  // Map the device to memory
  fbptr =
      (char *)mmap(0, screenSize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

  if ((int)fbptr == -1) {
    perror("Error: failed to map framebuffer device to memory");
    exit(4);
  }

  printf("The framebuffer device was mapped to memory successfully.\n");
  printf("%dx%d, %dbpp\n", vinfo.xres_virtual, vinfo.yres_virtual,
         vinfo.bits_per_pixel);

  FrameBufferInfo result = {(char *)fbptr,
                            (int)vinfo.xoffset,
                            (int)vinfo.yoffset,
                            (int)vinfo.bits_per_pixel,
                            (int)finfo.line_length,
                            (float)screenSize,
                            (int)vinfo.xres_virtual,
                            (int)vinfo.yres_virtual,
                            (int)fbfd};

  return result;
}
