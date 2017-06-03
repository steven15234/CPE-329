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
uint32_t a_to_i(char* str, uint8_t len);
int main(void) {

   //initalize the UART with a read buffer of size 5
   UART0_init(SIZE, BAUD_19200);

   DAC_init();

   // Enable global interrupt
   __enable_irq();
   NVIC_SetPriority(EUSCIA0_IRQn, 4); /* set priority to 4 in NVIC */
   NVIC_EnableIRQ(EUSCIA0_IRQn);      /* enable interrupt in NVIC */


   char* buf;

   while (1) {

       if(UART_get_flag()){

           unsigned value = 0;
           buf = UART_get_buffer();
           value = a_to_i(buf, SIZE);
           value = (value > MAX_VALUE)? MAX_VALUE: value;
           DAC_drive(value);
           //P2->OUT = value;
           UART_reset_flag();
           UART_send(buf, TRUE);
       }
       delay_ms(10);
   }
}

void i_to_a(char* str, uint8_t len, uint32_t val){
    int i;
    for(i=1; i<=len; i++){
        str[len-i] = (char) ((val % 10UL) + '0');
        val/=10;
      }
      str[i-1] = '\0';
}




