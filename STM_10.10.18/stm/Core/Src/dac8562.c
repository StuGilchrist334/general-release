#include "spi_device.h"
#include "dac8562.h"

static void write_to_dac(spi_device_t *device, uint8_t *data)
{
	// pull chipselect to low, to enable dac
	ioport_set_pin_level(device->ncs_gpio, IOPORT_PIN_LEVEL_LOW);

  	spi2_send_bytes(3, data, SPI_PHASE_2EDGE);

	// pull chipselect to high, also to activate the last written value to be output
	ioport_set_pin_level(device->ncs_gpio, IOPORT_PIN_LEVEL_HIGH);
}

/**
* @brief Initialises the DAC as suggested in the datasheet after powerup.
*
* @author Brandon McHale
* @date 202212211649 created - BMH
* @date 202212221244 modified - PV - added comments / header, moved the table from data sheet into the function
* @date 202212311913 modified - PV - changed setting values to to a function call
*/
static void DAC_initialize(spi_device_t *device)
{
	// 24 Bits;
	/*
		:--------------------FIRST BYTE------------:-----------SECOND BYTE------------------:-----------THIRD BYTE-----------:
		Don't care :	Command Bits   :	Address    :		    Data MSB                    :			Data LSB             :
		[DB23,DB22,[DB21,DB20,DB19][DB18,DB17,DB16] [DB15,DB14,DB13,DB12,DB11,DB10,DB9,DB8] [DB7,DB6,DB5,DB4,DB3,DB2,DB1,DB0]
		x       x   1    0    1      x	  x    x      x    x    x     x	  x	   x	x	x	  x	  x	  x	  x	  x	  x	  x   1    = Power On Reset
		x       x   1    0    0      x	  x	   x	  x	   x	x	  x	  x	   x	x	x	  x	  x   0   0   x   x   1   1    = Power up both DACS
		x		x	1	 1	  1		 x	  x	   x	  x	   x	x	  x	  x	   x	x	x	  x	  x	  x	  x	  x	  x	  x   1    = En Internal Ref, Rst Gain=2
		x		x	0	 0	  0		 0	  1	   0	  x	   x	x	  x	  x	   x	x	x	  x	  x	  x	  x	  x	  x	  1   1    = A & B Gain=1
		x		x	1	 1	  0		 x	  x	   x	  x	   x	x	  x	  x	   x	x	x	  x	  x	  x	  x	  x	  x	  1   1    = LDAC\ Inactive for A&B Synchronous Mode
		x		x	0	 1	  1      1	  1	   1	  w	   w	w	  w	  w	   w	w	w	  w	  w	  w	  w	  w	  w	  w   w    = Write to DACS A&B Update DAC A&B
		x       x   0	 1	  1      0    0    0	  w	   w	w	  w	  w	   w	w	w	  w	  w	  w	  w	  w	  w	  w   w    = Write to DACA Update DACA
		x       x   0	 1	  1      0    0    1	  w	   w	w	  w	  w	   w	w	w	  w	  w	  w	  w	  w	  w	  w   w    = Write to DACB Update DACB
			
			
		To Start DAC Up after power applied to hardware;
			
		1)Software Reset
		2)Internal Reference Enable
		3)Gain Change back to 1 ( 2) sets it to 2) - Optional operation
		4)Power up DAC A and DAC B
		4)Set LDAC Inactive
		5)Ready to Rx DAC values			
	*/
	
	uint8_t   data[3];
	
	data[0]	= 0x28;
	data[1] = 0x00;
	data[2]	= 0x01;
	write_to_dac(device, data);
	//HAL_Delay(1);
	
	data[0]	= 0x38;
	data[1] = 0x00;
	data[2]	= 0x01;
	write_to_dac(device, data);
	//HAL_Delay(1);
	
	data[0]	= 0x02;
	data[1] = 0x00;
	data[2]	= 0x03;
	write_to_dac(device, data);
	//HAL_Delay(1);
	
	data[0]	= 0x30;
	data[1] = 0x00;
	data[2]	= 0x03;
	write_to_dac(device, data);

}

/*
 * @brief Initializes the dac8562 driver
 *
 * Call this before using any of the other functions
 *
 * @param[out]  device       Device to read/write.  This struct is populated by
 *                           the following parameters and should be used in calls
 *                           to the other functions.
 * @param[in]   pins         SPI interface the device is connected to
 * @param[in]   ncs_gpio     GPIO pin to use for this device's nCS pin
 *
 * @author Palaniappan Valliappan
 * @date 202212311725 created - PV
 */
void dac8562_init(spi_device_t *device, GPIOPin ncs_gpio)
{
    // Initialise the device struct
    device->ncs_gpio = ncs_gpio;

    // Configure the nCS pin with the slave deselected
    ioport_set_pin_level(ncs_gpio, IOPORT_PIN_LEVEL_HIGH);
	
	DAC_initialize(device);
}

/**
* @brief Sets DAC A (non inverting) output.
*
* @param dc_value unsigned 16 bit integer code for setting the DAC output
*
* @author Brandon McHale
* @date 202212211649 created - BMH
* @date 202212221244 modified - PV - added comments / header
* @date 202212311915 modified - PV - changed setting values to to a function call
*/
void DAC_SET_A(spi_device_t *device, unsigned int dc_value)	
{
	uint8_t data[3];
	
	if (dc_value > 0x9000)	//0x7A00
	{
		dc_value = 0x9000;	//0x7A00
	}
		
	data[0] = 0x18;
	data[1] = dc_value >>8;
	data[2] = dc_value & 0xFF;
	
	write_to_dac(device, data);
}

/**
* @brief Sets DAC B (inverting) output.
*
* @param dc_value unsigned 16 bit integer code for setting the DAC output
*
* @author Brandon McHale
* @date 202212211649 created - BMH
* @date 202212221244 modified - PV - added comments / header, changed input parameter from int to unsigned int
* @date 202212311915 modified - PV - changed setting values to to a function call
*/
void DAC_SET_B(spi_device_t *device, unsigned int dc_value)
{	
	uint8_t data[3];
	
	if (dc_value > 0x9000)	//0x7A00
	{
		dc_value = 0x9000;	//0x7A00
	}
	
	data[0] = 0x19;
	data[1] = dc_value >>8;
	data[2] = dc_value & 0xFF;
	
	write_to_dac(device, data);
}


