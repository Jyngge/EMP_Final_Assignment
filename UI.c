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
#include "math.h"
#include "event_groups.h"
/***************** Defines ********************/
#define PASSWORD_LENGTH 4
#define ARROW_UP        0x18
#define ARROW_DOWN      0x19


extern QueueHandle_t xKeypadQueue;
extern QueueHandle_t xLcdFunctionQueue;
extern QueueHandle_t xButtonEventQueue_SW4;
extern QueueHandle_t xButtonEventQueue_SW0;
EventGroupHandle_t xUIEventGroup;
INT16U ulCurrentFloor = 2;
INT16U ulTargetFloor = 0;
INT16U ulTripCount;

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

BOOLEAN ulCheckPassword(INT8U* buffer)
{
    INT8U i;
    INT16U value = 0;
    for(i=0;i<PASSWORD_LENGTH;i++)
    {

        if(buffer[i] == '*' || buffer[i] == '#')
        {
            return 0;
        }
        value += (buffer[i] - '0')* pow(10,i);
    }
    if(!(value % 8))
    {
        return 1;
    }
    
    return 0;
}


ElevatorState_t vElevatorState(ElevatorState_t state)
{
    static BaseType_t xStatus;
    static INT8U ucKeypadPress;
    static INT8U ucButtonPress;
    static INT8S i;
    static INT8U ucPasswordBuffer[4];
    static INT8U distance;
    static BOOLEAN EncodeDirection = 0;

    switch(state){
    case idle:
        
        xStatus = xQueueReceive(xKeypadQueue,&ucKeypadPress,0);
        if(xStatus == pdPASS)
        {
            state = inputCode;

            lcdSendWriteChar(ucKeypadPress,5);
            i = 3;
            ucPasswordBuffer[i--] = ucKeypadPress;
            
        }
        else
        {
            xQueueReceive(xButtonEventQueue_SW4,&ucButtonPress,0);
            if(ucButtonPress == BE_LONG_PUSH)
            {
                if(ulCurrentFloor == ulTargetFloor)
                {
                    break;
                }
                state = moving;

            }
        }
    break;
    case inputCode:
        if(i >= 0)
        {
            xStatus = xQueueReceive(xKeypadQueue,&ucKeypadPress,1);
            if(xStatus == pdPASS)
            {
                lcdSendWriteChar(ucKeypadPress,2);
                ucPasswordBuffer[i--] = ucKeypadPress;
            }
        }
        else
        {
            xQueueReceive(xKeypadQueue,&ucKeypadPress,portMAX_DELAY);
            
            if(ucKeypadPress != '#')
            break;

            if(ulCheckPassword(ucPasswordBuffer))
            {
                state = selectFloor;
            }
            else
            {
                state = idle;
                lcdSendMoveCursor(0,1,2);
                lcdSendWriteString("    ",2);
                lcdSendMoveCursor(0,1,5);
            }

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
            ulTargetFloor = postion;
            state = moving;
        }


    break;

    case moving:
        lcdSendMoveCursor(8,0,2);
        if(ulTargetFloor > ulCurrentFloor)
        {
            distance = ulTargetFloor - ulCurrentFloor;
            lcdSendWriteChar((INT8U)ARROW_UP,2);
        }
        else
        {
            distance = ulCurrentFloor - ulTargetFloor;
            lcdSendWriteChar(0b00011001,2);
        }
        // notify led task and wait for respons
        xQueueReceive(xButtonEventQueue_SW0, &ucButtonPress,portMAX_DELAY);

        if(ulTripCount == 4)
        {
            state = broken;
        }
        state = idle;
        lcdSendMoveCursor(0,1,5);
    break;
    case broken:
        lcdSendCommand(lcdClearDisplay,portMAX_DELAY);
        lcdSendWriteString("Elavator broken!",2);
        vTaskDelay(pdMS_TO_TICKS(2000));
        state = pot;
    break;
    case pot:
        lcdSendCommand(lcdClearDisplay,portMAX_DELAY);
        lcdSendWriteString("Match value",2);

        vTaskDelay(pdMS_TO_TICKS(2000));
        state = Digi360;
    break;

    case Digi360:
        lcdSendCommand(lcdClearDisplay,portMAX_DELAY);
        lcdSendWriteString("Turn ",portMAX_DELAY);
        if(EncodeDirection)
        {
            lcdSendWriteString("right!",portMAX_DELAY);
        }
        else
        {
            lcdSendWriteString("left!",portMAX_DELAY);
        }




        

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
    lcdSendMoveCursor(0,1,5);
    

    while(1)
    {

        state = vElevatorState(state);

    }

}
