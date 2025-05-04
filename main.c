

/**
 * main.c
 */
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "systick_frt.h"
#include "FreeRTOS.h"
#include "task.h"
#include "status_led.h"

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

#define greenLED 0x08
#define redLED 0x02 
#define blueLED 0x04
#define yellowLED 0x01
#define whiteLED 0x0E
#define purpleLED 0x0C
#define orangeLED 0x0A
#define pinkLED 0x06


void init_hardware(){
  SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;
  INT8U dummyload = SYSCTL_RCGC2_R;

  GPIO_PORTF_DEN_R = PIN0 + PIN1 + PIN2 + PIN3 + PIN4;
  GPIO_PORTF_DIR_R = PIN1 +PIN2+PIN3;

}

void testTask(void *pvParameters)
{
    INT8U *sColourArray = (INT8U *)pvParameters;
    INT8U iterator = 0;


    while(1)
    {
        //vprintf(pcTaskName, "with the colour: %d\n\r", sColour); // Print the task name and colour
        GPIO_PORTF_DATA_R = sColourArray[iterator++];
        if(iterator > 4) {
            iterator = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));                           // Delay for 1000ms
    }
}

static void setupHardware(void)
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :
*****************************************************************************/
{
  // TODO: Put hardware configuration and initialisation in here
  init_hardware();
  // Warning: If you do not initialize the hardware clock, the timings will be inaccurate
  init_systick();
}

int main(void)
{
    setupHardware();
    const static INT8U LedColourArray[5] = {2, 2, 2, 2, 2};

    
    xTaskCreate(testTask, "TestTask1", 1000, (void*)LedColourArray, HIGH_PRIO, NULL); // Create the test task
  
    vTaskStartScheduler();
	return 0;
}



