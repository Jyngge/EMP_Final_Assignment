/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: UI.c
*
* PROJECT....: Final assignment
*
* DESCRIPTION: 
*
* Change Log:
******************************************************************************
* Date    Id          Change
* 090525  Majur22     Module created.
* 
*****************************************************************************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "FreeRTOS.h"
#include "task.h"
#include "list.h"
#include "queue.h"
#include "semphr.h"
#include "lcd.h"
#include "keypad.h"
#include "button.h"
#include "digiswitch.h"
#include <string.h>

/***************** Defines ********************/

extern QueueHandle_t xKeypadQueue;
extern QueueHandle_t xLcdFunctionQueue;
extern QueueHandle_t xButtonEventQueue_SW4;
extern QueueHandle_t xButtonEventQueue_SW0;
INT16U password;
INT16U ulCurrentFloor;
INT16U targetFloor;
extern INT16U postion;
extern TaskHandle_t xDigiSwitchTaskHandle;

typedef enum{
    idle,           //idles
    moving,         // only one entry and exit , timout
    inputCode,      // entered by keypad press, return on timeout
    selectFloor,    // Floor select, from correct keypress
    broken,         // entered from moving state on 4th trip
                    // display intruction
    pot,            // terget value / actual value
    Digi360         // 360 degree
}ElevatorState_t;

ElevatorState_t vElevatorState(ElevatorState_t state)
{
    static BaseType_t xStatus;
    static INT8U ucKeypadPress;
    static INT8U ucButtonPress;
    static INT8U i;

    switch(state){
    case idle:
        xStatus = xQueueReceive(xKeypadQueue,&ucKeypadPress,1);
        if(xStatus == pdPASS)
        {
            state = inputCode;
            lcdSendMoveCursor(0,1,5);
            lcdSendWriteChar(ucKeypadPress,5);
            password = 0;
            i = 100;
        }
        else
        {
            xQueueReceive(xButtonEventQueue_SW4,&ucButtonPress,1);
            if(ucButtonPress == BE_LONG_PUSH)
            {
                state = moving;
            }
        }
    break;
    case inputCode:
        xStatus = xQueueReceive(xKeypadQueue,&ucKeypadPress,1);
        if(xStatus == pdPASS)
        {
            lcdSendWriteChar(ucKeypadPress,2);
            password += (ucKeypadPress - '0')*i;
            i /= 10;
        }
        if(i != 1)
        {
            break;
        }
        if(!(password % 8)) // only need to check if last 3 didgets are divisible by 8. Need to invalidate some char
        {
            state = selectFloor;
            postion = ulCurrentFloor;
        }
        else
        {
            state = idle;
            lcdSendMoveCursor(0,1,2);
            lcdSendWriteString("    ",2);
        }
    break;
    case selectFloor:
        lcdSendCommand(lcdClearDisplay,2);
        lcdSendWriteString("Select Floor: ",2);
        lcdSendMoveCursor(4,1,2);
        lcdSendCommand(lcdIncrementCursorLeft,2);
        postion = ulCurrentFloor;
        vRoteryEncoderResume();

        xStatus = xQueueReceive(xButtonEventQueue_SW0,&ucButtonPress,portMAX_DELAY);
        if(ucButtonPress == BE_SINGLE_PUSH)
        {
            vRoteryEncoderSuspend();
            targetFloor = postion;
            state = moving;
        }


    break;

    case moving:
    // gen event
    break;
    case broken:
    // gen event

    break;
    }

    return state;
}
void vUITask(void)
/**********************************************
 * Input    :    
 * Output   : 
 * Function : Task to handle UI updates and interactions
 **********************************************/
{
    static ElevatorState_t state = idle;
    vRoteryEncoderSuspend();
    lcdSendCommand(lcdClearDisplay,2);
    lcdSendWriteString("Floor: ",2);
    lcdSendWriteChar((INT8U)(ulCurrentFloor + '0'),2);
    

    while(1)
    {

        state = vElevatorState(state);

    }

}
