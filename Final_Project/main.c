#include "msp.h"
#include "clocks.h"
#include "delay.h"
#include "UART.h"
#include <stdint.h>
#include <string.h>

#define FALSE 0
#define TRUE 1

#define DIRTY_VAL 1234567
#define BUFFER 35

#define G   7653
#define A   6818
#define B   6073
#define C   5736
#define D   5111
#define E   4552
#define F   4298


void set_notes(unsigned int* distances, int len);
void print_graphs(unsigned int* values, int len, int range);
float get_distance(uint16_t trig_bit, uint16_t echo_bit);
int ipow10(int n);


char V_REPORT[BUFFER + 15];


unsigned int distances[3] = {0,0,0};
unsigned int prev[3] = {0,0,0};

int main(void) {

   set_HS_CLKS(); //SMCLK = 3MHz, MCLK = HSMCLK = 48MHz
   WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer


   P2->SEL1 &= ~(BIT0 | BIT1 | BIT2);         /* configure P2.0 as simple I/O */
   P2->SEL0 &= ~(BIT0 | BIT1 | BIT2);
   P2->DIR |= BIT0 | BIT1 | BIT2;           /* P2.0 set as output pin */
   P2->OUT &= ~(0x7);

   //configure P1.4 as mute button
   P1->SEL1 &= ~BIT4;
   P1->SEL0 &= ~BIT4;
   P1->DIR &= ~BIT4;
   P1->REN |= BIT4;
   P1->OUT |= BIT4;

   //configure P5.0-2 as outpins
   P5->SEL0 &= ~(BIT0 | BIT1 | BIT2);
   P5->SEL1 &= ~(BIT0 | BIT1 | BIT2);
   P5->DIR |= BIT0 | BIT1 | BIT2;
   P5->OUT &= ~(BIT0 | BIT1 | BIT2);

   //configure 3.5-7 as input pins
   P3->SEL0 &= ~(BIT5 | BIT6 | BIT7);
   P3->SEL1 &= ~(BIT5 | BIT6 | BIT7);
   P3->DIR &= ~(BIT5 | BIT6 | BIT7);


   //P2.4 as TA0.1 output
   P2->SEL0 |= BIT4;
   P2->SEL1 &= ~BIT4;
   P2->DIR |= BIT4;

   //P5.6 as TA2.1 output
   P5->SEL0 |= BIT6;
   P5->SEL1 &= ~BIT6;
   P5->DIR |= BIT6;

   //P7.6 as TA1.2 output
   P7->SEL0 |= BIT6;
   P7->SEL1 &= ~BIT6;
   P7->DIR |= BIT6;

   //echo pin setup
   P3->SEL0 &= ~(BIT5 | BIT6 | BIT7);
   P3->SEL1 &= ~(BIT5 | BIT6 | BIT7);  //standard IO

   UART0_init(0, BAUD_19200);

   // Timer32 set up in free-running mode, 32-bit, /16 pre-scale for a 3MHz clock
   TIMER32_1->CONTROL = TIMER32_CONTROL_SIZE | TIMER32_CONTROL_PRESCALE_1;
   TIMER32_1->LOAD= 0xFFFFFFFE;
   //start timer
   TIMER32_1->CONTROL |= TIMER32_CONTROL_ENABLE;


   //All timers are default G note at beginning
   //Timer_A0.1 setup
   //SMCLK in up mode with /1 divider for a 3MHz signal
   TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_MC__UP
           | TIMER_A_CTL_ID__1|  TIMER_A_CTL_CLR;
   TIMER_A0->CCR[0] = G;
   TIMER_A0->CCTL[1] = TIMER_A_CCTLN_OUTMOD_7; /* CCR1 reset/set */
   TIMER_A0->CCR[1] = G /2;

   //Timer_A2.1 setup
   //SMCLK in up mode with /1 divider for a 3MHz signal
   TIMER_A2->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_MC__UP
           | TIMER_A_CTL_ID__1|  TIMER_A_CTL_CLR;
   TIMER_A2->CCR[0] = G;
   TIMER_A2->CCTL[1] = TIMER_A_CCTLN_OUTMOD_7; /* CCR1 reset/set */
   TIMER_A2->CCR[1] = G /2;

   //Timer_A1.2 setup
   //SMCLK in up mode with /1 divider for a 3MHz signal
   TIMER_A1->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_MC__UP
           | TIMER_A_CTL_ID__1|  TIMER_A_CTL_CLR;
   TIMER_A1->CCR[0] = G;
   TIMER_A1->CCTL[2] = TIMER_A_CCTLN_OUTMOD_7; /* CCR2 reset/set */
   TIMER_A1->CCR[2] = G /2;


   UART_send_VT100("[?25l"); //turn off cursor blink
   UART_send_VT100("[2J"); //clear screen

   int button, i;

   while(1){

   button = P1->IN & BIT4;
   if(!button){
       P2->DIR ^= BIT4;
       P5->DIR ^= BIT6;
       P7->DIR ^= BIT6;
       P2->OUT ^= BIT0;
       delay_ms(500);
   }

   distances[0] = get_distance(BIT0, BIT5);
   distances[1] = get_distance(BIT1, BIT6);
   distances[2] = get_distance(BIT2, BIT7);

   set_notes(distances, 3);
   print_graphs(distances, 3, BUFFER);

   }


}

