/**
 * @file qaqc.h
 * @brief QAQC reply related code
 *
 * @author Palaniappan Valliappan
 * @version 1.1
 * @date 202302230919 created - PV
 * @date 202302231346 modified - PV - added sensor / electrode number reset
 */


#ifndef QAQC_H_
#define QAQC_H_

#include "globals.h"
#include "calculate_ph.h"
#include "usart_commsboard.h"


/**
* @brief Resets the qaqc reply to default values
*
* called after each scan to clear the previous data in the structure to default values.
*
* @author Palaniappan Valliappan
* @version v1.4
* @date 202301290352 created - PV
* @date 202302171324 modified - PV - changed default pH value to out of range number
* @date 202302200921 modified - PV - re-factored variable names to match definition in globals.h
* @date 202302231051 modified - PV - removed todo comments
* @date 202302231346 modified - PV - added sensor / electrode number reset
*/
void reset_qaqc_reply_data(void);

#endif /* QAQC_H_ */
