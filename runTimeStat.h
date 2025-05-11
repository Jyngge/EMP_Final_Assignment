/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: runTimeStat.h
*
* PROJECT....: Final assignment
*
* DESCRIPTION: 
*
* Change Log:
******************************************************************************
* Date    Id          Change
* 110525  Majur22     Module created.
* 
*****************************************************************************/
/***************** Include files **************/
#ifndef RUNTIMESTAT_H_
#define RUNTIMESTAT_H_

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "systick_frt.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/***************** Defines ********************/

/***************** Constants ******************/

/***************** Variables ******************/

/***************** Function *******************/

void vRunTimeStatInit(void);
static void prvStatsTask( void *pvParameters );


#endif /* RUNTIMESTAT_H_ */
