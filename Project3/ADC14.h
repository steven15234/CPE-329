/*
 * ADC14.h
 *
 *  Created on: May 11, 2017
 *      Author: Derek
 */

#ifndef ADC14_H_
#define ADC14_H_

#include "msp.h"
#include <stdint.h>


void ADC14_init(void);

void ADC14_start_sample();
uint16_t ADC14_get_flag();
uint16_t ADC14_get_timer_flag();

void ADC14_reset_flag();
void ADC14_reset_timer_flag();

uint32_t ADC14_get_value();
uint32_t ADC14_get_timing();


#endif /* ADC14_H_ */
