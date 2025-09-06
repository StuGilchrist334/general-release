#include "qaqc.h"

qaqc_reply_struct qaqc_reply;

void reset_qaqc_reply_data(void)
{
	qaqc_reply.electrode_number = 0;
	qaqc_reply.gain = 0;
	qaqc_reply.running_average_ph = displayed_ph_running_average;
	qaqc_reply.calculated_temperature = last_temperature;
	qaqc_reply.salinity = 0;
	qaqc_reply.specific_conductivity = 0;
	qaqc_reply.actual_conductivity = 0;
	qaqc_reply.electrode_health = 0;
	qaqc_reply.array_health = 0;
	qaqc_reply.sensor_diag = 0;
	qaqc_reply.data_status = 0;
	qaqc_reply.peak_potential = 0xffff;

	uint8_t temp_value[2];

	convert_dac_potential_to_custom_bcd_2_byte_qaqc(&temp_value[0], qaqc_reply.peak_potential);
	qaqc_reply.peak_potential_reply = (uint16_t) ((uint16_t) (temp_value[0] << 8)) + temp_value[1];

	//sg! need some other resets in here probably

	qaqc_reply.peak_value = 0;
	qaqc_reply.qaqc_2 = 0;
	qaqc_reply.start_potential = 0;
	qaqc_reply.end_potential = 0;

	qaqc_reply.last_qaqc = QAQC_REPLY_LAST_SCAN;
	qaqc_reply.useful_customer_data = QAQC_REPLY_NOT_USEFUL_CUSTOMER_DATA;
}
