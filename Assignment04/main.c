/*
 * main.c
 *
 *  Created on: Apr 20, 2017
 *      Author: Derek
 */

#include "msp.h"
#include "timers.h"

uint limit, base;

int main(void) {
    WDT_A->CTL = WDT_A_CTL_PW |  WDT_A_CTL_HOLD;          // Stop WDT

    set_DC0(FREQ_24_MHz);
    calcTimers(&limit, &base, 25, 0.25);
    // Configure GPIO
    P1->DIR |= BIT0;
    P1->OUT |= BIT0;

    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
    TIMER_A0->CCR[0] = base;
    TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | // SMCLK, continuous mode
            TIMER_A_CTL_MC__CONTINUOUS;

    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;    // Enable sleep on exit from ISR

    // Enable global interrupt
    __enable_irq();
    NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);

    while (1){
        __sleep();
        __no_operation();                   // For debugger
    }
}


// Timer A0 interrupt service routine
void TA0_0_IRQHandler(void) {
   TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    if(BIT0 & P1->OUT){
        TIMER_A0->CCR[0] += limit;
        P1->OUT &= ~BIT0;
    }
    else{
        TIMER_A0->CCR[0] += base;
        P1->OUT |= BIT0;
    }
}
