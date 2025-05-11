/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: digiswitch.h
*
* PROJECT....: Final assignment
*
* DESCRIPTION: 
*
* Change Log:
******************************************************************************
* Date    Id          Change
* 100525  Majur22     Module created.
*
*****************************************************************************/

#ifndef DIGISWITCH_H_
#define DIGISWITCH_H_

/***************** Include files **************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "lcd.h"

/***************** Defines ********************/

#define PIN5 0x20
#define PIN6 0x40
#define PIN7 0x80
#define ALL_PINS 0xFF
/***************** Constants ******************/

/***************** Variables ******************/

/***************** Functions ******************/
void vDigiswitchInit(void);
void vDigiswitchTask(void *pvParameters);



#endif /* DIGISWITCH_H_ */
