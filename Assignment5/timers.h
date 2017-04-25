/*
 * timers.h
 *
 *  Created on: Apr 20, 2017
 *      Author: Derek
 */

#ifndef TIMERS_H_
#define TIMERS_H_
#include "delay.h"

void calcTimers(uint* limit, uint* base, float freq, float duty_cycle){
    //PWM Period
    *limit = (uint) ((GLOBAL_FREQ * 1000000.0f/ (freq * 1000.0f)) - 1.0f);
    //PWM Duty Cycle
    *base = (uint) ((float)(*limit + 1) * (duty_cycle));
    *limit = *limit - *base;
}

#endif /* TIMERS_H_ */
