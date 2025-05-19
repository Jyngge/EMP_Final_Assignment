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
#include "event_groups.h"
#include "UI.h"

/***************** Defines ********************/

#define PIN5 0x20
#define PIN6 0x40
#define PIN7 0x80
#define ALL_PINS 0xFF

#define ENCODER_FLOOR_SELECT    0x01
#define ENCODER_360             0x02
#define ENCODER_TURN_DIR        0x04
#define ENCODER_TURN_COMPLET    0x08
#define ENCODER_SELECT_COMPLET  0x10
#define ENCODER_EVENT_FLAGS     0x0F

/***************** Constants ******************/

/***************** Variables ******************/

/***************** Functions ******************/
void vDigiswitchInit(void);
void vDigiswitchTask360(void *pvParameters);
void vDigiswitchTaskSelectFloor(void *pvParameters);
void vRoteryEncoderSuspend(void);
void vRoteryEncoderResume(void);



#endif /* DIGISWITCH_H_ */
