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

uint16_t ADC14_get_flag();
void ADC14_reset_flag();
uint32_t ADC14_get_value();


#endif /* ADC14_H_ */
