
/*
 * main.c
 *
 *  Created on: Apr 20, 2017
 *      Author: Derek
 */

#include "msp.h"
#include "timers.h"

uint limitA, baseA, limitB, baseB;

int main(void) {
    WDT_A->CTL = WDT_A_CTL_PW |  WDT_A_CTL_HOLD;          // Stop WDT

    set_DC0(FREQ_1_5_MHz);

    calcTimers(&limitA, &baseA, 0.5, 0.5);
    calcTimers(&limitB, &baseB, 1, 0.5);
    limitA++;
    limitB++;
    // Configure P1.0 as Bit 1
    P1->DIR |= BIT0;
    P1->OUT &= ~BIT0;

    // Configure P2.0 as Bit 0
    P2->DIR |= BIT0;
    P2->OUT &= ~BIT0;

    //Setup the timers
    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
    TIMER_A0->CCTL[1] = TIMER_A_CCTLN_CCIE;

    TIMER_A0->CCR[0] = baseA;
    TIMER_A0->CCR[1] = baseB;

    TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | // SMCLK, continuous mode
            TIMER_A_CTL_MC__CONTINUOUS;

    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;    // Enable sleep on exit from ISR

    // Enable global interrupt
    __enable_irq();
    NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);
    NVIC->ISER[0] = 1 << ((TA0_N_IRQn) & 31);

    while (1){
        __sleep();
        __no_operation();                   // For debugger
    }
}


// Timer A0 interrupt service routine
void TA0_0_IRQHandler(void) {
   TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    if(BIT0 & P1->OUT){
        TIMER_A0->CCR[0] += limitA;
        P1->OUT &= ~BIT0;
    }
    else{
        TIMER_A0->CCR[0] += baseA;
        P1->OUT |= BIT0;
    }
}

void TA0_N_IRQHandler(void) {
   TIMER_A0->CCTL[1] &= ~TIMER_A_CCTLN_CCIFG;
    if(BIT0 & P2->OUT){
        TIMER_A0->CCR[1] += limitB;
        P2->OUT &= ~BIT0;
    }
    else{
        TIMER_A0->CCR[1] += baseB;
        P2->OUT |= BIT0;
    }

}
