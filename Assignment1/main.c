/* CPE 329
 *
 * delay.h
 *
 *  Created on: Apr 6, 2017
 *      Author: Derek Nola and Bevin Tang
 */

#include "msp.h"
#include <stdio.h>
#include <stdlib.h>
#include "delay.h"



int main(void) {
    uint freq = 24;
    setDC0(FREQ_24_MHz);
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

    P2->SEL1 &= ~1;         /* configure P2.0 as simple I/O */
    P2->SEL0 &= ~1;
    P2->DIR |= 1;           /* P2.0 set as output pin */

    P2->SEL1 &= ~4;         /* configure P2.2 as simple I/O */
    P2->SEL0 &= ~4;
    P2->DIR |= 4;           /* P2.2 set as output pin */

    while (1) {

        P2->OUT |= 1;       /* turn on P2.0 red LED */
        P2->OUT |= 4;       /* turn on P2.2 blue LED */
        delay_us(50, freq);

        P2->OUT &= ~1;      /* turn off P2.0 red LED */
        P2->OUT &= ~4;      /* turn off P2.2 blue LED */
        delay_us(50, freq);

    }
}


