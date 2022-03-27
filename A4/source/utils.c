#include <stdlib.h>

int randRangeUtil(int min, int max) { return rand() % (max + 1 - min) + min; }

int clampUtil(int val, int min, int max) {
  if (val >= max) return max;

  if (val <= min) return min;

  return val;
}
