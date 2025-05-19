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
* 020525  Majur22     Module created.
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
#include "button.h"
#include "timers.h"
#include "keypad.h"
#include "digiswitch.h"
#include "UI.h"
#include "event_groups.h"
#include "LED_Control.h"
#include "potmeter.h"
#include <stdlib.h>
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
extern QueueHandle_t xLcdFunctionQueue;
extern QueueHandle_t xButtonEventQueue_SW4;
extern QueueHandle_t xButtonEventQueue_SW0;
extern QueueHandle_t xStringQueue;
extern QueueHandle_t xLedStatusQueue;
extern QueueHandle_t xButtonEventQueue_DIGI;
extern TaskHandle_t xButtonTaskHandle_SW4;
extern TaskHandle_t xButtonTaskHandle_DIGI;
extern TaskHandle_t xDigiSwitchTaskHandleSelectFloor;
extern TaskHandle_t xDigiSwitchTaskHandle360;
extern TaskHandle_t xKeypadTaskHandle;
extern TaskHandle_t xPotMatchTaskHandle;
extern TaskHandle_t xElevatorStatusHandler;
extern TaskHandle_t xMovingStateHandler;
extern SemaphoreHandle_t xLcdQueueMutex;
extern EventGroupHandle_t xUIEventGroup;
/***************** Functions ******************/

void init_hardware(){
  SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOB + SYSCTL_RCGC2_GPIOF + SYSCTL_RCGC2_GPIOD + SYSCTL_RCGC2_GPIOC + SYSCTL_RCGC2_GPIOA + SYSCTL_RCGC2_GPIOE;
  INT8U dummyload = SYSCTL_RCGC2_R;

  // Unlock GPIO Port F Pin 0
  GPIO_PORTF_LOCK_R = 0x4C4F434B;          // Unlock GPIO Port F
  GPIO_PORTF_CR_R |= PIN0;  
  // buttons and leds
  GPIO_PORTF_DEN_R |= ALL_PINS;
  GPIO_PORTF_DIR_R |= PIN1 +PIN2+PIN3;
  GPIO_PORTF_PUR_R |= PIN0 + PIN4;
  GPIO_PORTF_IBE_R &= ~(PIN0 + PIN4); // interrupt on single edge 
  GPIO_PORTF_IEV_R &= ~(PIN0 + PIN4); // falling edge
  GPIO_PORTF_IM_R |= PIN0 + PIN4;      // enable interrupts on PF0 and PF4
  NVIC_EN0_R = 0x40000000;            // enable interrupt for GPIO Port F in NVIC
  // lcd pins
  GPIO_PORTD_DEN_R = ALL_PINS; // enable all pins on port D
  GPIO_PORTD_DIR_R = ALL_PINS; // set all pins on port D to output
  GPIO_PORTC_DEN_R = ALL_PINS; // enable all pins on port C
  GPIO_PORTC_DIR_R = ALL_PINS; // set all pins on port C to output
  
  GPIO_PORTE_ICR_R = 0xFF;
  NVIC_EN0_R |= (1 << 4);


}

static void setupHardware(void)
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :
*****************************************************************************/
{

  init_hardware();
  vKeypadInit();
  vDigiswitchInit();
  vPotmeterInit();
  init_adc();
  init_systick();

}
  


int main(void)
{

  setupHardware();

  xButtonEventQueue_SW4 = xQueueCreate(5, sizeof(INT8U));
  if(xButtonEventQueue_SW4 == NULL){
      vLcdStringWrite("Queue creation failed!");
    while(1);
  }

  xButtonEventQueue_DIGI = xQueueCreate(5, sizeof(INT8U));
  if(xButtonEventQueue_DIGI == NULL){
      vLcdStringWrite("Queue creation failed!");
    while(1);
  }
  xLcdFunctionQueue = xQueueCreate(20, sizeof(LcdMessage_t));
  if (xLcdFunctionQueue == NULL)
  {
      vLcdStringWrite("Queue creation failed!");
      while (1);
  }

  xLedStatusQueue = xQueueCreate(4, sizeof(LedStatusMessage));
  if(xLedStatusQueue == NULL){
      vLcdStringWrite("Queue creation failed!");
    while(1);
  }

  xStringQueue = xQueueCreate(1, sizeof(INT16U));
  if(xLedStatusQueue == NULL){
      vLcdStringWrite("Queue creation failed!");
    while(1);
  }

  xUIEventGroup = xEventGroupCreate();

  static ButtonInfo_t SW4 = { 
    .PORT_IM_R = (volatile INT32U *)0x40025410, 
    .PORT_DATA_R = (volatile INT32U *)0x400253FC, 
    .pin = PIN4,  
    .xButtonEventQueue = &xButtonEventQueue_SW4 
  };

 
  static ButtonInfo_t DIGI = { 
      .PORT_IM_R = (volatile INT32U *)0x40004410, 
      .PORT_DATA_R = (volatile INT32U *)0x400043FC, 
      .pin = PIN7,  
      .xButtonEventQueue = &xButtonEventQueue_DIGI 
  };
  
  //xTaskCreate(vButtonTestTask, "ButtonTest", configMINIMAL_STACK_SIZE, NULL, LOW_PRIO, NULL);
  xTaskCreate(vLCDTask, "LCD", USERTASK_STACK_SIZE, NULL, MED_PRIO, NULL);
  //xTaskCreate(vLcdTaskTester,"LCD Test", USERTASK_STACK_SIZE, NULL, LOW_PRIO,NULL);
  xTaskCreate(status_led_task, "Status LED", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);
  xTaskCreate(button_task, "Button1", USERTASK_STACK_SIZE, &SW4, HIGH_PRIO, &xButtonTaskHandle_SW4);
  xTaskCreate(button_task, "Button3", USERTASK_STACK_SIZE, &DIGI, HIGH_PRIO, &xButtonTaskHandle_DIGI);
  xTaskCreate(vKeypadScanTask, "Keypad Scan", USERTASK_STACK_SIZE, NULL, HIGH_PRIO, &xKeypadTaskHandle);
  //xTaskCreate(vKeypadTestTask, "Keypad Test", USERTASK_STACK_SIZE, NULL, HIGH_PRIO, NULL);
  xTaskCreate(vDigiswitchTaskSelectFloor, "DigiSwitchSelectFloor", USERTASK_STACK_SIZE, NULL, HIGH_PRIO, &xDigiSwitchTaskHandleSelectFloor);
  xTaskCreate(vDigiswitchTask360, "DigiSwitch360", USERTASK_STACK_SIZE, NULL, 4, &xDigiSwitchTaskHandle360);
  xTaskCreate(vUITask, "UI", USERTASK_STACK_SIZE + 20, NULL, MED_PRIO, &xMovingStateHandler);
  //xTaskCreate(vLedTestTask, "TestTask", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);
  xTaskCreate(vPotMatchTask, "PotMatch", USERTASK_STACK_SIZE + 10, NULL, LOW_PRIO, &xPotMatchTaskHandle);
  xTaskCreate(vElevatorLedTask, "Elevator Status", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);
  xTaskCreate(vQueueReadLED, "LEDQueue", USERTASK_STACK_SIZE, NULL, LOW_PRIO, &xElevatorStatusHandler);



  vTaskStartScheduler();
	return 0;
}