void set_notes(unsigned int* dist, int len){
    int note[3], i;
    for(i = 0; i < len; i++){
        if(dist[i] == DIRTY_VAL)
            continue;
        if(dist[i] < 15){
            if(dist[i] < 5)
                note[i] = 1;
            else if(dist[i] < 10)
                note[i] = G;
            else
                note[i] = A;
        }
        else if(dist[i] < 50){
            if(dist[i] < 20)
                note[i] = B;
            else if(dist[i] < 25)
                note[i] = C;
            else if(dist[i] < 30)
                note[i] = D;
            else if(dist[i] < 35)
                note[i] = E;
            else F;
        }
        else
            note[i] = 1;
    }
    TIMER_A0->CCR[1] = note[0] /2;
    TIMER_A0->CCR[0] = note[0];
    TIMER_A2->CCR[1] = note[1] /2;
    TIMER_A2->CCR[0] = note[1];
    TIMER_A1->CCR[2] = note[2] /2;
    TIMER_A1->CCR[0] = note[2];
}

void print_graphs(unsigned int* values, int len, int range){
    UART_send_VT100("[H"); //set cursor home
    UART_send("Final Project:\n\rDerek Nola and Bevin Tang\r\n");
    int i,j;
    for(j = 0; j < len; j++) {
        for(i = 1; i <= range; i++){
            if(i%5 == 0)
                V_REPORT[i - 1] = '|';
            else if(values[j] > i)
                V_REPORT[i - 1] = '-';
            else if(values[j] == i)
                V_REPORT[i - 1] = '-';
            else{
                V_REPORT[i - 1] = ' ';
            }
        }
        V_REPORT[i - 1] = 0;
        UART_send(V_REPORT);
        sprintf(V_REPORT, " sensor %d: %c[0K %2d \r\n\n", j, 27,values[j]);
        UART_send(V_REPORT);
    }

    sprintf(V_REPORT, "cm:%c[6C10%c[8C20%c[8C30\n\r", 27, 27, 27);
    UART_send(V_REPORT);

    sprintf(V_REPORT, "N:%c[2CG%c[4CA%c[4CB%c[4CC%c[4CD%c[4CE%c[4CF",
      27,27,27,27,27,27,27);
    UART_send(V_REPORT);

}

float get_distance(uint16_t trig_bit, uint16_t echo_bit){
    int start = 0, end = 0, j;
    float diff = 0;
    P5->OUT |= trig_bit; //sent trig pulse
    for (j = 50; j; j--); //10us delay
    P5->OUT &= ~trig_bit; //turn off trig pulse
    while(0 == (P3->IN & echo_bit));
    start = TIMER32_1->VALUE;
    while(echo_bit == (P3->IN & echo_bit));
    end = TIMER32_1->VALUE;
    diff = start - end;
    if(diff > 100000)
        return DIRTY_VAL; //indicate bad sensor readings
    if(diff > 3500)
        return diff/190.0f;
    if(diff > 1700)
        return diff/180.0f;
    else if(diff > 1300)
        return diff/190.0f;
    else
        return diff/205.0f; //correct scale to cm

}


//Fast 10^n lookup
int ipow10(int n){

    static int LUT_pow10[10] = {
        1, 10, 100, 1000, 10000,
        100000, 1000000, 10000000, 100000000, 1000000000
    };

    return LUT_pow10[n];
}

