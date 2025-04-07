#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>
#include <uart.h>

#include "lcd.h"
#include "led.h"
#include "joystick.h"
#include "types.h"
#include "motor.h"
#include "timer.h"

//Perform any necessary initialization for the touchscreen and associated ADC pins.
void touch_init(void){
  //set up the I/O pins E1, E2, E3 to be output pins
  CLEARBIT(TRISEbits.TRISE1); //I/O pin set to output
  CLEARBIT(TRISEbits.TRISE2); //I/O pin set to output
  CLEARBIT(TRISEbits.TRISE3); //I/O pin set to output
}

// Configure pins and associated ADC to sample either the x- or y-dimension
// dimensions == 0 standby mode
// dimension == 1 sample x
// dimenstion == 2 sample y
void touch_select_dim(uint8_t dimension){
  if(dimension == 0){ // Standby
    SETBIT(LATEbits.LATE1);
    SETBIT(LATEbits.LATE2);
    SETBIT(LATEbits.LATE3);
  }else if(dimension == 1){ // X
    CLEARBIT(LATEbits.LATE1);
    SETBIT(LATEbits.LATE2);
    SETBIT(LATEbits.LATE3);
  }else if(dimension == 2){ // Y
    SETBIT(LATEbits.LATE1);
    CLEARBIT(LATEbits.LATE2);
    CLEARBIT(LATEbits.LATE3);
  }
}

// Acquire a position sample from the touchscreen on the dimension selected with the previous 
// touch_select_dim(...) command
uint16_t touch_read(void);
