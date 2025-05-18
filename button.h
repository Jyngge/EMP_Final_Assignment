/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: button.h
*
* PROJECT....: ECP
*
* DESCRIPTION: Test.
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 090215  MoH   Module created.
*
*****************************************************************************/

#ifndef BUTTON_H_
#define BUTTON_H_


/***************************** Include files *******************************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "button.h"
#include "freeRTOS.h"
#include "list.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "lcd.h"

/*****************************    Defines    *******************************/
typedef enum
{
  BE_SINGLE_PUSH,
  BE_DOUBLE_PUSH,
  BE_LONG_PUSH,
} button_event_t;

//typedef struct 
//{	
//    INT16U button_state;
//	button_event_t event;
//	QueueHandle_t *xButtonEventQueue;
//} ButtonState_t;

typedef struct {
    volatile INT32U* PORT_IM_R; // pointer to the register
    volatile INT32U* PORT_DATA_R;
    QueueHandle_t *xButtonEventQueue;
    INT16U pin;
    INT16U button_state;
    INT16U event;
} ButtonInfo_t;

/*****************************   Constants   *******************************/

/*****************************   Functions   *******************************/

void button_task(void *pvParameters);
/*****************************************************************************
*   Input    : -
*   Output   : Button Event
*   Function : Test function
******************************************************************************/


/****************************** End Of Module *******************************/
#endif /*BUTTON_H_*/
