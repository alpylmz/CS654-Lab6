/****************************************************/
/*                                                  */
/*   CS-454/654 Embedded Systems Development        */
/*   Instructor: Renato Mancuso <rmancuso@bu.edu>   */
/*   Boston University                              */
/*                                                  */
/*   Description: simple HelloWorld application     */
/*                for Amazing Ball platform         */
/*                                                  */
/****************************************************/

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

#define ARR_LEN 10



//Initial configuration by EE #include "types.h"
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);
// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT); 
// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);
// Disable Code Protection
_FGS(GCP_OFF);  


void init_adc1(){
    // disable ADC
    CLEARBIT(AD1CON1bits.ADON);    
    //Configure AD1CON1
    SETBIT(AD1CON1bits.AD12B); //set 12b Operation Mode    
    //CLEARBIT(AD2CON1bits.AD12B); //set 10b Operation Mode for ADC2
    AD1CON1bits.FORM = 0; //set integer output    
    AD1CON1bits.SSRC = 0x7; //set automatic conversion    
    //Configure AD1CON2
    AD1CON2 = 0; //not using scanning sampling    
    //Configure AD1CON3
    CLEARBIT(AD1CON3bits.ADRC); //internal clock source
    AD1CON3bits.SAMC = 0x1F; //sample-to-conversion clock = 31Tad
    AD1CON3bits.ADCS = 0x2; //Tad = 3Tcy (Time cycles)
    
    //enable ADC
    SETBIT(AD1CON1bits.ADON);    
    
    //AD1CHS0bits.CH0SA = 0x04;    //set adc to sample joystick x
    //AD1CHS0bits.CH0SA = 0x05;    //set adc to sample joystick y
}

void init_adc2(){
    // disable ADC
    CLEARBIT(AD2CON1bits.ADON);    
    //Configure AD1CON1
    SETBIT(AD2CON1bits.AD12B); //set 12b Operation Mode    
    //CLEARBIT(AD2CON1bits.AD12B); //set 10b Operation Mode for ADC2
    AD2CON1bits.FORM = 0; //set integer output    
    AD2CON1bits.SSRC = 0x7; //set automatic conversion    
    //Configure AD1CON2
    AD2CON2 = 0; //not using scanning sampling    
    //Configure AD1CON3
    CLEARBIT(AD2CON3bits.ADRC); //internal clock source
    AD2CON3bits.SAMC = 0x1F; //sample-to-conversion clock = 31Tad
    AD2CON3bits.ADCS = 0x2; //Tad = 3Tcy (Time cycles)
    
    //enable ADC
    SETBIT(AD2CON1bits.ADON);    
    
    //AD1CHS0bits.CH0SA = 0x04;    //set adc to sample joystick x
    //AD1CHS0bits.CH0SA = 0x05;    //set adc to sample joystick y
}

// implement merge sort and return the median
unsigned short median(unsigned short *arr, int n) {
    // Sort the array
    int i, j;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                unsigned short temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
    
    // Return the median
    if (n % 2 == 0) {
        return (arr[n / 2 - 1] + arr[n / 2]) / 2;
    } else {
        return arr[n / 2];
    }
}


unsigned short calibrate_touchscreen(){
    __delay_ms(100);
    unsigned short adc1ress[ARR_LEN] = {0};
    int i = 0;
    for(i = 0; i < ARR_LEN; i++){
        SETBIT(AD1CON1bits.SAMP);
        while(!AD1CON1bits.DONE);
        CLEARBIT(AD1CON1bits.DONE);
        if(ADC1BUF0 % 4096 < 1000){
            adc1ress[i] = ADC1BUF0 % 4096;
            lcd_locate(0, 6);
            lcd_printf("ADC1: %d ", adc1ress[i]);
        }else{
            adc1ress[i] = ADC1BUF0 % 4096;
            lcd_locate(0, 6);
            lcd_printf("ADC1: %d", adc1ress[i]);
        }
        __delay_ms(10);
    }

    //__delay_ms(400);

    // calculate the median and return it
    return median(adc1ress, ARR_LEN);
}

void wait_motor(){
    __delay_ms(5000);
}


