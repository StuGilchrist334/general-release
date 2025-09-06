/**
 * @brief AD7766 ADC driver
 *
 * @author Dave Adshead
 * @version 1.1
 * @date 202302141610 modified - PV - commented out code to allow -ve to +ve data instead of 0 to full range
 */

#include <stdint.h>
#include <stdbool.h>
#include "adc_ad7766.h"


/*
 * Initializes the AD7766 driver
 * Call this before using any of the other functions
 *
 * @param[out]  device       Device to read/write.  This struct is populated by
 *                           the following parameters and should be used in calls
 *                           to the other functions.
 * @param[in]   ncs_gpio     GPIO pin to use for this device's nCS pin
 * @param[in]   ndrdy_gpio   GPIO pin to use for this device's nDRDY pin
 * @param[in]   npd_gpio     GPIO pin to use for this device's nSYNC/nPD pin
 */
 void ad7766_init(spi_device_t *device, GPIOPin ncs_gpio, GPIOPin ndrdy_gpio, GPIOPin npd_gpio)
 {
    // Initialise the device struct
    device->ncs_gpio = ncs_gpio;
    device->gpio1 = ndrdy_gpio;
    device->gpio2 = npd_gpio;

    // Configure the nCS pin with the slave deselected
    ioport_set_pin_level(ncs_gpio, IOPORT_PIN_LEVEL_HIGH);

    // Start the ADC converting
    ioport_set_pin_level(npd_gpio, IOPORT_PIN_LEVEL_HIGH);
}

/*
 * Reads an ADC input
 *
 * @param[in]   device       Device to use
 *
 * @returns A 24-bit signed ADC reading
 */
int32_t ad7766_convert(spi_device_t *device)
{
    uint32_t  value;
    uint8_t   miso_data[3];

    // The ADC is continuously converting with a new result
    // every 8, 16 or 32us depending on the 'speed grade'.
    // If we take too long to read a result it becomes invalid,
    // so we disable interrupts to prevent corruption if an
    // interrupt is serviced and delays the reading process.

    cpu_irq_disable();

    // Look for a falling edge on nDRDY
    // First wait for it to go high
    //while (!ioport_get_pin_level(device->gpio1))
    while((device->gpio1.port->IDR & device->gpio1.pin) == 0x00u); //put this in as the regular ioport_get_pin_level was failing - too slow to detect the transition?

    // Wait for the low edge
    //while (ioport_get_pin_level(device->gpio1))
    while((device->gpio1.port->IDR & device->gpio1.pin) != 0x00u); //put this in as the regular ioport_get_pin_level was failing - too slow to detect the transition?

    // Data is now valid, we have a short time to read it
    ioport_set_pin_level(device->ncs_gpio, IOPORT_PIN_LEVEL_LOW);

    spi2_get_bytes(3, miso_data, SPI_PHASE_2EDGE);

    ioport_set_pin_level(device->ncs_gpio, IOPORT_PIN_LEVEL_HIGH);

    cpu_irq_enable();

    // Construct the ADC value from the SPI data
    value = ((uint32_t)miso_data[0] << 16) | ((uint32_t)miso_data[1] << 8) | (uint32_t)miso_data[2];

    return (int32_t)value;
}
