#ifndef DAC8562_H_
#define DAC8562_H_

void dac8562_init(spi_device_t *device, GPIOPin ncs_gpio);

void DAC_SET_A(spi_device_t *device, unsigned int dc_value);

void DAC_SET_B(spi_device_t *device, unsigned int dc_value);

#endif /* DAC8562_H_ */
