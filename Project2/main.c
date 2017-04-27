/*
* Project 2
* Derek Nola and Bevin Tang
*/


#include "timers.h"
//#include "keypad.h"
//#include "msp430.h"

#include <stdlib.h>


#define SQR 0x0D
#define SINE 0x0E
#define SAW 0x0F

#define FIVEMS 0xDB23
#define SINE_STEPS 36
#define VOLT_SCALE 5120
//TEMP INCLUDES
#include <stdio.h>


//GLOBALS
char GKEY = 'E';
float duty_cycle = 0.5f;
uint32_t freq = 100;
int8_t wave_type = SQR;
volatile unsigned int TempDAC_Value = 0;

uint16_t SQRBase, SQRLimit;


int32_t isin(int32_t deg, int32_t scale);
void generateWave(void);

int main(int argc, char const *argv[])
{
   
   char* result = malloc(17);
   
   char* wave_name = malloc(5);
   wave_name = "SQR";

   //generate default 100 Hz square wave with 50% duty cycle
   calcAdvTimer(&SQRLimit, &SQRBase, freq, duty_cycle);
   sprintf(result,"Freq: %uHz, DC: %2.0f%%, %s \n", freq, duty_cycle*100, wave_name);
   printf("%s", result);
   while(GKEY != 'x'){
      printf("Enter Keypad Key:\n");
      scanf(" %c", &GKEY);
      switch(GKEY){
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
            freq = 100 * (GKEY - '0');
            break;
         case '7':
            wave_name = "SQR";
            wave_type = SQR;
            break;
         case '8':
            wave_name = "SINE";
            wave_type = SINE;
            break;
         case '9':
            wave_name = "SAW";
            wave_type = SAW;
         case '*':
            if(wave_type == SQR)
               duty_cycle = (duty_cycle - 0.1f >= 0.1f)? duty_cycle - 0.1f : 0.1f;
            break;
         case '0':
            if(wave_type == SQR)
               duty_cycle = 0.5f;
            break;
         case '#':
            if(wave_type == SQR)
               duty_cycle = (duty_cycle + 0.1f <= 0.91f)? duty_cycle + 0.1f : 0.9f;
            break;
      }
      generateWave();
      sprintf(result, "Freq: %uHz, DC: %2.0f%%, %s \n", freq, duty_cycle*100, wave_name);
      printf("%s", result);
   }

   return 0;
}


void generateWave(void){
   static uint16_t DAC_COUNT = 0;
   uint16_t DAC_INC = 360 / SINE_STEPS;
   
   if(wave_type == SQR){
      //TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
      if(!TempDAC_Value){
        // TIMER_A0->CCR[0] += SQRLimit;
         TempDAC_Value = VOLT_SCALE;
      } 
      else{
         //TIMER_A0->CCR[0] += SQRBase;
         TempDAC_Value = 0;
      }
      
      //Drive_DAC(TempDAC_Value);
   }
   else if(wave_type == SINE){
      //TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

      if(DAC_COUNT > 360)
         DAC_COUNT = 0;
      TempDAC_Value = isin(DAC_COUNT, VOLT_SCALE/2);
      //Drive_DAC(TempDAC_Value);
      DAC_COUNT+= DAC_INC;
      //TIMER_A0->CCR[0] += 0x0F00; //reset timer
      
   }
   else if(wave_type == SAW){

   }
   printf("%u %u\n", DAC_COUNT, TempDAC_Value);
}

// A sine approximation via Bhaskara I's approx.
// @param x    Angle (0 to 360 allowed) 
// @return     sine value*scale 
int32_t isin(int32_t deg, int32_t scale){
   int8_t shift = 1;
   if(deg > 180){
      deg -= 180;
      shift = -1;
   }
   int32_t num = (deg << 2)*(180 - deg);
   float dem = 40500 - (deg*(180-deg));
   return scale * (num / dem * shift + 1);
}