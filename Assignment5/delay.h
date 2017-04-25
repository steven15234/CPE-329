/*
 * delay.h
 *
 *  Created on: Apr 6, 2017
 *      Author: Bevin Tang
 */

#ifndef DELAY_H_
#define DELAY_H_

#include <stdio.h>
#include <stdlib.h>

typedef unsigned int uint;

#define FREQ_1_5_MHz CS_CTL0_DCORSEL_0
#define FREQ_3_MHz CS_CTL0_DCORSEL_1
#define FREQ_6_MHz CS_CTL0_DCORSEL_2
#define FREQ_12_MHz CS_CTL0_DCORSEL_3
#define FREQ_24_MHz CS_CTL0_DCORSEL_4
#define FREQ_48_MHz CS_CTL0_DCORSEL_5

float GLOBAL_FREQ = 3.0f;

void set_DC0(uint freq){

    switch(freq){
    case FREQ_1_5_MHz:
        GLOBAL_FREQ = 1.5f;
         break;
    case FREQ_3_MHz:
        GLOBAL_FREQ = 3.0f;
        break;
    case FREQ_6_MHz:
        GLOBAL_FREQ = 6.0f;
        break;
    case FREQ_12_MHz:
        GLOBAL_FREQ = 12.0f;
        break;
    case FREQ_24_MHz:
        GLOBAL_FREQ = 24.0f;
        break;
    }
    if(freq > CS_CTL0_DCORSEL_5){
        fprintf(stderr, "Invalid Clock");
        exit(1);
    }

    // change DC0 from default of 3MHz
    CS->KEY = CS_KEY_VAL; // unlock CS registers
    CS->CTL0 = 0; // clear register CTL0
    CS->CTL0 = freq; // set DCO = freq
    CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3; // select clock sources
    CS->KEY = 0; // lock the CS registers

}

/* delay milliseconds when system clock is at 3 MHz for Rev C MCU */
void delay_ms(uint delay) {
    int i, j;
    for (j = 0; j < delay; j++)
        for (i = 100*(int)GLOBAL_FREQ; i > 0; i--);      /* Delay 1 ms*/
}

void delay_us(const int delay){
    int j;
    const int scale = delay*(int)GLOBAL_FREQ / 10;
    for (j = scale; j; j--);
}
#endif /* DELAY_H_ */
