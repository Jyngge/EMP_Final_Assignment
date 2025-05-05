/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: main.c
*
* PROJECT....: Final assignment
*
* DESCRIPTION: 
*
* Change Log:
******************************************************************************
* Date    Id          Change
* 040525  Majur22     Module created.
*
*****************************************************************************/
/***************** Include files **************/

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "systick_frt.h"
#include "FreeRTOS.h"
#include "task.h"
#include "status_led.h"
#include "lcd.h"
#include "queue.h"
#include "semphr.h"
/***************** Defines ********************/

#define USERTASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define IDLE_PRIO 0
#define LOW_PRIO  1
#define MED_PRIO  2
#define HIGH_PRIO 3

#define PIN0 0x01
#define PIN1 0x02
#define PIN2 0x04
#define PIN3 0x08
#define PIN4 0x10
#define PIN5 0x20
#define ALL_PINS 0xFF
#define greenLED 0x08
#define redLED 0x02 
#define blueLED 0x04
#define yellowLED 0x01
#define whiteLED 0x0E
#define purpleLED 0x0C
#define orangeLED 0x0A
#define pinkLED 0x06
#define CLEAR_DISPLAY 0x01
/***************** Constants ******************/
/***************** Variables ******************/
extern QueueHandle_t xStringQueue;
extern QueueHandle_t xControlQueue;

/***************** Functions ******************/
void init_hardware(){
  SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF + SYSCTL_RCGC2_GPIOD + SYSCTL_RCGC2_GPIOC;
  INT8U dummyload = SYSCTL_RCGC2_R;

  // buttons and leds
  GPIO_PORTF_DEN_R = PIN0 + PIN1 + PIN2 + PIN3 + PIN4;
  GPIO_PORTF_DIR_R = PIN1 +PIN2+PIN3;
  // lcd pins
  GPIO_PORTD_DEN_R = ALL_PINS; // enable all pins on port D
  GPIO_PORTD_DIR_R = ALL_PINS; // set all pins on port D to output
  GPIO_PORTC_DEN_R = ALL_PINS; // enable all pins on port C
  GPIO_PORTC_DIR_R = ALL_PINS; // set all pins on port C to output
  
}

static void setupHardware(void)
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :
*****************************************************************************/
{

  init_hardware();
  status_led_init();
  init_systick();


}

void vTestSendingTask(void *pvParameters)
{
    INT8U *stringMessage = "Hello LCD!";
    INT8U controlMessage = CLEAR_DISPLAY;

    while (1)
    {
        // Send a string to the string queue
        xQueueSend(xStringQueue, &stringMessage, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(3000)); // Wait 3 seconds

        // Send a control instruction to the control queue
        xQueueSend(xControlQueue, &controlMessage, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait 1 second
    }
}

int main(void)
{  

  xStringQueue = xQueueCreate(5, sizeof(INT8U *));
  if(xStringQueue == NULL){
    while(1);
  }

  xControlQueue = xQueueCreate(5, sizeof(INT8U));
  if(xControlQueue == NULL){
    while(1);
  }

  setupHardware();
  
  xTaskCreate(vLCDTask, "LCD", USERTASK_STACK_SIZE, NULL, MED_PRIO, NULL);
  xTaskCreate(status_led_task, "Status LED", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);
  xTaskCreate(vTestSendingTask, "Test Sending", USERTASK_STACK_SIZE, NULL, MED_PRIO, NULL);

  vTaskStartScheduler();
	return 0;
}




