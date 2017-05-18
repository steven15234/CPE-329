/* www.MicroDigitalEd.com
 * p4_1.c UART0 transmit
 *
 */

#include "msp.h"
#include "UART.h"
#include "clocks.h"
#include "DAC.h"
#include "ADC14.h"
#include "timers.h"
#include <math.h>
#include "keypad.h"

#define FALSE 0
#define TRUE 1

#define DC 0x45
#define AC 0x46

#define TEN_KHZ 149


#define START 0
#define CALC_LIMITS 1
#define CALC_FREQ 2
#define COLLECT 3
#define REPORT 4

#define UART_SIZE 50
#define AC_SIZE 100
#define DC_SIZE 100

void i_to_a(char* str, uint8_t len, uint32_t val);
void DC_setup_timer();
void AC_setup_calc_timer();
void DC_state_machine(void);
void AC_state_machine(void);

void calc_new_sample_limits(uint32_t* times, uint32_t* threshold, uint32_t*, float freq);

int DC_TIMER = 0;
int AC_TIMER = 0;
int DC_STATE, AC_STATE, AC_FREQ_STATE;

char V_REPORT[5];
int AC_ARR[AC_SIZE];

int main(void) {

   set_HS_CLKS();
   WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

   UART0_init(UART_SIZE, BAUD_19200);

   ADC14_limits_init(0xFFFF, 0xFFFF);

   P1->SEL1 &= ~BIT0;         /* configure P2.0 as simple I/O */
   P1->SEL0 &= ~BIT0;
   P1->DIR |= BIT0;           /* P2.0 set as output pin */

   P2->SEL1 &= ~BIT0 | BIT1 | BIT2;         /* configure P2.0 as simple I/O */
   P2->SEL0 &= ~BIT0 | BIT1 | BIT2;
   P2->DIR |= BIT0 | BIT1 | BIT2;           /* P2.0 set as output pin */


   // Timer32 set up in free-running mode, 32-bit, no pre-scale
   TIMER32_1->CONTROL = TIMER32_CONTROL_SIZE;
   TIMER32_1->LOAD= 0xFFFFFFFE;
   //start timer
   TIMER32_1->CONTROL |= TIMER32_CONTROL_ENABLE;

   //Timer_A setup
   DC_setup_timer();
   AC_setup_calc_timer();

   // Enable global interrupt
   NVIC_SetPriority(EUSCIA0_IRQn, 4); /* UART set priority to 4 */
   NVIC_EnableIRQ(EUSCIA0_IRQn);      /* enable UART interrupt in NVIC */
   NVIC_EnableIRQ(TA0_0_IRQn); /*enable TimerA interrupt*/
   NVIC_EnableIRQ(TA1_0_IRQn); /*enable TimerA interrupt*/
   NVIC_EnableIRQ(ADC14_IRQn); /*enable ADC14 interrupt*/
   __enable_irq();

   //State setup
   char GKEY = '1';
   int MODE = AC;
   AC_STATE = DC_STATE = START;

   UART_send("start", TRUE);
   while (1) {
       GKEY = '3';//num_to_char(keypad_getkey());
       //main switch statement changes
       //global values based off keypress
       switch(GKEY){
       case '1':
           MODE = DC;
           DC_STATE = START;
           TIMER_A0->CCR[0] = TEN_KHZ; // start DC timer
           TIMER_A1->CCR[0] = 0; //stop AC timer
           break;
       case '2':
           MODE = AC;
           AC_STATE = START;
           TIMER_A1->CCR[0] = TEN_KHZ; // start AC timer
           TIMER_A0->CCR[0] = 0; //stop DC timer

           break;
       }
       if(MODE == DC){
           DC_state_machine();
       }
       else{
           AC_state_machine();
       }

   }
}

void DC_setup_timer(){
    TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_ID__1 /* SMCLK, /1 divider*/
               | TIMER_A_CTL_MC__UP; /* Count UP*/
    TIMER_A0->EX0 = TIMER_A_EX0_IDEX__1; /*ensures no further division */
    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; /* Interrupt for CCR[0] */
    TIMER_A0->CCR[0] = TEN_KHZ; /*max value produces a 10KHz interrupt*/
}

void DC_state_machine(void){
    static uint32_t volts = 0;
    int i;
    if(START == DC_STATE){
        volts = 0;
        DC_STATE = COLLECT;
        i = 0;
    }
    if(COLLECT == DC_STATE){
        if(i < DC_SIZE && DC_TIMER){
            __disable_irq();
            P2->OUT ^= BIT0; /*toggle red LED */
            DC_TIMER = 0;
            volts += ADC14_get_value();
            ADC14_reset_flag();
            i++;
            __enable_irq();
        }
        if(i == DC_SIZE){
            i = 0;
            volts /= DC_SIZE;
            if(volts > 1500)
                volts -= (volts /55);
            else
                volts -= (volts /50);
            DC_STATE = REPORT;
        }
        ADC14_start_sample();
    }
    if(REPORT == DC_STATE){
        i_to_a(V_REPORT, 4, volts);
        UART_send(V_REPORT, TRUE);
        DC_STATE = COLLECT;

    }

}

//ACJLNADKVJBLDIHV PID
void AC_setup_calc_timer(){
    TIMER_A1->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_ID__1 /* SMCLK, /1 divider*/
                   | TIMER_A_CTL_MC__UP; /* Count UP*/
    TIMER_A1->EX0 = TIMER_A_EX0_IDEX__1; /*ensures no further division */
    TIMER_A1->CCTL[0] = TIMER_A_CCTLN_CCIE; /* Interrupt for CCR[0] */
    TIMER_A1->CCR[0] = TEN_KHZ; /*max value produces a 10KHz interrupt*/
}

