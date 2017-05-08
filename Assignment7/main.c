/* www.MicroDigitalEd.com
 * p4_1.c UART0 transmit
 *
 */

#include "msp.h"
#include "delay.h"
#include "UART.h"
#include "DAC.h"

#define FALSE 0
#define TRUE 1


#define SIZE 5
#define MAX_VALUE 4096

int pow10(int n);

int main(void) {

   //initalize the UART with a read buffer of size 5
   UART0_init(SIZE);


   /* initialize P2.2-P2.0 for tri-color LEDs */
   P2->SEL1 &= ~7;         /* configure P2.2-P2.0 as simple I/O */
   P2->SEL0 &= ~7;
   P2->DIR |= 7;           /* P2.2-2.0 set as output */

   DAC_init();

   // Enable global interrupt
   __enable_irq();
   NVIC_SetPriority(EUSCIA0_IRQn, 4); /* set priority to 4 in NVIC */
   NVIC_EnableIRQ(EUSCIA0_IRQn);      /* enable interrupt in NVIC */


   char* buf;

   while (1) {

       if(UART_get_flag()){
           int i, j;
           unsigned value = 0;
           buf = UART_get_buffer();
           j = SIZE - 2;
           for(i = 0; i < SIZE - 1; i++){
               value +=  (buf[i] - '0') * pow10(j--);
           }
           value = (value > MAX_VALUE)? MAX_VALUE: value;
           //DAC_drive(value);
           //P2->OUT = value;
           UART_reset_flag();
           UART_send(buf, TRUE);
       }
       delay_ms(10);
   }
}



int pow10(int n){

    static int LUT_pow10[10] = {
        1, 10, 100, 1000, 10000,
        100000, 1000000, 10000000, 100000000, 1000000000
    };

    return LUT_pow10[n];
}



