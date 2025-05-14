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
extern TaskHandle_t xButtonTaskHandle_SW4;
extern TaskHandle_t xButtonTaskHandle_SW0;
extern TaskHandle_t xKeypadTaskHandle;
extern TaskHandle_t xDigiSwitchTaskHandle;
extern SemaphoreHandle_t xLcdQueueMutex;
/***************** Functions ******************/
void init_hardware(){
  SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF + SYSCTL_RCGC2_GPIOD + SYSCTL_RCGC2_GPIOC + SYSCTL_RCGC2_GPIOA + SYSCTL_RCGC2_GPIOE;
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
  init_systick();

}



void vTestTaskButtons(void *pvParameters)
{
    // One press adds 1 to buffer, double press adds 5, long press adds 10

    BaseType_t xStatus;
    ButtonState_t xButton;
    QueueHandle_t xButtonEventQueue;
    xButtonEventQueue = *(QueueHandle_t *) pvParameters;
    while (1)
    {

        xStatus = xQueueReceive(xButtonEventQueue, &xButton.event, portMAX_DELAY);

        if (xStatus == pdPASS)
        {
            
            switch (xButton.event)
            {
                case BE_SINGLE_PUSH:  
                    xPutLcdFunctionQueue(lcd_clear_display, NULL, NULL);
                    xPutLcdFunctionQueue(lcd_string_write, "Single Press!", NULL);
                    
                    //instructionWrite.pvParameter1 = "Single Press!";
                    //xQueueSend(xLcdFunctionQueue, &instructionClear, portMAX_DELAY);
                    //xQueueSend(xLcdFunctionQueue, &instructionWrite, portMAX_DELAY);
                    

                    break;
                case BE_DOUBLE_PUSH:
                    xPutLcdFunctionQueue(lcd_clear_display, NULL, NULL);
                    xPutLcdFunctionQueue(lcd_string_write, "Double Press!", NULL);
                    //instructionWrite.pvParameter1 = "Double Press!";
                    //xQueueSend(xLcdFunctionQueue, &instructionClear, portMAX_DELAY);
                    //xQueueSend(xLcdFunctionQueue, &instructionWrite, portMAX_DELAY);
                    
                    break;
                case BE_LONG_PUSH:
                    xPutLcdFunctionQueue(lcd_clear_display, NULL, NULL);
                    xPutLcdFunctionQueue(lcd_string_write, "Long Press!", NULL);
                    //instructionWrite.pvParameter1 = "Long Press!";
                    //xQueueSend(xLcdFunctionQueue, &instructionClear, portMAX_DELAY);
                    //xQueueSend(xLcdFunctionQueue, &instructionWrite, portMAX_DELAY);
                    
                    break;
                default:
                    lcd_string_write("failed to parse ButtonEvent");
                    while(1);
            }
        }
    }
}
enum elevatorState
{
    ELEVATOR_inital,
    ELEVATOR_MOVING,
    ELEVATOR_STOPPED,
    ELEVATOR_INPUT,
    ELEVATOR_CHOSE_FLOOR,
    ELEVATOR_BROKEN,
};
void usEleveatorState(INT16U state)
{
    INT8U event;
    #define DUMMYevent 0xFF
    switch (state)
    {
        case ELEVATOR_inital:
            xQueueReceive(xButtonEventQueue_SW4,&event, portMAX_DELAY);
            if(event == BE_LONG_PUSH)
            {
              // Send target floor
              state = ELEVATOR_MOVING;
            }
            break;
        case ELEVATOR_MOVING:
            // Queue Stop EVENT
            if(DUMMYevent == DUMMYevent)
            {
              state = ELEVATOR_STOPPED;
            }
            break;
        case ELEVATOR_STOPPED:
            //door opens timeout?
            state = ELEVATOR_INPUT;
            break;
        case ELEVATOR_INPUT:

            state = PIN3;
            break;
        case ELEVATOR_CHOSE_FLOOR:
            state = PIN4;
            break;
        case ELEVATOR_BROKEN:
            state = PIN5;
            break;
        default:
            lcd_string_write("failed to parse ElevatorState");
            while(1);
    }
}

void vElevatorTask(void *pvParameters)
{

  INT16U elevatorState = ELEVATOR_inital;
  while(1)
  {
    
    usEleveatorState(elevatorState);
    
  }

  
}
  



int main(void)
{

  setupHardware();
  
  xLcdQueueMutex = xSemaphoreCreateMutex();
  if(xLcdQueueMutex == NULL){
    lcd_string_write("Mutex creation failed!");
    while(1);
  }

  xButtonEventQueue_SW4 = xQueueCreate(5, sizeof(INT8U));
  if(xButtonEventQueue_SW4 == NULL){
    lcd_string_write("Queue creation failed!");
    while(1);
  }

  xButtonEventQueue_SW0 = xQueueCreate(5, sizeof(INT8U));
  if(xButtonEventQueue_SW0 == NULL){
    lcd_string_write("Queue creation failed!");
    while(1);
  }
  xLcdFunctionQueue = xQueueCreate(15, sizeof(LcdFunction_t));
  if (xLcdFunctionQueue == NULL)
  {
      lcd_string_write("Queue creation failed!");
      while (1);
  }

  xTaskCreate(vLCDTask, "LCD", USERTASK_STACK_SIZE, NULL, MED_PRIO, NULL);

  xTaskCreate(status_led_task, "Status LED", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL);
  //xTaskCreate(button_task, "Button1", USERTASK_STACK_SIZE, &temp1, HIGH_PRIO, &xButtonTaskHandle_SW4);
  //xTaskCreate(button_task, "Button2", USERTASK_STACK_SIZE, &temp2, HIGH_PRIO, &xButtonTaskHandle_SW0);
  //xTaskCreate(vTestTaskButtons,"Button1Test", USERTASK_STACK_SIZE, &xButtonEventQueue_SW4, MED_PRIO,NULL);
  //xTaskCreate(vTestTaskButtons,"Button1Test", USERTASK_STACK_SIZE, &xButtonEventQueue_SW0, MED_PRIO,NULL);
  xTaskCreate(vKeypadScanTask, "Keypad Scan", USERTASK_STACK_SIZE, NULL, HIGH_PRIO, &xKeypadTaskHandle);
  //xTaskCreate(vKeypadTestTask, "Keypad Test", USERTASK_STACK_SIZE, NULL, HIGH_PRIO, NULL);
  //xTaskCreate(vDigiswitchTask, "DigiSwitch", USERTASK_STACK_SIZE, NULL, HIGH_PRIO, &xDigiSwitchTaskHandle);
  //xTaskCreate(vUITask, "UI", USERTASK_STACK_SIZE + 16, NULL, LOW_PRIO, NULL);
  vTaskStartScheduler();
	return 0;
}




