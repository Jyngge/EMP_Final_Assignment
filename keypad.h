/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: keypad.h
*
* PROJECT....: Final assignment
*
* DESCRIPTION:
*
* Change Log:
******************************************************************************
* Date    Id          Change
* 060525  Majur22     Module created.
*
*****************************************************************************/
#ifndef _KEYPAD_H
#define _KEYPAD_H
/***************** Include files **************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "lcd.h"
/***************** Defines ********************/

/***************** Constants ******************/

/***************** Variables ******************/

/***************** Functions ******************/
void vKeypadInit(void);
void vKeypadScanTask(void *pvParameters);
void vKeypadTestTask(void *pvParameters);

 #endif
