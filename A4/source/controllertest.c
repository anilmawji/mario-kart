
/*
 * CPSC 359 Assignment 4: Controller Test
 * Winter 2022
 *
 * Team members:
 *   Umar Hassan (umar.hassan@ucalgary.ca)
 *   UCID: 30130208
 *
 *   Anil Mawji (anil.mawji@ucalgary.ca)
 *   UCID: 30099809
 */

#include <controller.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>

/*
int main(int argc, char* argv[]) {
  initGPIO();
  initSNES();

  printf("Created by Umar Hassan and Anil Mawji\n\n");

  int waiting = FALSE;
  int shouldPrint = TRUE;

  do {
    readSNES();
    waiting = TRUE;

    // Loop through each controller button
    for (int i = 0; i < NUM_REAL_BUTTONS; i++) {
      if (isButtonPressed(i)) {
        printf("You have pressed: %s\n\n", getButtonName(i));
        waiting = FALSE;
        // Only print again after having pressed a button, ensuring that we
        // don't print two times in a row
        shouldPrint = TRUE;
      }
    }

    if (waiting && shouldPrint && secondsSinceLastButtonPress() > 0.75) {
      printf("Please press a button...\n\n");
      shouldPrint = FALSE;
    }
  } while (!isButtonPressed(START));

  printf("Program is terminating...\n\n");

  return 0;
}
*/
