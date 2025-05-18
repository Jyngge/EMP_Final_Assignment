/*
 * LED_Control.c
 *
 *  Created on: 4 May 2025
 *      Author: ironr
 */
#include <stdint.h>

#include "tm4c123gh6pm.h"

#include "FreeRTOS.h"

#include "Task.h"

#include "queue.h"

#include "semphr.h"

#include "emp_type.h"

#include "LED_Control.h"

#include <math.h>

QueueHandle_t xLedStatusQueue;

extern TaskHandle_t xMovingStateHandler;

TaskHandle_t xElevatorStatusHandler;

LedStatusMessage QueueLED;

int delay_01 = 250;
int Calc_Move = 2;
int floors_moving;
int State = 0;
int mid_point;
int direction;
int start;
int end = 2;
int Read;

void init_LED(void) {
  /*****************************************************************************
   *   Input    :
   *   Output   :
   *   Function :
   ******************************************************************************/
  int dummy;

  // Enable the GPIO port that is used for the on-board LED.
  SYSCTL_RCGC2_R &= SYSCTL_RCGC2_GPIOD | SYSCTL_RCGC2_GPIOF;

  // Do a dummy read to insert a few cycles after enabling the peripheral.
  dummy = SYSCTL_RCGC2_R;

  // Set the direction as output (PF1, PF2 and PF3).
  GPIO_PORTF_DIR_R = 0x0E;
  // Set the direction as output (PD6).
  GPIO_PORTD_DIR_R = 0x40;

  // Enable the GPIO pins for digital function (PF0, PF1, PF2, PF3, PF4).
  GPIO_PORTF_DEN_R = 0x1F;
  // Enable the GPIO pins for digital function (PD6).
  GPIO_PORTD_DEN_R = 0x40;

  // Enable internal pull-up (PF0 and PF4).
  GPIO_PORTF_PUR_R = 0x11;
}

void toggleLED(void) {
  /*****************************************************************************
   *   Input    :
   *   Output   :
   *   Function : The toggler
   ******************************************************************************/
  // Toggle PF1 using XOR
  GPIO_PORTF_DATA_R ^= 0x02; // 0x02 = bit 1 = PF1  (make variable so other LED'S can be toggled )
}

void set_leds(red, yellow, green)
BOOLEAN red, yellow, green;
/*****************************************************************************
 *   Input    :
 *   Output   :
 *   Function :
 ******************************************************************************/
{
  turn_led(LED_RED, red);
  turn_led(LED_YELLOW, yellow);
  turn_led(LED_GREEN, green);
}

BOOLEAN turn_led(name, action)
INT8U name;
INT8U action;
/*****************************************************************************
 *   Input    :
 *   Output   :
 *   Function :
 ******************************************************************************/
{
  BOOLEAN Result;

  switch (name) {
  case LED_GREEN:
    switch (action) {
    case TURN_LED_ON:
      GPIO_PORTF_DATA_R &= 0xF7;
      break;
    case TURN_LED_OFF:
      GPIO_PORTF_DATA_R |= 0x08;
      break;
    case TOGGLE_LED:
      GPIO_PORTF_DATA_R ^= 0x08;
      break;
    }
    Result = !(GPIO_PORTF_DATA_R & 0x02);
    break;
  case LED_YELLOW:
    switch (action) {
    case TURN_LED_ON:
      GPIO_PORTF_DATA_R &= 0xFB;
      break;
    case TURN_LED_OFF:
      GPIO_PORTF_DATA_R |= 0x04;
      break;
    case TOGGLE_LED:
      GPIO_PORTF_DATA_R ^= 0x04;
      break;
    }
    Result = !(GPIO_PORTF_DATA_R & 0x01);
    break;
  case LED_RED:
    switch (action) {
    case TURN_LED_ON:
      GPIO_PORTF_DATA_R &= 0xFD;
      break;
    case TURN_LED_OFF:
      GPIO_PORTF_DATA_R |= 0x02;
      break;
    case TOGGLE_LED:
      GPIO_PORTF_DATA_R ^= 0x02;
      break;
    }
    Result = !(GPIO_PORTF_DATA_R & 0x40);
    break;
  }
  return (Result);
}

