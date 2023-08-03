#include <stdio.h>
#include <stdlib.h>

#include "APC1.h"

// states of FSM
typedef enum {
    SETUP,
    GET_DATA,
    PROCESS_DATA,
    OUTPUT_DATA,
} STATE;

STATE state = SETUP;

void setup() {

}

void loop() {

  switch (state) {

    case SETUP:
      APC1_Setup();
      state = GET_DATA;
      break;
    case GET_DATA:
      if (APC1_Get_Data()) {
        state = PROCESS_DATA;
      }
      break;
    case PROCESS_DATA:
      if (APC1_Process_Data()) {
        state = OUTPUT_DATA;
      }
      break;
    case OUTPUT_DATA:
      APC1_Print();
      APC1_Print_LCD();
      state = GET_DATA;
      break;
    default:
      return -1;
      break;

  }
  delay(2000);

}
