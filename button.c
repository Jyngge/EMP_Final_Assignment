/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: button.c
*
* PROJECT....: EMP
*
* DESCRIPTION: See module specification file (.h-file).
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 090215  MoH   Module created.
*
*****************************************************************************/

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

/*****************************    Defines    *******************************/
#define BS_IDLE           0
#define BS_FIRST_PUSH     1
#define BS_FIRST_RELEASE  2
#define BS_SECOND_PUSH    3
#define BS_LONG_PUSH      4

#define ONE_SHOT          pdFALSE



/*****************************   Constants   *******************************/
QueueHandle_t xButtonEventQueue;
TimerHandle_t xButtonTimeOutTimer;


/*****************************   Variables   *******************************/
static INT8U  button_state = BS_IDLE;
static button_event_t event;
/*****************************   Functions   *******************************/
INT8U button_pushed()
{
  return( !(GPIO_PORTF_DATA_R & 0x10) );                                // SW1 at PF4
}


void vLongPushCallback( TimerHandle_t xTimer )
{
	if(button_state == (BS_FIRST_PUSH || BS_SECOND_PUSH))	                                    // if the timer runs out before the button is released it was a long push
	{
		button_state = BS_LONG_PUSH;
		event = BE_LONG_PUSH;
		xQueueSend( xButtonEventQueue, &event, 0 );	// send event to queue
	}
	else if( button_state == BS_FIRST_RELEASE )
	{ 
		button_state = BS_IDLE;
		event = BE_SINGLE_PUSH;
		xQueueSend( xButtonEventQueue, &event, 0 );			// send event to queue
	}
}

void button_task( void *pvParameters )
/*****************************************************************************
*   Input    :
*   Output   :
*   Function :
******************************************************************************/
{

	xButtonTimeOutTimer = xTimerCreate( "LPTimeout", pdMS_TO_TICKS( 2000 ), ONE_SHOT, NULL, vLongPushCallback);
	if( xButtonTimeOutTimer == NULL )
	{
		while(1); 
	}

  	while(1)
  	{

  		switch( button_state )
  		{
  		case BS_IDLE:

		    if( button_pushed( ))		                                    // if button pushed
		    {
		        button_state = BS_FIRST_PUSH;                               // we go from the idle state to first push
				xTimerChangePeriod(xButtonTimeOutTimer, pdMS_TO_TICKS(2000), 0);
				xTimerStart( xButtonTimeOutTimer, 0 );                          // start the timer
  			}
		    break;
  		case BS_FIRST_PUSH:
		
  		  	if( !button_pushed() )	                                        // if button released before the timer runs out it was a normal push
			{
				xTimerStop( xButtonTimeOutTimer,0);                          // stop the timer
				button_state = BS_FIRST_RELEASE;
				xTimerChangePeriod( xButtonTimeOutTimer, pdMS_TO_TICKS( 200 ), 0); // we set a new timer to see if it is a long push
				xTimerStart( xButtonTimeOutTimer, 0 );                          // start the timer
  		  	}	
		    break;
  		case BS_FIRST_RELEASE:
  		  if( button_pushed() )		                                    // if button is pressed again before the timer runs out it was a double push
			{
			    button_state = BS_SECOND_PUSH;
			    xTimerStop( xButtonTimeOutTimer,0);
			    xTimerChangePeriod(xButtonTimeOutTimer, pdMS_TO_TICKS(2000), 0);                          // stop the timer
				xTimerStart( xButtonTimeOutTimer, 0 );                          // start the timer
  			}
			break;
  	  case BS_SECOND_PUSH:
  	  	if( !button_pushed() )					                        // if button released before the timer runs out it was a double push
			{
				event = BE_DOUBLE_PUSH;
				xQueueSend( xButtonEventQueue, &event, 0 );	// send event to queue
			    button_state = BS_IDLE;
  	  	}
			break;
  	  case BS_LONG_PUSH:

  	  	if( !button_pushed() )					                        // when the button is released after a long push we go back to the idle state
  	  	    button_state = BS_IDLE;
			break;
  	  default:
  	  	break;

		
  		}
		vTaskDelay(10);
  	}
}


/****************************** End Of Module *******************************/












