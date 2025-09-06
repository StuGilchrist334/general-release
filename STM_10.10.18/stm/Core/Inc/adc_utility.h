#ifndef ADC_UTILITY_H_
#define ADC_UTILITY_H_

#include "adc_ad7766.h"
#include "spi_device.h"
#include "tasktable.h"
#include "globals.h"

void adc_utility_init(void);

int32_t adc_convert_manipulate(void);

int32_t adc_convert_manipulate_vp(void);

int32_t adc_conversion(void);

int check_signal_size(int array_size,int current_gain, int sensor_number, bool act_on_clip, uint16_t* max_value);

#endif /* ADC_UTILITY_H_ */
