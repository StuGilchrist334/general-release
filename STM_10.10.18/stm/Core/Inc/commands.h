/**
 * @file commands.h
 * @brief Lists all the command definitions for communication with interface board.
 *
 * @author Palaniappan Valliappan
 * @version 1.6
 * @date 202301171643 created - PV
 * @date 202301181329 modified - PV - renamed few #defines for consistency
 * @date 202301191735 modified - PV - added qaqc data ready to be read flag
 * @date 202301201001 modified - PV - edited todo comment
 * @date 202302161016 modified - PV - integrate code changes made in Brandon's V8.6.9
 * @date 202302171611 modified - RR - added command text ph files and options
 * @date 202302201500 modified - PV - re-factored the names used for ph file to ph output
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

// start scan
#define COMMAND_TEXT_SCAN_START							0x01
// start scan - options
#define COMMAND_OPTION_TEXT_SCAN_OCEAN_FIRST			0x01
#define COMMAND_OPTION_TEXT_SCAN_LI_OCEAN_FIRST			0x02
#define COMMAND_OPTION_TEXT_SCAN_SI_OCEAN_FIRST			0x03

#define COMMAND_OPTION_TEXT_SCAN_FRESH_FIRST			0x04
#define COMMAND_OPTION_TEXT_SCAN_LI_FRESH_FIRST			0x05
#define COMMAND_OPTION_TEXT_SCAN_SI_FRESH_FIRST			0x06

#define COMMAND_OPTION_TEXT_SCAN_EXISTING_FIRST			0x07
#define COMMAND_OPTION_TEXT_START_EXISTING_FIRST		0x08

#define COMMAND_OPTION_TEXT_SCAN_VF_FIRST				0x09
#define COMMAND_OPTION_TEXT_SCAN_LI_VF_FIRST			0x0A
#define COMMAND_OPTION_TEXT_SCAN_SI_VF_FIRST			0x0B

#define COMMAND_OPTION_TEXT_SCAN_OCEAN					0x81
#define COMMAND_OPTION_TEXT_SCAN_LI_OCEAN				0x82
#define COMMAND_OPTION_TEXT_SCAN_SI_OCEAN				0x83

#define COMMAND_OPTION_TEXT_SCAN_FRESH					0x84
#define COMMAND_OPTION_TEXT_SCAN_LI_FRESH				0x85
#define COMMAND_OPTION_TEXT_SCAN_SI_FRESH				0x86

#define COMMAND_OPTION_TEXT_SCAN_EXISTING				0x87
#define COMMAND_OPTION_TEXT_START_EXISTING				0x88

#define COMMAND_OPTION_TEXT_SCAN_VF						0x89
#define COMMAND_OPTION_TEXT_SCAN_LI_VF					0x8A
#define COMMAND_OPTION_TEXT_SCAN_SI_VF					0x8B

// stop scan
#define COMMAND_TEXT_SCAN_STOP							0x02
// baud rate
#define COMMAND_TEXT_EXTERNAL_BAUD_RATE					0x05
// sensor mode
#define COMMAND_TEXT_SENSOR_MODE						0x06
// sensor mode - options
#define COMMAND_OPTION_TEXT_GET							0x00	// all 'get' option for other commands use this as well
#define COMMAND_OPTION_TEXT_SENSOR_MODE_CONTROLLED		0x01
#define COMMAND_OPTION_TEXT_SENSOR_MODE_AUTONOMOUS_OCEAN		0x02
#define COMMAND_OPTION_TEXT_SENSOR_MODE_AUTONOMOUS_FRESH	0x03

// sensor name
// TODO: CHECK - command sensor name - check if it replies ANB?
#define COMMAND_TEXT_SENSOR_NAME						0x07
// sensor sub name
// TODO: CHECK - command sensor sub name - check what is returned for this
#define COMMAND_TEXT_SENSOR_SUB_NAME					0x08
// sensor array interval - in minutes, if 1 to 14 set as 0 and valid values from 15 to 255
#define COMMAND_TEXT_SENSOR_ARRAY_INTERVAL				0x09
// sensor auton delay - 0xaa (hours) 0xbb (minutes) - valid values are 0 minutes, 15 minutes to 4 hours
#define COMMAND_TEXT_SENSOR_AUTON_DELAY					0x0A
// sensor communication style
#define COMMAND_TEXT_SENSOR_COMMUNICATION_STYLE			0x0B
// sensor communication style - options
#define COMMAND_OPTION_TEXT_ASCII_COMMUNICATION_STYLE	0x01
#define COMMAND_OPTION_TEXT_MOBUS_COMMUNICATION_STYLE	0x02

// sensor serial number
// set uses 6 ascii characters for serial number after command
#define COMMAND_TEXT_SENSOR_SERIAL_NUMBER				0x0C
// sensor immersion rule
#define COMMAND_TEXT_SENSOR_IMMERSION_RULE				0x0D
// sensor immersion rule - options
#define COMMAND_OPTION_TEXT_USE_IMMERSION				0x01
#define COMMAND_OPTION_TEXT_IGNORE_IMMERSION			0x02

// for getting salinity, ph, temperature and immersion status
#define COMMAND_TEXT_SALINITY							0x0E
#define COMMAND_TEXT_PH									0x0F

#define COMMAND_TEXT_TEMPERATURE						0x10
#define COMMAND_TEMPERATURE_LENGTH						0X04

#define COMMAND_TEXT_IMMERSED_DRY_STATUS				0x11
// sensor square wave data - option to write raw data to sd card on interface board
#define COMMAND_TEXT_SQUAREWAVE_DATA					0x12
// sensor square wave data - options
#define COMMAND_OPTION_TEXT_SQUAREWAVE_DATA_ON			0x01
#define COMMAND_OPTION_TEXT_SQUAREWAVE_DATA_OFF			0x02

// manufacturer name
#define COMMAND_TEXT_MANUFACTURER						0x13
// driver board firmware version
#define COMMAND_TEXT_DRIVER_VERSION						0x14
// system modbus address
#define COMMAND_TEXT_SYSTEM_MODBUS_ADDRESS				0x15
// qaqc data
#define COMMAND_TEXT_QAQC_DATA							0x16
// array health
#define COMMAND_TEXT_ARRAY_HEALTH						0x17
// qaqc data ready to be read
#define COMMAND_TEXT_QAQC_DATA_READY_TO_BE_READ			0x18

// ph output style
#define COMMAND_TEXT_PH_OUTPUT_STYLE					0x1B
// pH output style - options
#define COMMAND_OPTION_TEXT_PH_OUTPUT_LONG_ON			0x01
#define COMMAND_OPTION_TEXT_PH_OUTPUT_LONG_OFF			0x02

//temperature calibration
#define COMMAND_TEXT_TEMPERATURE_CALIBRATION			0x1C

#define COMMAND_TEXT_PH_HEARTBEAT_STYLE					0x1D
#define COMMAND_OPTION_TEXT_ALL_HEARTBEAT				0x01
#define COMMAND_OPTION_TEXT_PH_ONLY						0x02

#define COMMAND_TEXT_ANB_OR_ALTERNATE_HEAD				0x1E
#define COMMAND_OPTION_ANBSENSORS_HEAD					0x01
#define COMMAND_OPTION_VALEPORT_HEAD					0x02

#define COMMAND_TEXT_CLEAR_PH_ARRAY						0x1F

#define COMMAND_TEXT_NEED_MORE_TIME						0x20
#define COMMAND_TEXT_COMMENCING_SHUTDOWN				0x21

#define COMMAND_TEXT_CONDITION_REF_OCEAN				0x24
#define COMMAND_TEXT_CONDITION_REF_FRESHWATER			0x25
#define COMMAND_TEXT_SCAN_N_NO_OF_PH_ELECTRODES			0x2E //tell everyone this has been repurposed
#define COMMAND_TEXT_PRECONDITION_PH_ELECTRODES			0x28

#define COMMAND_TEXT_INTERFACE							0x2F
#define COMMAND_OPTION_TEXT_INTERFACE_ANB				0x01
#define COMMAND_OPTION_TEXT_INTERFACE_WM				0x02

#define COMMAND_TEXT_EE_RESET							0x31
#define COMMAND_TEXT_NO_STORE							0x32
#define COMMAND_TEXT_FIRMWARE_VERSION					0x33
#define COMMAND_TEXT_FAST_PROFILING						0x34
#define COMMAND_TEXT_ABRADED							0x35
#define COMMAND_TEXT_USE_SDCARD							0x36
#define COMMAND_TEXT_POWER								0x37
#define COMMAND_TEXT_SALINITY_RANGE						0x38
#define COMMAND_TEXT_PHS_PER_LOOP						0x39

#endif /* COMMANDS_H_ */
