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
#include "button.h"

/*****************************    Defines    *******************************/
#define BS_IDLE           0
#define BS_FIRST_PUSH     1
#define BS_FIRST_RELEASE  2
#define BS_SECOND_PUSH    3
#define BS_LONG_PUSH      4

#define ONE_SHOT          pdFALSE

/*****************************   Constants   *******************************/


/*****************************   Variables   *******************************/
QueueHandle_t xButtonEventQueue_SW4;
QueueHandle_t xButtonEventQueue_SW0;
QueueHandle_t xButtonEventQueue_DIGI;
TaskHandle_t xButtonTaskHandle_DIGI;
TaskHandle_t xButtonTaskHandle_SW4;
TaskHandle_t xButtonTaskHandle_SW0;

/*****************************   Functions   *******************************/
INT16U button_pushed(INT16U pin)
{
  return( !(GPIO_PORTF_DATA_R & pin) );                                // SW1 at PF4
}


void vLongPushCallback( TimerHandle_t xTimer )
{
    ButtonInfo_t *xButton;
	xButton = (ButtonInfo_t *)pvTimerGetTimerID( xTimer );

	if(xButton->button_state == (BS_FIRST_PUSH || BS_SECOND_PUSH))	                                    // if the timer runs out before the button is released it was a long push
	{
		xButton->button_state = BS_LONG_PUSH;
		xButton->event = BE_LONG_PUSH;
		xQueueSend( *xButton->xButtonEventQueue, &xButton->event, 0 );	// send event to queue
	}
	else if( xButton->button_state == BS_FIRST_RELEASE )
	{ 
		xButton->button_state = BS_IDLE;
		xButton->event = BE_SINGLE_PUSH;
		xQueueSend( *xButton->xButtonEventQueue, &xButton->event, 0 );			// send event to queue
	}
}

void vButtonInterruptHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xTaskWokenBySW4 = pdFALSE;
    BaseType_t xTaskWokenBySW0 = pdFALSE;

    INT16U ucButtonPushed = GPIO_PORTF_MIS_R & 0x11; // Check which button caused the interrupt
    GPIO_PORTF_ICR_R = ucButtonPushed;             // Clear the interrupt flag
    GPIO_PORTF_IM_R &= ~ucButtonPushed;            // Disable the interrupt for the button

    
    if (ucButtonPushed & 0x10) // SW4 (PF4)
    {
		vTaskNotifyGiveFromISR(xButtonTaskHandle_SW4, &xTaskWokenBySW4);
    }

    
    if (ucButtonPushed & 0x01) // SW0 (PF0)
    {
		vTaskNotifyGiveFromISR(xButtonTaskHandle_SW0, &xTaskWokenBySW0);
    }

    // Combine the results of both notifications
    xHigherPriorityTaskWoken = (xTaskWokenBySW4 || xTaskWokenBySW0);

    // Perform a context switch if a higher-priority task was woken
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void button_task( void *pvParameters )
/*****************************************************************************
*   Input    : 	Associated Pin.
*   Output   : 	Button event to button event queue.	
*   Function : 	This task handles the button events. It uses a timer to determine if the button is pushed, double pushed or long pushed.
*            	The task is notified from the interrupt handler when the button is pushed.
******************************************************************************/
{
	
	TimerHandle_t xButtonTimeOutTimer;
	//ButtonState_t xButton;
	
	TaskHandle_t xButtonTaskHandle = xTaskGetCurrentTaskHandle();
	ButtonInfo_t xButtonInfo = *(ButtonInfo_t *)pvParameters;
	xButtonInfo.button_state = BS_IDLE;

	xButtonTimeOutTimer = xTimerCreate( "LPTimeout", pdMS_TO_TICKS( 2000 ), ONE_SHOT, &xButtonInfo, vLongPushCallback);
	if( xButtonTimeOutTimer == NULL )
	{
		vLcdStringWrite("Timer creation failed!");
		while(1); 
	}


  	while(1)
  	{

		
  		switch( xButtonInfo.button_state )
  		{
  		case BS_IDLE:
  		    *(xButtonInfo.PORT_IM_R) |= xButtonInfo.pin;
			ulTaskNotifyTake( pdTRUE, portMAX_DELAY );	// clear the notification

		    if( !(*(xButtonInfo.PORT_DATA_R) & xButtonInfo.pin))		                                    // if button pushed
		    {
		        xButtonInfo.button_state = BS_FIRST_PUSH;                               // we go from the idle state to first push
				xTimerChangePeriod(xButtonTimeOutTimer, pdMS_TO_TICKS(2000), 0);
				xTimerStart( xButtonTimeOutTimer, 0 );                          // start the timer
  			}

		    break;
  		case BS_FIRST_PUSH:
		
  		  	if( (*(xButtonInfo.PORT_DATA_R) & xButtonInfo.pin) )	                                        // if button released before the timer runs out it was a normal push
			{
				xTimerStop( xButtonTimeOutTimer,0);                          // stop the timer
				xButtonInfo.button_state = BS_FIRST_RELEASE;
				xTimerChangePeriod( xButtonTimeOutTimer, pdMS_TO_TICKS( 200 ), 0); // we set a new timer to see if it is a long push
				xTimerStart( xButtonTimeOutTimer, 0 );                          // start the timer
  		  	}	
		    break;
  		case BS_FIRST_RELEASE:
  		 if( !(*(xButtonInfo.PORT_DATA_R) & xButtonInfo.pin) )		                                    // if button is pressed again before the timer runs out it was a double push
			{
			    xButtonInfo.button_state = BS_SECOND_PUSH;
			    xTimerStop( xButtonTimeOutTimer,0);
			    xTimerChangePeriod(xButtonTimeOutTimer, pdMS_TO_TICKS(2000), 0);                          // stop the timer
				xTimerStart( xButtonTimeOutTimer, 0 );                          // start the timer
  			}
			break;
  	  case BS_SECOND_PUSH:
  	  	if( (*(xButtonInfo.PORT_DATA_R) & xButtonInfo.pin) )					                        // if button released before the timer runs out it was a double push
			{
				xButtonInfo.event = BE_DOUBLE_PUSH;
				xQueueSend( *xButtonInfo.xButtonEventQueue, &xButtonInfo.event, 0 );	// send event to queue
			    xButtonInfo.button_state = BS_IDLE;
  	  	}
			break;
  	  case BS_LONG_PUSH:

  	  	if( (*(xButtonInfo.PORT_DATA_R) & xButtonInfo.pin) )					                        // when the button is released after a long push we go back to the idle state
			xButtonInfo.button_state = BS_IDLE;
			break;
  	  default:
  	  	break;

		
  		}
		vTaskDelay(10);
  	}
}


/****************************** End Of Module *******************************/












