#include "spi_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initializes the AD7766 driver
 * Call this before using any of the other functions
 *
 * @param[out]  device       Device to read/write.  This struct is populated by
 *                           the following parameters and should be used in calls
 *                           to the other functions.
 * @param[in]   pins         SPI interface the device is connected to
 * @param[in]   ncs_gpio     GPIO pin to use for this device's nCS pin
 * @param[in]   ndrdy_gpio   GPIO pin to use for this device's nDRDY pin
 * @param[in]   npd_gpio     GPIO pin to use for this device's nSYNC/nPD pin
 */
void ad7766_init(spi_device_t *device, GPIOPin ncs_gpio, GPIOPin ndrdy_gpio, GPIOPin npd_gpio);
/*
 * Reads an ADC input
 *
 * @param[in]   device       Device to use
 *
 * @returns A 24-bit signed ADC reading
 */
int32_t ad7766_convert(spi_device_t *device);


#ifdef __cplusplus
}
#endif
