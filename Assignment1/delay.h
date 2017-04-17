/*
 * delay.h
 *
 *  Created on: Apr 6, 2017
 *      Author: Bevin Tang
 */

#ifndef DELAY_H_
#define DELAY_H_

typedef unsigned int uint;

#define FREQ_1_5_MHz CS_CTL0_DCORSEL_0
#define FREQ_3_MHz CS_CTL0_DCORSEL_1
#define FREQ_6_MHz CS_CTL0_DCORSEL_2
#define FREQ_12_MHz CS_CTL0_DCORSEL_3
#define FREQ_24_MHz CS_CTL0_DCORSEL_4
#define FREQ_48_MHz CS_CTL0_DCORSEL_5

void setDC0(uint freq){

    if(freq > CS_CTL0_DCORSEL_5){
        fprintf(stderr, "Invalid Clock");
        exit(1);
    }

    // change DC0 from default of 3MHz to 12MHz.
    CS->KEY = CS_KEY_VAL; // unlock CS registers
    CS->CTL0 = 0; // clear register CTL0
    CS->CTL0 = freq; // set DCO = 12 MHz
    CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3; // select clock sources
    CS->KEY = 0; // lock the CS registers

}

/* delay milliseconds when system clock is at 3 MHz for Rev C MCU */
void delay_ms(uint delay, uint freq) {
    int i, j;
    for (j = 0; j < delay; j++)
        for (i = 100*freq; i > 0; i--);      /* Delay 1 ms*/
}

int delayFunc(){
    return 1+1;
}
void delay_us(const int delay, const int freq){
    int i, j;
    const int scale = delay*freq / 10;
    for (j = scale; j; j--);
}
#endif /* DELAY_H_ */
