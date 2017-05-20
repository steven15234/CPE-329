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
#define MAX_VOLT 3300

void i_to_a(char* str, uint8_t len, uint32_t val);
void DC_setup_timer();
void AC_setup_timer();
void DC_state_machine(void);
void AC_state_machine(void);

void calc_new_sample_limits(uint32_t* times, uint32_t* threshold, uint32_t*, float freq);

int DC_TIMER = 0;
int AC_TIMER = 0;
int DC_STATE, AC_STATE, AC_FREQ_STATE;

char V_REPORT[UART_SIZE];
int inital = 1;


int main(void) {

   set_HS_CLKS();
   WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

   UART0_init(UART_SIZE, BAUD_19200);
   keypad_init();
   ADC14_init();

   P1->SEL1 &= ~BIT0;         /* configure P2.0 as simple I/O */
   P1->SEL0 &= ~BIT0;
   P1->DIR |= BIT0;           /* P2.0 set as output pin */

   P2->SEL1 &= ~BIT0 | BIT1 | BIT2;         /* configure P2.0 as simple I/O */
   P2->SEL0 &= ~BIT0 | BIT1 | BIT2;
   P2->DIR |= BIT0 | BIT1 | BIT2;           /* P2.0 set as output pin */

   P5->SEL1 |= BIT4;                       // Configure P5.4 for ADC A1
   P5->SEL0 |= BIT4;

   // Timer32 set up in free-running mode, 32-bit, no pre-scale
   TIMER32_1->CONTROL = TIMER32_CONTROL_SIZE;
   TIMER32_1->LOAD= 0xFFFFFFFE;
   //start timer
   TIMER32_1->CONTROL |= TIMER32_CONTROL_ENABLE;

   //Timer_A setup
   DC_setup_timer();
   AC_setup_timer();

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
   UART_send_VT100("[?25l");
   while (1) {
       GKEY = num_to_char(keypad_getkey());
       //main switch statement changes
       //global values based off keypress
       switch(GKEY){
       case '1':
           MODE = DC;
           inital = 1;
           DC_STATE = START;
           TIMER_A0->CCR[0] = TEN_KHZ; // start DC timer
           TIMER_A1->CCR[0] = 0; //stop AC timer
           break;
       case '4':
           MODE = AC;
           inital = 1;
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


uint32_t DC_collect_state_machine(){
    uint32_t avg = 0;
    int temp;
    int done = 0;
    DC_TIMER = 0;
    int i;
    i = 0;
    while(!done){
        //take 100 samples, average them to avg
        //take 100 averages, average them to havg
        if(i < DC_SIZE && DC_TIMER){
            __disable_irq();
            P2->OUT ^= BIT0; /*toggle red LED */
            DC_TIMER = 0;

            temp = ADC14_get_value();
            avg += temp;
            ADC14_reset_flag();
            i++;
            __enable_irq();
        }
        if(i == DC_SIZE){
            //correcting voltage readings
            avg /= DC_SIZE;
            if(avg < 1500)
                avg -= (avg /110);
            else
                avg -= (avg /120);
            done = 1;
        }
        ADC14_start_sample();
    }
    return avg;
}

void DC_state_machine(void){
    static uint32_t volts = 0;
    int i;
    int scale;
    if(START == DC_STATE){
        volts = 0;
        DC_STATE = COLLECT;
        i = 0;
    }
    //collect DC samples
    if(COLLECT == DC_STATE){
        volts = 0;
        volts = DC_collect_state_machine();
        if(volts != 0)
            DC_STATE = REPORT;
    }
    //report the DC readings with VT100 over serial
    if(REPORT == DC_STATE){
        if(inital){
            UART_send_VT100("[2J");
        }
        UART_send_VT100("[H");
        sprintf(V_REPORT, "DC: %.3f V", volts / 1000.0f);
        UART_send(V_REPORT, TRUE);
        scale = ((float) volts / MAX_VOLT * 66);
        for(i = 0; i < scale - 1; i++)
            UART_send_char('-');
        UART_send_char('|');
        for(i = 0; i < 65 - scale; i++)
            UART_send_char(' ');
        UART_send_char('|');
        if(inital){
            UART_send(" ", TRUE);
            UART_send_VT100("[19C");
            UART_send("1V", FALSE);
            UART_send_VT100("[18C");
            UART_send("2V", FALSE);
            UART_send_VT100("[19C");
            UART_send("3V", TRUE);
            inital = 0;
        }
        DC_STATE = COLLECT;
        delay_ms(500);
    }

}

void AC_setup_timer(){
    TIMER_A1->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_ID__1 /* SMCLK, /1 divider*/
                   | TIMER_A_CTL_MC__UP; /* Count UP*/
    TIMER_A1->EX0 = TIMER_A_EX0_IDEX__1; /*ensures no further division */
    TIMER_A1->CCTL[0] = TIMER_A_CCTLN_CCIE; /* Interrupt for CCR[0] */
    TIMER_A1->CCR[0] = TEN_KHZ; /*max value produces a 10KHz interrupt*/
}


void AC_setup_calc_timer(){
    TIMER_A1->CCR[0] = 0; //stop AC timer
    TIMER_A1->R = 0;
    TIMER_A1->CCR[0] = TEN_KHZ; //start AC timer with new value
}

void AC_setup_collect_timer(float freq, uint32_t samples){
    uint16_t count = SMCLK_FREQ / (uint16_t) freq / samples;
    TIMER_A1->CCR[0] = 0; //stop AC timer
    TIMER_A1->R = 0;
    TIMER_A1->CCR[0] = count; //start AC timer with new value
}

void AC_limits_state_machine(uint32_t* havg, uint32_t* hmax, uint32_t* hmin){
    uint32_t avg = 0;
    uint32_t max = 0;
    uint32_t min = 10000;
    int temp;
    int outliers = 0;
    int done = 0;
    AC_TIMER = 0;
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
            if(max < temp){
                max = temp;
            }
            if(min > temp && temp != 0){
                min = temp;
            }
            ADC14_reset_flag();
            i++;
            __enable_irq();
        }
        if(i == AC_SIZE){
            avg /= AC_SIZE;
            //correcting voltage readings
            if(avg < 1500)
                avg -= (avg /110);
            else
                avg -= (avg /135);
            (*havg) += avg;

            if(j != 0){
                temp = (*hmin)/j;
                if(abs(min - temp) > temp/10)
                    outliers++;
                else
                    (*hmin) += min;
            }
            else
                (*hmin) += min;

            (*hmax) += max;

            avg= 0;
            i = max = 0;
            min = 10000;
            j++;
        }
        if(j == AC_SIZE){
            done = 1;
        }
        ADC14_start_sample();
    }

    (*havg) /= AC_SIZE;
    (*hmax) /= AC_SIZE;
    (*hmin) /= (AC_SIZE - outliers);
}

void AC_collect_state_machine(uint32_t* tRMS, uint32_t* cRMS, uint32_t dc, uint32_t samples){
    uint32_t i = 0;
    uint32_t done = 0;
    uint32_t rmsAC = 0;
    uint32_t rmsDC = 0;
    uint32_t temp = 0;
    AC_TIMER = 0;
    while(!done){
        if(i < samples  && AC_TIMER){
            __disable_irq();
            AC_TIMER = 0;
            temp = ADC14_get_value();
            rmsAC += temp * temp;
            rmsDC += (temp - dc) * (temp - dc);
            ADC14_reset_flag();
            i++;
            __enable_irq();
        }
        if(i == samples){
            done = 1;
        }
        ADC14_start_sample();
    }
    rmsAC /= samples;
    rmsDC /= samples;
    rmsAC = sqrt(rmsAC);
    rmsDC = sqrt(rmsDC);
    *tRMS = rmsAC;
    *cRMS = rmsDC;
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
    float temp;
    static uint32_t freqCount = 0;
    static uint32_t sampleTimes = 5;
    static uint32_t sampleThreshold = 10;
    static uint32_t sampleRMS = 50;
    static uint32_t max = 0;
    static uint32_t min = 10000;
    int scale = 0;
    static uint32_t avg = 0;
    if(START == AC_STATE){
        tRMS = cRMS = avg = max = min = 0;
        freq = 0.0f;
        freqCount = 0;
        AC_STATE = CALC_LIMITS;
        AC_FREQ_STATE = 0;
        AC_setup_calc_timer();
        ADC14_start_sample();
        P2->OUT = BIT0;
    }
    if(CALC_LIMITS == AC_STATE){
        //average from 100 averages of 100 samples (so 100000 total samples)
        AC_limits_state_machine(&avg, &max, &min);
        if(avg != 0 && max != 0){
            AC_STATE = CALC_FREQ;
            P2->OUT = BIT1;
            freq = 0;
        }
    }
    if(CALC_FREQ == AC_STATE){
        //Calculate frequency based off max and min voltage levels
       //take several values and average the result
       temp = AC_freq_state_machine(max, min, sampleThreshold);
       temp -= (temp/15);
       freq += temp;
       freqCount++;
        if(freqCount >= sampleTimes){
            freq /= sampleTimes;
            calc_new_sample_limits(&sampleTimes, &sampleThreshold, &sampleRMS, freq);
            AC_STATE = COLLECT;
            P2->OUT = BIT2;
        }
    }
    if(COLLECT == AC_STATE){
        //collect RMS voltage levels based of period timer
        AC_STATE = REPORT;
        AC_setup_collect_timer(freq, sampleRMS);
        AC_collect_state_machine(&tRMS, &cRMS, avg, sampleRMS);
        P2->OUT |= BIT1;

    }

    if(REPORT == AC_STATE){
        //report the AC values using VT100
        if(inital){
            UART_send_VT100("[H");
            UART_send_VT100("[2J");
        }
        else{
            UART_send_VT100("[H");
        }
        sprintf(V_REPORT, "DC: %.3f V", avg / 1000.0f);
        UART_send(V_REPORT, TRUE);

        //DC Bar Graph
        scale = ((float) avg / MAX_VOLT * 66);
        int i;
        for(i = 0; i < scale - 1; i++)
            UART_send_char('-');
        UART_send_char('|');
        for(i = 0; i < 65 - scale; i++)
            UART_send_char(' ');
        UART_send_char('|');

        //AC RMS
        UART_send(" ", TRUE);
        temp = (max - min)/1000.0f;
        sprintf(V_REPORT, "AC tRMS: %.3f V\tVPP: %.3f V", tRMS / 1000.0f, temp);
        UART_send(V_REPORT, FALSE);
        UART_send_VT100("[0K");
        freq = (freq > 10000)? 1.0f: freq;
        sprintf(V_REPORT, "\tFreq: %.1f Hz", freq);
        UART_send(V_REPORT, TRUE);

        sprintf(V_REPORT, "AC cRMS: %.3f V", cRMS / 1000.0f);
        UART_send(V_REPORT, TRUE);

        //AC Bar Graph
        scale = ((float) avg / MAX_VOLT * 66);
        for(i = 0; i < scale - 1; i++)
            UART_send_char('-');
        UART_send_char('|');
        for(i = 0; i < 65 - scale; i++)
            UART_send_char(' ');
        UART_send_char('|');

        //Bar graph scale
        if(inital){
           UART_send(" ", TRUE);
           UART_send_VT100("[19C");
           UART_send("1V", FALSE);
           UART_send_VT100("[18C");
           UART_send("2V", FALSE);
           UART_send_VT100("[19C");
           UART_send("3V", TRUE);
           inital = 0;
        }
        //restart state machine
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

//adjust sampling variables based of frequency
void calc_new_sample_limits(uint32_t* times, uint32_t* threshold, uint32_t* rms_samples, float freq){
    if(freq < 10){
        *times = 2;
        *threshold = 5;
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
        *threshold = 5;
        *rms_samples = 20;
    }
    else{
        *times = 20;
        *threshold = 10;
        *rms_samples = 10;
    }
}