int main(){
	// LCD Initialization Sequence 
	__C30_UART=1;	
	lcd_initialize();
	lcd_clear();
    
    touch_init();

    
    motor_init(0);
    
    
    init_adc1();
    
    // Move ball to one side of the touchscreen
    int duty_us = 1500;
    motor_set_duty(0, duty_us);
    motor_set_duty(1, duty_us);

    //SETBIT(TRISBbits.TRISB9);
    //SETBIT(AD1PCFGHbits.PCFG20); // digital mode
    
    // for x touchscreen
    SETBIT(TRISBbits.TRISB15);
    CLEARBIT(AD1PCFGLbits.PCFG15); // sets it to analog mode
    
    // for y touchscreen
    SETBIT(TRISBbits.TRISB9); // physical board pin connection look at page 4
    CLEARBIT(AD1PCFGLbits.PCFG9); // sets it to analog mode
    
    touch_select_dim(2);

    unsigned short median_x, median_y;
    median_x = median_y = 0;
    
    
    /*
    __delay_ms(1000);

    touch_select_dim(0);

    touch_select_dim(1);
    __delay_ms(1000);
    //AD1CHS0bits.CH0SA = 0x04; // set to AN20
    AD1CHS0bits.CH0SA = 0x0F; // set to AN15 
     

    
    while(1){
        __delay_ms(100);
        unsigned short adc1ress = 0;

        __delay_ms(100);
        SETBIT(AD1CON1bits.SAMP);
        while(!AD1CON1bits.DONE);
        CLEARBIT(AD1CON1bits.DONE);
        
        adc1ress = ADC1BUF0;
        adc1ress = adc1ress % 4096;
        lcd_locate(0, 6);
        lcd_printf("ADC1: %d", adc1ress);
        lcd_locate(0, 7);
        lcd_printf("Buffer: %d", ADC1BUF0);
        __delay_ms(1000);
        lcd_clear();

    }
    */
    
	while(1){

        // set minimum for both
        AD1CHS0bits.CH0SA = 0x09;
        motor_set_duty(0, 900);
        motor_set_duty(1, 900);

        wait_motor();

        touch_select_dim(2);
        median_x = calibrate_touchscreen();
        
        AD1CHS0bits.CH0SA = 0x0F;
        touch_select_dim(1);
        median_y = calibrate_touchscreen();
        
        lcd_locate(0, 0);
        if(median_x < 1000 && median_y < 1000){
            lcd_printf("C1:\t X:  %d,\t Y:  %d", median_x, median_y);
        }
        else if(median_x > 1000 && median_y < 1000){
            lcd_printf("C1:\t X: %d,\t Y:  %d", median_x, median_y);
        }
        else if(median_x < 1000 && median_y > 1000){
            lcd_printf("C1:\t X:  %d,\t Y: %d", median_x, median_y);
        }
        else{
            lcd_printf("C1:\t X: %d,\t Y: %d", median_x, median_y);
        }
        

        // set minimum for both
        motor_set_duty(0, 900);
        motor_set_duty(1, 2100);

        wait_motor();

        AD1CHS0bits.CH0SA = 0x09;
        touch_select_dim(2);
        median_x = calibrate_touchscreen();
        
        AD1CHS0bits.CH0SA = 0x0F;
        touch_select_dim(1);
        median_y = calibrate_touchscreen();
        
        lcd_locate(0, 1);
        if(median_x < 1000 && median_y < 1000){
            lcd_printf("C2:\t X:  %d,\t Y:  %d", median_x, median_y);
        }
        else if(median_x > 1000 && median_y < 1000){
            lcd_printf("C2:\t X: %d,\t Y:  %d", median_x, median_y);
        }
        else if(median_x < 1000 && median_y > 1000){
            lcd_printf("C2:\t X:  %d,\t Y: %d", median_x, median_y);
        }
        else{
            lcd_printf("C2:\t X: %d,\t Y: %d", median_x, median_y);
        }

        
        // set minimum for both
        motor_set_duty(0, 2100);
        motor_set_duty(1, 2100);

        wait_motor();

        AD1CHS0bits.CH0SA = 0x09;
        touch_select_dim(2);
        median_x = calibrate_touchscreen();
        
        AD1CHS0bits.CH0SA = 0x0F;
        touch_select_dim(1);
        median_y = calibrate_touchscreen();
        
        lcd_locate(0, 2);
        if(median_x < 1000 && median_y < 1000){
            lcd_printf("C3:\t X:  %d,\t Y:  %d", median_x, median_y);
        }
        else if(median_x > 1000 && median_y < 1000){
            lcd_printf("C3:\t X: %d,\t Y:  %d", median_x, median_y);
        }
        else if(median_x < 1000 && median_y > 1000){
            lcd_printf("C3:\t X:  %d,\t Y: %d", median_x, median_y);
        }
        else{
            lcd_printf("C3:\t X: %d,\t Y: %d", median_x, median_y);
        }


        // set minimum for both
        motor_set_duty(0, 2100);
        motor_set_duty(1, 900);

        wait_motor();

        AD1CHS0bits.CH0SA = 0x09;
        touch_select_dim(2);
        median_x = calibrate_touchscreen();
        
        AD1CHS0bits.CH0SA = 0x0F;
        touch_select_dim(1);
        median_y = calibrate_touchscreen();
        
        lcd_locate(0, 3);
        if(median_x < 1000 && median_y < 1000){
            lcd_printf("C4:\t X:  %d,\t Y:  %d", median_x, median_y);
        }
        else if(median_x > 1000 && median_y < 1000){
            lcd_printf("C4:\t X: %d,\t Y:  %d", median_x, median_y);
        }
        else if(median_x < 1000 && median_y > 1000){
            lcd_printf("C4:\t X:  %d,\t Y: %d", median_x, median_y);
        }
        else{
            lcd_printf("C4:\t X: %d,\t Y: %d", median_x, median_y);
        }
        
        
	}
    
    return 0;
}
