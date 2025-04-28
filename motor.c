/*
 * File:   motor.c
 * Author: team-4a
 *
 * Created on March 31, 2025, 6:45 PM
 */


#include "motor.h"
#include <stdlib.h>
int prev_x = 0;
int prev_y = 0;

void enable_timer1(){
    /* Disable Timer 1 */
    CLEARBIT(T1CONbits.TON);
    /* Setup Timer 1 for no interrupts, 50ms period */
    CLEARBIT(T1CONbits.TCS);
    CLEARBIT(T1CONbits.TGATE);
    TMR1 = 0x00;
    T1CONbits.TCKPS = 0b10; // Prescaler 1:64
    CLEARBIT(IEC0bits.T1IE);
    CLEARBIT(IFS0bits.T1IF);
    PR1 = 10000; // 50ms period
    /* Enable Timer 1 */
    SETBIT(T1CONbits.TON);
}

void __attribute__((__interrupt__)) _T1Interrupt(void){

    //AD1CHS0bits.CH0SA = 0x0F; / y-axis
    AD1CHS0bits.CH0SA = 0x09; // x-axis

    double minimum_x = 410;
    double maximum_x = 2540;
    double goal_x = (minimum_x + maximum_x) / 2;
    double curr_x_duty;


    goal_x = (2100 + 900) / 2;
    
    double duty_cycle_min = 900.0;
    double duty_cycle_max = 2100.0;

    double kp = 0.1;

    // select the x axis
    touch_select_dim(2);
    
    
    int curr_x = read_touchscreen();
    //lcd_locate(0, 1);
    //lcd_printf("X: %d", curr_x);
    __delay_ms(10);
    double doubled_curr_x = curr_x * 1.0;
    //lcd_locate(0, 6);
    //lcd_printf("X double: %.2f", doubled_curr_x);
    curr_x_duty = mapValue(doubled_curr_x, minimum_x, maximum_x, duty_cycle_min, duty_cycle_max);

    //lcd_locate(0, 2);
    //lcd_printf("Mapped x: %.4f ", curr_x_duty);

    
    int err_x = curr_x_duty - goal_x;
    //lcd_locate(0, 3);
    //lcd_printf("Error x: %.4f ", err_x);

    // have a p controller
    int duty_us = goal_x - (kp * err_x);
    //lcd_locate(0, 4);
    //lcd_printf("Duty: %d", duty_us);

    if(duty_us < 900){
        duty_us = 900;
    }
    else if(duty_us > 2100){
        duty_us = 2100;
    }
    motor_set_duty(1, duty_us);


    IFS0bits.T1IF = 0;
}


void motor_init(uint8_t chan){
    /* Disable Timer 2 */
    CLEARBIT(T2CONbits.TON);
    /* Setup Timer 2 for no interrupts, 20ms period */
    CLEARBIT(T2CONbits.TCS); 
    CLEARBIT(T2CONbits.TGATE);
    TMR2 = 0x00;
    T2CONbits.TCKPS = 0b10;
    CLEARBIT(IEC0bits.T2IE);
    CLEARBIT(IFS0bits.T2IF);
    PR2 = 4000;
    if(chan == 0){ // for X axis
        /*Configure Output Compare 8 (X-axis motor) */
        CLEARBIT(TRISDbits.TRISD7); // Set OC8 = RD7 as output
    }else if(chan == 1){
        CLEARBIT(TRISDbits.TRISD6); // Set OC7 = RD6 as output !!! not sure
    }
    /* Enable Timer 2 */
    SETBIT(T2CONbits.TON);
    OC8CONbits.OCM = 0b110; // Set PWM, no fault mode
    OC7CONbits.OCM = 0b110; // Set PWM, no fault mode
}


double mapValue(double x, double in_min, double in_max, double out_min, double out_max){
    if(in_max == in_min){
        return out_min;
    }
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    
}
/*
Set the duty cycle of the specified motor channel using 
the provided duty in microseconds. Use Timer2 for the duty cycle.
 */
void motor_set_duty(uint8_t chan, uint16_t duty_us){
    double ratio_us = ((double)duty_us)/20000.0;
    double duty_ratio = ratio_us * PR2;
    //duty_ratio = mapValue(duty_ratio, 200, 400, 2000, 3000);
    //lcd_locate(1,6);
    //lcd_printf("%f", duty_ratio);
    double inverted = (PR2 - duty_ratio);
    //double val = (12.8*1000000) / (64/duty_us);
    //double inverted = PR2 - val;
    // safety check according to lab manual
    //if(duty_us <= 210000 && duty_us >= 90000){
        if(chan == 0){ // for X axis       
            //lcd_locate(1,7);
            //lcd_printf("%f", inverted);
            if(abs(prev_x - inverted) < 10){
                return;
            }
            OC8R = inverted; // First duty cycle of x ms
            OC8RS = inverted; // Next duty cycle of x ms
            //OC8CONbits.OCM = 0b110; // Set PWM, no fault mode
            prev_x = inverted;
        }else if (chan == 1){
            if(abs(prev_y - inverted) < 10){
                return;
            }
            OC7R = inverted; // First duty cycle of y ms
            OC7RS = inverted; // Next duty cycle of y ms
            //OC7CONbits.OCM = 0b110; // Set PWM, no fault mode
            prev_y = inverted;
        }
    //}
}
