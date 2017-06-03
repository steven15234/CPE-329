/*
 * PWM.h
 *
 *  Created on: Apr 20, 2017
 *      Author: Derek
 */

#ifndef PWM_H_
#define PWM_H_

#include "delay.h"
#include "msp.h"

#define PWM_RS 0x07B
#define PWM_SR 0x07C


void new_PWM(Timer_A_Type * timer, uint port, uint mode, float freq, float duty_cycle){
    uint limit, base;

    //PWM Period
    limit = (uint) (((float)GLOBAL_FREQ / (freq * 1000.0f)) - 1.0f);
    //PWM Duty Cycle
    base = (uint) ((float)(limit + 1) * duty_cycle);

    //use either reset/set mode (H->L) or set/reset mode (L->H)
    if (mode == PWM_RS)
        mode = 0xE0;
    else if(mode == PWM_SR)
        mode = 0x60;

    timer->CCR[0] = limit;
    timer->CCR[port] = base;
    timer->CCTL[port] = mode;
    timer->CTL = 0x0214; /*use SMCLK, count up, clear TA0R register*/
}
#endif /* PWM_H_ */
