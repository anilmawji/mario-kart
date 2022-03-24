#include <fcntl.h>
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <wiringPi.h>

unsigned int* gpioPtr;

unsigned int* initGPIO() {
  int fdgpio = open("/dev/gpiomem", O_RDWR);

  if (fdgpio < 0) {
    perror("Failed to initialize GPIO");
    exit(1);
  }
  // Use GPIO as shared memory
  gpioPtr = (unsigned int*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED,
                                fdgpio, 0);

  return gpioPtr;
}

unsigned int* getGPIOPtr() { return gpioPtr; }
