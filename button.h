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

/*****************************    Defines    *******************************/
typedef enum
{
  BE_SINGLE_PUSH,
  BE_DOUBLE_PUSH,
  BE_LONG_PUSH,
} button_event_t;


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
