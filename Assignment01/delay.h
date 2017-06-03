/*
 * delay.h
 *
 *  Created on: Apr 6, 2017
 *      Author: Bevin Tang
 */

#ifndef DELAY_H_
#define DELAY_H_

#include <stdint.h>
#include "msp.h"
#include "Clocks.h"

/* delay milliseconds when system clock is at 48 MHz for Rev C MCU */
inline void delay_ms(uint16_t delay) {
    uint32_t i, j;
    for (j = 0; j < delay; j++)
        for (i = MCLK_FREQ/10000; i > 0; i--);      /* Delay 1 ms*/
}

inline void delay_us(const int delay){
    int j;
    const int scale = delay*MCLK_FREQ / 10;
    for (j = scale; j; j--);
}
#endif /* DELAY_H_ */
