#ifndef SPI_DEVICE_H_
#define SPI_DEVICE_H_

#include "ioport.h"

#ifdef __cplusplus
extern "C" {
#endif

	// SPI device
	// This identifies a specific device on a specific SPI interface
	struct spi_device_board
	{
		//SPI_t         *pins;        // SPI interface the device is on
		GPIOPin ncs_gpio;     // nCS line for this device
		GPIOPin gpio1;        // Driver-specific GPIO lines for this device
		GPIOPin gpio2;
	};
	typedef struct spi_device_board  spi_device_t;
	
#ifdef __cplusplus
}
#endif

#endif /* SPI_DEVICE_H_ */