void AC_setup_collect_timer(float freq, uint32_t samples){
    uint16_t count = SMCLK_FREQ / (uint16_t) freq / samples;
    TIMER_A1->CCR[0] = 0; //stop AC timer
    TIMER_A1->R = 0;
    TIMER_A1->CCR[0] = count; //start AC timer with new value
}

void AC_limits_state_machine(uint32_t* havg, uint32_t* max, uint32_t* min){
    uint32_t avg = 0;
    int temp;
    int done = 0;

    int i, j;
    i = j = 0;
    while(!done){
        //take 100 samples, average them to avg
        //take 100 averages, average them to havg
        if(i < AC_SIZE && AC_TIMER){
            __disable_irq();
            P2->OUT ^= BIT0; /*toggle red LED */
            AC_TIMER = 0;

            temp = ADC14_get_value();
            avg += temp;
            if(*max < temp){
                *max = temp;
            }
            if(*min > temp && temp != 0){
                *min = temp;
            }
            ADC14_reset_flag();
            i++;
            __enable_irq();
        }
        if(i == AC_SIZE){
            i = 0;
            avg /= AC_SIZE;
            //correcting voltage readings
            if(avg < 1500)
                avg -= (avg /110);
            else
                avg -= (avg /135);
            (*havg) += avg;
            avg= 0;
            j++;
        }
        if(j == AC_SIZE){
            done = 1;
        }
        ADC14_start_sample();
    }
    (*havg) /= AC_SIZE;
}

float AC_freq_state_machine(uint32_t max, uint32_t min, uint32_t threshold){
    uint32_t upper_limit = (max - max/threshold);
    uint32_t lower_limit = (min + min/threshold);
    uint32_t time1, time2;
    time1 = time2 = 0;
    int done = 0;
    int state = 0;
    uint32_t temp = 0;
    ADC14_start_sample();
    while(!done){
         if(state == 0 && ADC14_get_flag()){
             temp = ADC14_get_value();
             if(temp > upper_limit){
                 time1 = TIMER32_1->VALUE;
                 state = 1;
             }
             ADC14_reset_flag();
             ADC14_start_sample();
         }
         if(state == 1 && ADC14_get_flag()){
             temp = ADC14_get_value();
             if(temp < lower_limit){
                 state = 2;
             }
             ADC14_reset_flag();
             ADC14_start_sample();
         }
         if(state == 2 && ADC14_get_flag()){
             temp = ADC14_get_value();
             if(temp > upper_limit){
                 time2 = TIMER32_1->VALUE;
                 state = 3;
             }
             ADC14_reset_flag();
             ADC14_start_sample();
         }
         if(state == 3){
             if (time1 < time2){
                 state = 0;
                 ADC14_reset_flag();
                 ADC14_start_sample();
             }
             else{
                 temp = time1 - time2;
                 done = 1;
             }
         }
    }
    float total = MCLK_FREQ / temp;
    if(total < 1.0f)
        total = 1.0f;
    return total;
}

void AC_state_machine(void){
    uint32_t tRMS = 0;
    uint32_t cRMS = 0;
    static float freq = 0.0f;
    static uint32_t freqCount = 0;
    static uint32_t sampleTimes = 3;
    static uint32_t sampleThreshold = 10;
    static uint32_t sampleRMS = 50;
    static uint32_t max = 0;
    static uint32_t min = 10000;
    static uint32_t rms = 0;
    static uint32_t avg = 0;
    if(START == AC_STATE){
        tRMS = avg = max = rms = 0;
        min = 10000;
        freq = 0.0f;
        freqCount = 0;
        AC_STATE = CALC_LIMITS;
        AC_FREQ_STATE = 0;
        ADC14_start_sample();
        P2->OUT = BIT0;
    }
    if(CALC_LIMITS == AC_STATE){
        AC_limits_state_machine(&avg, &max, &min);
        if(avg != 0 && max != 0){
            AC_STATE = CALC_FREQ;
            P2->OUT = BIT1;
        }
    }
    if(CALC_FREQ == AC_STATE){
        freq += AC_freq_state_machine(max, min, sampleThreshold);
        freqCount++;
        if(freqCount == sampleTimes){
            freq /= sampleTimes;
            calc_new_sample_limits(&sampleTimes, &sampleThreshold, &sampleRMS, freq);
            AC_STATE = COLLECT;
            P2->OUT = BIT2;
        }
    }
    if(COLLECT == AC_STATE){
        AC_STATE = REPORT;
        P2->OUT = 0;

    }
    if(REPORT == AC_STATE){
        AC_STATE = START;
    }

}

void TA0_0_IRQHandler(void) {
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; /*clear interrupt flag*/
    DC_TIMER = 1;
}

void TA1_0_IRQHandler(void) {
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; /*clear interrupt flag*/
    AC_TIMER = 1;
}

void i_to_a(char* str, uint8_t len, uint32_t val){
    int i;
    for(i=1; i<=len; i++){
        str[len-i] = (char) ((val % 10UL) + '0');
        val/=10;
      }
      str[i-1] = '\0';
}

void calc_new_sample_limits(uint32_t* times, uint32_t* threshold, uint32_t* rms_samples, float freq){
    if(freq < 10){
        *times = 2;
        *threshold = 20;
        *rms_samples = 100;
        return;
    }
    if(freq < 100){
        *times = 3;
        *threshold = 10;
        *rms_samples = 50;
        return;
    }
    if(freq < 500){
        *times = 5;
        *threshold = 10;
        *rms_samples = 20;
    }
    else{
        *times = 10;
        *threshold = 5;
        *rms_samples = 10;
    }
}



