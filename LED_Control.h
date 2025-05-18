/*
 * LED_Control.h
 *
 *  Created on: 4 May 2025
 *      Author: ironr
 */

#ifndef LED_CONTROL_H_
#define LED_CONTROL_H_

// LED devices
#define LED_STATUS  11
#define LED_RED     21
#define LED_YELLOW  22
#define LED_GREEN   23

// LED acions
#define TURN_LED_ON     1
#define TURN_LED_OFF    2
#define TOGGLE_LED      3

// Door States
#define DOOR_OPEN       1
#define DOOR_CLOSED     0

// for input of vElevatorLedTask
typedef struct {
  int floors_moving;
  int door_state;
  int Return_Value;
} LedStatusMessage;


void init_LED(void);
void toggleLED(void);

void set_leds( BOOLEAN, BOOLEAN, BOOLEAN );

BOOLEAN turn_led( INT8U, INT8U );

void Color_led_task(void *pvParameters);

void vQueueReadLED(void * pvParameters);

void vElevatorLedTask(void *pvParameters);

void vLedTestTask(void *pvParameters);



#endif /* LED_CONTROL_H_ */
