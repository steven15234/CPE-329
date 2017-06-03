#include "msp.h"
#include "clocks.h"
#include "keypad.h"
#include <stdint.h>
#include <string.h>

#define FALSE 0
#define TRUE 1

#define FIFTY_HZ 60000

uint32_t a_to_i(char* str);
int ipow10(int n);
void getCode(char* code);
uint16_t servo_pwm_angle(uint16_t degrees);

int16_t current_angle, temp;
uint16_t pwm;
char GKEY[3];

int main(void) {

   set_HS_CLKS(); //SMCLK = 3MHz, MCLK = HSMCLK = 48MHz
   WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

   keypad_init();

   P2->SEL1 &= ~(BIT0 | BIT1 | BIT2);         /* configure P2.0 as simple I/O */
   P2->SEL0 &= ~(BIT0 | BIT1 | BIT2);
   P2->DIR |= BIT0 | BIT1 | BIT2;           /* P2.0 set as output pin */
   P2->OUT &= ~(0x7);

   //P5.6 as TA2.1 output
   P5->SEL0 |= BIT6;
   P5->SEL1 &= ~BIT6;
   P5->DIR |= BIT6;

   temp = current_angle = 90; //default 90 degrees;
   pwm = servo_pwm_angle(current_angle);

   //Timer_A2.1 setup
   //SMCLK in up mode with /1 divider for a 3MHz signal
   TIMER_A2->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_MC__UP
           | TIMER_A_CTL_ID__1|  TIMER_A_CTL_CLR;
   TIMER_A2->CCR[0] = FIFTY_HZ; /*max value produces a 50Hz interrupt*/
   TIMER_A2->CCTL[1] = TIMER_A_CCTLN_OUTMOD_7; /* CCR1 reset/set */
   TIMER_A2->CCR[1] = pwm; //default 90 degrees;

   //variable init
   GKEY[0] = GKEY[1] = 'E';
   GKEY[2] = 0;

   while (1) {
       getCode(GKEY);
       //angle changed based of XX code
       if(GKEY[0] == '*'){
           //prevent going below 0 angle
           current_angle = (current_angle > 0)? current_angle - 10: 0;
       }
       else if(GKEY[0] == '#')
           //prevent going above 180 angle
           current_angle = (current_angle < 180)? current_angle + 10: 180;
       else if(GKEY[0] != 'E'){
           current_angle = 10 * a_to_i(GKEY);
           current_angle = (current_angle > 180)? 180: current_angle;
       }
       if(current_angle != temp){
           temp = current_angle;
           pwm = servo_pwm_angle(current_angle);
           TIMER_A2->CCR[1] = pwm;//update timer value
       }

   }

}

void getCode(char* code){
  code[0] = code[1] = 'E';
  code[0] = num_to_char(keypad_getkey());
  //cycle until key is pressed
  while(code[0] == 'E'){
      code[0] = num_to_char(keypad_getkey());
  }
  if(code[0] == '*' || code[0] == '#'){
    code[1] = 'E';
    delay_ms(250);
    return;
  }
  //debounce delay
  delay_ms(250);

  code[1] = num_to_char(keypad_getkey());
  while(code[1] == 'E'){
      code[1] = num_to_char(keypad_getkey());
  }
  if(code[1] == '*' || code[1] == '#'){
    code[0] = code[1];
    code[1] = 'E';
  }
  //debounce delay
  delay_ms(250);
}


uint16_t servo_pwm_angle(uint16_t degrees){
    const uint16_t scale = (3*FIFTY_HZ)/(20*4);
    return scale*degrees/90 + scale;
}

//fast string to unsigned number
uint32_t a_to_i(char* str){
    int i, j, result;
    j = strlen(str) - 1;
    for(i = result = 0; i < strlen(str); i++){
       result +=  (str[i] - '0') * ipow10(j--);
    }
    return result;
}

//Fast 10^n lookup
int ipow10(int n){

    static int LUT_pow10[10] = {
        1, 10, 100, 1000, 10000,
        100000, 1000000, 10000000, 100000000, 1000000000
    };

    return LUT_pow10[n];
}
