/*
 * ADC14.c
 *
 *  Created on: May 11, 2017
 *      Author: Derek
 */


#include "ADC14.h"

#define NOCOMPARE 0
#define COMPARE 1

static uint16_t ADC14_FLAG = 0;

static uint32_t ADC14_VALUE = 0;

void ADC14_init(void){
    // Sampling time, S&H=32, ADC14 on, HSMCLK as source
   ADC14->CTL1 = ADC14_CTL1_RES__14BIT;         // Use sampling timer, 14-bit conversion results
   ADC14->CTL0 = ADC14_CTL0_SHT0__32 | ADC14_CTL0_SHP | ADC14_CTL0_ON | ADC14_CTL0_SSEL__HSMCLK;
   ADC14->MCTL[0] |= ADC14_MCTLN_INCH_1 ;   // A1 ADC input select; Vref=AVCC
   ADC14->IER0 = ADC14_IER0_IE0;          // Enable ADC conv complete interrupt
}

void ADC14_start_sample(){
    ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;
}

uint16_t ADC14_get_flag(){
    return ADC14_FLAG;
}

void ADC14_reset_flag(){
    ADC14_FLAG = 0;
}

uint32_t ADC14_get_value(){
    return ADC14_VALUE / 5;
}

// ADC14 interrupt service routine
void ADC14_IRQHandler(void) {
    ADC14_VALUE = ADC14->MEM[0];
    ADC14_FLAG = 1;
}
