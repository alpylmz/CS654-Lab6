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

unsigned short read_touchscreen(){
    SETBIT(AD1CON1bits.SAMP);
    while(!AD1CON1bits.DONE);
    CLEARBIT(AD1CON1bits.DONE);
    __delay_ms(10);
    return ADC1BUF0 % 4096;
}

void wait_motor(){
    __delay_ms(5000);
}

typedef struct Point{
    unsigned int x;
    unsigned int y;
} Point;


int main(){
	// LCD Initialization Sequence 
	__C30_UART=1;	
	lcd_initialize();
	lcd_clear();
    
    touch_init();
    
    motor_init(0);
    
    init_adc1();

    //SETBIT(TRISBbits.TRISB9);
    //SETBIT(AD1PCFGHbits.PCFG20); // digital mode
    
    // for x touchscreen
    SETBIT(TRISBbits.TRISB15);
    CLEARBIT(AD1PCFGLbits.PCFG15); // sets it to analog mode
    
    // for y touchscreen
    SETBIT(TRISBbits.TRISB9); // physical board pin connection look at page 4
    CLEARBIT(AD1PCFGLbits.PCFG9); // sets it to analog mode

    
    /*
    unsigned short median_x, median_y;
    median_x = median_y = 0;
    Point points[4];
    // START CALIBRATING
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
    points[0].x = median_x;
    points[0].y = median_y;
    
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
    points[1].x = median_x;
    points[1].y = median_y;
    
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
    points[2].x = median_x;
    points[2].y = median_y;

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
    points[3].x = median_x;
    points[3].y = median_y;
    
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
    */

    // FINISHED CALIBRATING

    /*
    // use points array
    // get the minimum for x and y values over the 4 points
    int min_x = points[0].x;
    int min_y = points[0].y;
    int max_x = points[0].x;
    int max_y = points[0].y;

    for(i = 1; i < 4; i++){
        if(points[i].x < min_x){
            min_x = points[i].x;
        }
        if(points[i].y < min_y){
            min_y = points[i].y;
        }
        if(points[i].x > max_x){
            max_x = points[i].x;
        }
        if(points[i].y > max_y){
            max_y = points[i].y;
        }
    }
    */
    //AD1CHS0bits.CH0SA = 0x0F; / y-axis
    AD1CHS0bits.CH0SA = 0x09; // x-axis
    // move y to one side of the touchscreen
    motor_set_duty(1, 2100);
    motor_set_duty(0, 2100);

    double minimum_x = 410;
    double maximum_x = 2540;
    double goal_x = (minimum_x + maximum_x) / 2;
    double curr_x_duty;

    goal_x = (2100 + 900) / 2;
    
    double duty_cycle_min = 900.0;
    double duty_cycle_max = 2100.0;

    int kp = 0.1;

    // select the x axis
    touch_select_dim(2);
    
    int i = 0;
    while(1){
        i++;
        if(i == 20){
            i = 0;
            lcd_clear();
        }
        
        int curr_x = read_touchscreen();
        lcd_locate(0, 1);
        lcd_printf("X: %d", curr_x);
        __delay_ms(10);
        double doubled_curr_x = curr_x * 1.0;
        lcd_locate(0, 6);
        lcd_printf("X double: %.2f", doubled_curr_x);
        curr_x_duty = mapValue(doubled_curr_x, minimum_x, maximum_x, duty_cycle_min, duty_cycle_max);

        lcd_locate(0, 2);
        lcd_printf("Mapped x: %.4f ", curr_x_duty);

        
        int err_x = curr_x_duty - goal_x;
        lcd_locate(0, 3);
        lcd_printf("Error x: %.4f ", err_x);

        // have a p controller
        int duty_us = goal_x - (kp * err_x);
        lcd_locate(0, 4);
        lcd_printf("Duty: %d", duty_us);

        if(duty_us < 900){
            duty_us = 900;
        }
        else if(duty_us > 2100){
            duty_us = 2100;
        }
        motor_set_duty(1, duty_us);
        __delay_ms(10);

    }




    

    
    
        
	
    
    return 0;
}
