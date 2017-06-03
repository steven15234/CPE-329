/*
 * clocks.h
 *
 *  Created on: Apr 6, 2017
 *      Author: Bevin Tang
 */

#ifndef CLOCKS_H_
#define CLOCKS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "msp.h"

#define MCLK_FREQ 48000000
#define SMCLK_FREQ 3000000


#define FREQ_1_5_MHz CS_CTL0_DCORSEL_0
#define FREQ_3_MHz CS_CTL0_DCORSEL_1
#define FREQ_6_MHz CS_CTL0_DCORSEL_2
#define FREQ_12_MHz CS_CTL0_DCORSEL_3
#define FREQ_24_MHz CS_CTL0_DCORSEL_4
#define FREQ_48_MHz CS_CTL0_DCORSEL_5


inline void set48Mode(void){
    /* Step 1: Transition to VCORE Level 1: AM0_LDO --> AM1_LDO */
    while ((PCM->CTL1 & PCM_CTL1_PMR_BUSY));
     PCM->CTL0 = PCM_CTL0_KEY_VAL | PCM_CTL0_AMR_1;
    while ((PCM->CTL1 & PCM_CTL1_PMR_BUSY));

    /* Step 2: Configure Flash wait-state to 1 for both banks 0 & 1 */
    FLCTL->BANK0_RDCTL = (FLCTL->BANK0_RDCTL &
     ~(FLCTL_BANK0_RDCTL_WAIT_MASK)) | FLCTL_BANK0_RDCTL_WAIT_1;
    FLCTL->BANK1_RDCTL = (FLCTL->BANK0_RDCTL &
     ~(FLCTL_BANK1_RDCTL_WAIT_MASK)) | FLCTL_BANK1_RDCTL_WAIT_1;

}


inline void set_DC0(uint32_t freq){


    if(freq == FREQ_48_MHz)
        set48Mode();
    if(freq > CS_CTL0_DCORSEL_5){
        fprintf(stderr, "Invalid Clock");
        exit(1);
    }

    // change DC0 from default of 3MHz
    CS->KEY = CS_KEY_VAL; // unlock CS registers
    CS->CTL0 = 0; // clear register CTL0
    CS->CTL0 = freq; // set DCO = freq
    /* Select MCLK = DCO, no divider */
    CS->CTL1 &= ~(CS_CTL1_SELM_MASK | CS_CTL1_DIVM_MASK) | CS_CTL1_SELM_3;
    CS->KEY = 0; // lock the CS registers

}

inline void set_HS_CLKS(void){
    set48Mode(); /*set the high power for 48Mhz*/
    CS->KEY = CS_KEY_VAL ; // Unlock CS module for register access
    PJ->SEL0 |= BIT2 | BIT3;        // Configure PJ.2/3 for HFXT function
    PJ->SEL1 &= ~(BIT2 | BIT3);
    //enable HFXT to 48Mhz
    CS->CTL2 |= CS_CTL2_HFXT_EN | CS_CTL2_HFXTFREQ_6 | CS_CTL2_HFXTDRIVE;
    while(CS->IFG & CS_IFG_HFXTIFG)
        CS->CLRIFG |= CS_CLRIFG_CLR_HFXTIFG;
    /* Select MCLK & HSMCLK = HFXT, no divider */
    CS->CTL1 = CS->CTL1 & ~(CS_CTL1_SELM_MASK | CS_CTL1_DIVM_MASK |
               CS_CTL1_SELS_MASK | CS_CTL1_DIVHS_MASK) | CS_CTL1_SELM__HFXTCLK |
               CS_CTL1_SELS__HFXTCLK;
    CS->CTL1 |= CS_CTL1_DIVS__16;  /* Select SMCLK = HFXT, /16 divider*/
    CS->KEY = 0; // Lock CS module from unintended accesses
}

#endif /* CLOCKS_H_ */
