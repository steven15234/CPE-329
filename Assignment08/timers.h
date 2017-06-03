/*
 * timers.h
 *
 *  Created on: Apr 20, 2017
 *      Author: Derek
 */

#ifndef TIMERS_H_
#define TIMERS_H_
#include <stdint.h>

#include "delay.h"

//Given delay in ms
uint32_t calcSimpleTimer(uint32_t delay){
    return ((uint32_t) GLOBAL_FREQ) * 1000000 * delay;
}
void calcAdvTimer(uint16_t* limit, uint16_t* base, uint32_t freq, float duty_cycle){
    if(freq == 100)
        freq = 200;
    duty_cycle = -1*(duty_cycle -1);
    //PWM Period
    *limit = (uint16_t) ((GLOBAL_FREQ * 1000000.0f/ freq) - 1.0f);
    //PWM Duty Cycle
    *base = (uint16_t) ((float)(*limit + 1) * (duty_cycle));
    *limit = *limit - *base;
}

#endif /* TIMERS_H_ */
