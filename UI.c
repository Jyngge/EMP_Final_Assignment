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
#include <stdlib.h>
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
#include "LED_Control.h"
#include "adc.h"
#include "potmeter.h"
#include "UI.h"
/***************** Defines ********************/
#define PASSWORD_LENGTH 4
#define ARROW_UP        0x18
#define ARROW_DOWN      0x19


extern QueueHandle_t xKeypadQueue;
extern QueueHandle_t xLcdFunctionQueue;
extern QueueHandle_t xButtonEventQueue_SW4;
extern QueueHandle_t xButtonEventQueue_SW0;
extern QueueHandle_t xButtonEventQueue_DIGI;
extern QueueHandle_t xLedStatusQueue;
extern QueueHandle_t xStringQueue;
extern TaskHandle_t xElevatorStatusHandler;
extern TaskHandle_t xPotMatchTaskHandle;

ElevatorState_t state = idle;
TaskHandle_t xMovingStateHandler;

EventGroupHandle_t xUIEventGroup;
BOOLEAN EncodeDirection = 0;
INT16U ulCurrentFloor = 2;
INT16U ulTargetFloor = 0;
INT16U ulTripCount;
INT16U ulValueMatch;
INT8U floorStr[3];
INT16U potTargetValue;
float targetVoltage;
extern INT16S postion;
extern INT16S displayPosition;
extern TaskHandle_t xDigiSwitchTaskHandle;

LedStatusMessage QueueLED;

void vCurrentFloorToString(INT8U *buffer, INT16U ulCurrentFloor)
{
    if (ulCurrentFloor < 10)
    {
        buffer[0] = ' ';
        buffer[1] = (INT8U)('0' + ulCurrentFloor);
        buffer[2] = '\0';
    }
    else
    {
        buffer[0] = (INT8U)('0' + (ulCurrentFloor / 10));
        buffer[1] = (INT8U)('0' + (ulCurrentFloor % 10));
        buffer[2] = '\0';
    }
}

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


