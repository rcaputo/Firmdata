/*
 * adc.h
 *
 *  Created on: Jul 21, 2012
 *      Author: Tyler
 */

#ifndef ADC_H_
#define ADC_H_

void adc_init(void);
bool adc_take_sample(uint8_t pin, uint8_t channel);
void adc_check_sample(void);

#endif /* ADC_H_ */
