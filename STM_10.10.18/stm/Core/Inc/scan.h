#ifndef SCAN_H_
#define SCAN_H_

#include "tasktable_master.h"
#include "subsystem.h"
#include "timer.h"
#include "string.h"
#include "usart_commsboard.h"
#include "data_smoothing.h"
#include "globals.h"
#include "temperature.h"
#include "qaqc.h"
#include "salinity_conductivity.h"

extern qaqc_reply_struct qaqc_reply;
extern bool ph_array_index_looped;
extern bool store_last_electrode_number;

void scan(int scan_type);

#endif /* SCAN_H_ */