void vUITask(void)
/**********************************************
 * Input    :
 * Output   :
 * Function : Task to handle UI updates and interactions
 **********************************************/
{

    uint32_t NotificationValue = 0;
    srand(xTaskGetTickCount());
    lcdSendCommand(lcdClearDisplay,2);
    lcdSendWriteString("Floor: ",2);
    lcdSendWriteChar((INT8U)(ulCurrentFloor + '0'),2);
    lcdSendMoveCursor(0,1,5);
    static BaseType_t xStatus;
    static INT8U ucKeypadPress;
    static INT8U ucButtonPress;
    static INT8S i;
    static INT16U distance;
    static INT8U ucPasswordBuffer[4];
    xQueueReset(xKeypadQueue);
    while(1)
    {

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
        vTaskDelay(pdMS_TO_TICKS(200));
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
        postion = ulCurrentFloor;
        xQueueReset(xButtonEventQueue_DIGI);
        xQueueReceive(xButtonEventQueue_DIGI,&ucButtonPress,portMAX_DELAY);
        ulTargetFloor = postion;
        state = moving;

    break;

    case moving:

        lcdSendCommand(lcdClearDisplay,2);
        lcdSendMoveCursor(0,0,5);

        QueueLED.floors_moving = ulTargetFloor;    //magic number
        QueueLED.door_state = 0;

        xQueueSend(xLedStatusQueue, &QueueLED, 10);         // notify led task and wait for respons
        xTaskNotifyGive(xElevatorStatusHandler);

            if(ulTargetFloor > ulCurrentFloor)
            {
                distance = ulTargetFloor - ulCurrentFloor;
                lcdSendWriteString("Going up",2);
                lcdSendMoveCursor(0,1,2);
                lcdSendWriteString("Floor:",2);
                lcdSendMoveCursor(6,1,2);
            }
            else
            {
                distance = ulCurrentFloor - ulTargetFloor;
                lcdSendWriteString("Going down",2);
                lcdSendMoveCursor(0,1,2);
                lcdSendWriteString("Floor:",2);
                lcdSendMoveCursor(6,1,2);
            }

            while(ulCurrentFloor != ulTargetFloor)
            {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            xQueueReceive(xLedStatusQueue, &QueueLED,10);
            ulCurrentFloor = QueueLED.Return_Value;
            lcdSendMoveCursor(6,1,2);
            if(ulCurrentFloor > 9){                                     //Martin i know theres a vIntToString but i can't make it work
                int ulCurrentFloorOverflow1 = ulCurrentFloor / 10;
                int ulCurrentFloorOverflow2 = ulCurrentFloor % 10;
                lcdSendWriteChar((INT8U)(ulCurrentFloorOverflow1 + '0'),2);
                lcdSendMoveCursor(7,1,2);
                lcdSendWriteChar((INT8U)(ulCurrentFloorOverflow2 + '0'),2);
            }
            else{
            lcdSendWriteChar((INT8U)(ulCurrentFloor + '0'),2);
            }
            }

            if(ulCurrentFloor == ulTargetFloor)//(ulTripCount == 4)
            {
                state = broken;
            }
            if(ulCurrentFloor == 21 ) //ulTargetFloor)
            {
                state = idle;
                lcdSendCommand(lcdClearDisplay,portMAX_DELAY);
                lcdSendWriteLiteralString("FLOOR:",portMAX_DELAY);
                vCurrentFloorToString(floorStr, ulCurrentFloor);
                lcdSendWriteString(floorStr, portMAX_DELAY);
                lcdSendMoveCursor(0,1,portMAX_DELAY);
            }
            lcdSendMoveCursor(0,1,5);

    break;
    case broken:
        QueueLED.door_state = 1;

        xQueueSend(xLedStatusQueue, &QueueLED, 10);         // notify led task and wait for respons
        xTaskNotifyGive(xElevatorStatusHandler);
        
        lcdSendCommand(lcdClearDisplay,portMAX_DELAY);
        lcdSendWriteString("Elavator broken!",2);
        vTaskDelay(pdMS_TO_TICKS(2000));

        state = pot;
    break;
    case pot:
        state = Digi360;
        break;
    
    potTargetValue = (INT16U)(rand() % 34);
    
    lcdSendCommand(lcdClearDisplay, portMAX_DELAY);
    lcdSendWriteString("Match value:", portMAX_DELAY);

    // Display the target value
    char buffer[8];
    int v_int = potTargetValue / 10;      // whole volts
    int v_dec = potTargetValue % 10;      // tenths
    snprintf((char*)buffer, sizeof(buffer), "%d.%dV", v_int, v_dec);
    lcdSendMoveCursor(0, 1, portMAX_DELAY);
    lcdSendWriteString(buffer, portMAX_DELAY);

    // Notify the potmeter match task to start
    xEventGroupClearBits(xUIEventGroup,ENCODER_SELECT_COMPLET);
    ulTaskNotifyTake(pdTRUE, 0);
    xTaskNotifyGive(xPotMatchTaskHandle);
    ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
    //NotificationValue = ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
    state = Digi360;
    break;

    case Digi360:

        postion = 0;
        displayPosition = 0;
        xEventGroupClearBits(xUIEventGroup,ENCODER_TURN_COMPLET);
        lcdSendCommand(lcdClearDisplay,portMAX_DELAY);
        lcdSendWriteString("Turn ",portMAX_DELAY);
        if(EncodeDirection)
        {
            lcdSendWriteString("left!",portMAX_DELAY);
        }
        else
        {
            lcdSendWriteString("right!",portMAX_DELAY);
        }
        lcdSendMoveCursor(0,1,portMAX_DELAY);
        
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        lcdSendMoveCursor(0,0,portMAX_DELAY);
        lcdSendWriteString("Press the Encoder",portMAX_DELAY);
        xQueueReset(xButtonEventQueue_DIGI);
        xQueueReceive(xButtonEventQueue_DIGI,&ucButtonPress,portMAX_DELAY);
        EncodeDirection ^= 1;
        lcdSendCommand(lcdClearDisplay,portMAX_DELAY);
        lcdSendWriteString("FLOOR:",portMAX_DELAY);
        vCurrentFloorToString(floorStr, ulCurrentFloor);
        lcdSendWriteString(floorStr, portMAX_DELAY);

        lcdSendMoveCursor(0,1,portMAX_DELAY);
        postion = ulCurrentFloor;
        displayPosition = ulCurrentFloor;
        state = idle;
    break;
    }

    }

}