void Color_led_task(void * pvParameters)
/*****************************************************************************
 *   Input    :
 *   Output   :
 *   Function :
 ******************************************************************************/
{
  while (1) {
    // Toggle LED's
    set_leds(TURN_LED_ON, TURN_LED_OFF, TURN_LED_OFF);
    vTaskDelay(delay_01 / portTICK_RATE_MS);
    set_leds(TURN_LED_OFF, TURN_LED_ON, TURN_LED_OFF);
    vTaskDelay(delay_01 / portTICK_RATE_MS);
    set_leds(TURN_LED_OFF, TURN_LED_OFF, TURN_LED_ON);
    vTaskDelay(delay_01 / portTICK_RATE_MS);
  }
}

void vQueueReadLED(void * pvParameters)
/*****************************************************************************
 *   Input    :
 *   Output   :
 *   Function :
 ******************************************************************************/
{
  while(1)
  {
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
      if (xQueueReceive(xLedStatusQueue, & QueueLED, portMAX_DELAY) == pdTRUE)
      {
          floors_moving = QueueLED.floors_moving;
      State = QueueLED.door_state;
      start = Calc_Move;
      end = floors_moving;
      direction = (end > start) ? 1 : -1;
      mid_point = start + ((end - start) / 2);
      vTaskDelay(delay_01 / portTICK_RATE_MS);
    }
  }
}

void vElevatorLedTask(void * pvParameters)
/*****************************************************************************
 *   Input    :
 *   Output   :
 *   Function :
 ******************************************************************************/
{
  while (1) {

    if (State == 0)
    {
      // Elevator changing floor
      for (; Calc_Move != end; Calc_Move += direction)
      {
          QueueLED.Return_Value = Calc_Move;
          xQueueSend(xLedStatusQueue, &QueueLED, portMAX_DELAY);
          xTaskNotifyGive(xMovingStateHandler);
        // Elevator moving - 0-50% yellow blink & 50-100% red blink
        if ((direction == 1 && Calc_Move < mid_point) || (direction == -1 && Calc_Move > mid_point))
        {
          set_leds(TURN_LED_OFF, TURN_LED_ON, TURN_LED_OFF);
        } else {
          set_leds(TURN_LED_ON, TURN_LED_OFF, TURN_LED_OFF);

        }
        vTaskDelay(delay_01 / portTICK_RATE_MS);
        set_leds(TURN_LED_OFF, TURN_LED_OFF, TURN_LED_OFF);
        vTaskDelay(delay_01 / portTICK_RATE_MS);
      }
      // Elevator stopped
      if (Calc_Move == floors_moving) {
        set_leds(TURN_LED_OFF, TURN_LED_OFF, TURN_LED_ON);
        QueueLED.Return_Value = Calc_Move;
        xQueueSend(xLedStatusQueue, &QueueLED, portMAX_DELAY);
        xTaskNotifyGive(xMovingStateHandler);
      } else {
        set_leds(TURN_LED_OFF, TURN_LED_OFF, TURN_LED_OFF);
      }
    }
    // Elevator breakdown
    while (State == 1) {
      set_leds(TURN_LED_ON, TURN_LED_ON, TURN_LED_ON);
      vTaskDelay(delay_01 / portTICK_RATE_MS);
      set_leds(TURN_LED_OFF, TURN_LED_OFF, TURN_LED_OFF);
      vTaskDelay(delay_01 / portTICK_RATE_MS);
    }
  }
}

void vLedTestTask(void * pvParameters)
/*****************************************************************************
 *   Input    :
 *   Output   :
 *   Function : A task just for testing the vElevatorLedTask
 ******************************************************************************/
{
  LedStatusMessage testMsg;

  while (1) {
    //------------
    testMsg.floors_moving = 15;
    testMsg.door_state = 0;
    xQueueSend(xLedStatusQueue, & testMsg, portMAX_DELAY);
    Read = 1;
    vTaskDelay(10000 / portTICK_RATE_MS);

    //------------
    testMsg.floors_moving = 15;
    testMsg.door_state = 1;
    xQueueSend(xLedStatusQueue, & testMsg, portMAX_DELAY);
    Read = 1;
    vTaskDelay(10000 / portTICK_RATE_MS);

    //------------
    testMsg.floors_moving = 20;
    testMsg.door_state = 0;
    xQueueSend(xLedStatusQueue, & testMsg, portMAX_DELAY);
    Read = 1;
    vTaskDelay(10000 / portTICK_RATE_MS);

    //------------
    testMsg.floors_moving = 0;
    testMsg.door_state = 0;
    xQueueSend(xLedStatusQueue, & testMsg, portMAX_DELAY);
    Read = 1;
    vTaskDelay(10000 / portTICK_RATE_MS);
  }
}
