/*
 * ADC14.c
 *
 *  Created on: May 11, 2017
 *      Author: Derek
 */


#include "ADC14.h"

static uint16_t ADC14_FLAG = 0;
static uint32_t ADC14_VALUE = 0;

uint16_t ADC14_get_flag(){
    return ADC14_FLAG;
}

void ADC14_reset_flag(){
    ADC14_FLAG = 0;
}

uint32_t ADC14_get_value(){
    return ADC14_VALUE;
}

// ADC14 interrupt service routine
void ADC14_IRQHandler(void) {
    ADC14_VALUE = ADC14->MEM[0];
    ADC14_FLAG = 1;
}
