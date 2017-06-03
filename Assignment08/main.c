/* www.MicroDigitalEd.com
 * p4_1.c UART0 transmit
 *
 */

#include "msp.h"
#include "UART.h"
#include "DAC.h"
#include "ADC14.h"

#define FALSE 0w
#define TRUE 1


#define SIZE 5

void i_to_a(char* str, uint8_t len, uint32_t val);

int main(void) {

   //Initialize the UART with a read buffer of size 5
   UART0_init(SIZE, BAUD_19200);

   DAC_init();


   P5->SEL1 |= BIT4;                       // Configure P5.4 for ADC A1
   P5->SEL0 |= BIT4;

   // Enable global interrupt
   __enable_irq();
   NVIC_SetPriority(EUSCIA0_IRQn, 4); /* set priority to 4 in NVIC */
   NVIC_EnableIRQ(EUSCIA0_IRQn);      /* enable UART interrupt in NVIC */

   NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31); /*enable ADC14 interrupt*/


   // Sampling time, S&H=16, ADC14 on
   ADC14->CTL0 = ADC14_CTL0_SHT0_2 | ADC14_CTL0_SHP | ADC14_CTL0_ON;
   ADC14->CTL1 = ADC14_CTL1_RES__14BIT;         // Use sampling timer, 12-bit conversion results



   char buf[SIZE];
   uint32_t temp;
   // Start sampling/conversion first time
   ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;

   while (1) {

       if(ADC14_get_flag()){
           temp = ADC14_get_value() / 5;
           if(temp > 1000)
               i_to_a(buf, SIZE - 1, temp);
           else
               i_to_a(buf, SIZE - 2, temp);
           UART_send(buf, TRUE);
           ADC14_reset_flag();
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




