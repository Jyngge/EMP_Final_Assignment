/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: digiswitch.c
*
* PROJECT....: Final assignment
*
* DESCRIPTION: 
*
* Change Log:
******************************************************************************
* Date    Id          Change
* 100525  Majur22     Module created.
*
*****************************************************************************/

#include "digiswitch.h"
#include "lcd.h"
#include <limits.h>
#include "timers.h"


INT16S postion = 2;
INT16U A = 0;
INT16U B = 0;
INT16U dir = 0;
const TickType_t debounceDelay = pdMS_TO_TICKS(25);
TimerHandle_t xDigiSwitchTimeOutTimer;
TaskHandle_t xDigiSwitchTaskHandle;
extern QueueHandle_t xLcdFunctionQueue;
extern TaskHandle_t xButtonTaskHandle_DIGI;
extern EventGroupHandle_t xUIEventGroup;
extern INT16U ulTargetFloor;
extern BOOLEAN EncodeDirection;

// object based drawing

void vDigiswitchInit(void)
{
    GPIO_PORTA_DEN_R |= PIN5 | PIN6 | PIN7; // enable pins 5 and 6 on port A
    GPIO_PORTA_DIR_R &= ~(PIN5 | PIN6 | PIN7); // set pins 5 and 6 on port A to input
    GPIO_PORTA_PUR_R |= PIN5 | PIN6 | PIN7; // enable pull-up resistors on pins 5 and 6
    GPIO_PORTA_IS_R &= ~(PIN5 | PIN7); // set pins 5 and 6 on port A to edge-sensitive
    GPIO_PORTA_IBE_R |= PIN5;
    GPIO_PORTA_IBE_R &= ~PIN7;
    GPIO_PORTA_ICR_R |= PIN5 | PIN7; // clear interrupt flag for pin 5
    GPIO_PORTA_IM_R |= PIN5 | PIN7; // enable interrupts on pins 5
    
    NVIC_EN0_R |= (1 << 0); // enable interrupt for GPIO Port A in NVIC
}
INT16U sReadA()
{
   return GPIO_PORTA_DATA_R & PIN5;
}

INT16U sReadB()
{
    return (GPIO_PORTA_DATA_R & PIN6) >> 1;
}


void vDigiswitchCallBack(TimerHandle_t xTimer){
    lcdSendMoveCursor(0,0,portMAX_DELAY);
    if(EncodeDirection)
    {
        lcdSendWriteString("Turn LEFT!     ",portMAX_DELAY);
    }
    else
    {
        lcdSendWriteString("Turn RIGHT!    ",portMAX_DELAY);
    }
    

}

void digiswitchInterruptHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    static TickType_t lastInterruptTime = 0;
    TickType_t currentTime;


    switch(GPIO_PORTA_MIS_R)
    {
        case PIN5:
        currentTime = xTaskGetTickCountFromISR();
        if ((currentTime - lastInterruptTime) >= debounceDelay)
        {
            lastInterruptTime = currentTime;

            A = sReadA();   // read pin 5
            B = sReadB();   // read pin 6
            if(A == B)
                postion--;
            else
                postion++;

            xTaskNotifyFromISR(xDigiSwitchTaskHandle,NULL,eIncrement,&xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        GPIO_PORTA_ICR_R = PIN5;
        break;

        case PIN7:
        GPIO_PORTA_ICR_R = PIN7;
        GPIO_PORTA_IM_R &= ~PIN7;

        vTaskNotifyGiveFromISR(xButtonTaskHandle_DIGI, &xHigherPriorityTaskWoken)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        default:
        GPIO_PORTA_ICR_R = PIN5 + PIN7;

    }
    
    
}


void vIntToString(INT8U *buffer,INT16U displayPosition)
{
    INT16U temp = displayPosition;
    INT16U i = 0;
    for(i = 0; i < 4; i++)
    {
        buffer[i] = (temp % 10) + '0'; // convert to ASCII
        temp /= 10;
    }

    //or(i = 0; i < 4; i++)
    //{   
    //    buffer[0 + i] = tempBuffer[3-i];
    //}
   
}


void vDigiswitchTask(void *pvParameters)
{
    
    EventBits_t xEventGroupValue;
    const EventBits_t xBitsToWaitFor = ( ENCODER_360 | ENCODER_FLOOR_SELECT );
    xDigiSwitchTimeOutTimer = xTimerCreate( "DigiErrorTimeout", pdMS_TO_TICKS( 1000 ), pdFALSE, NULL, vDigiswitchCallBack);
    static const INT8U *pcFloor[20] = {" 0"," 1"," 2"," 3"," 4"," 5"," 6"," 7"," 8"," 9","10","11","12","14","15","16","17","18","19","20"};
    static const INT8U *pcDegree[30] = {
    " 12\xDF", " 24\xDF", " 36\xDF", " 48\xDF", " 60\xDF",
    " 72\xDF", " 84\xDF", " 96\xDF", "108\xDF", "120\xDF",
    "132\xDF", "144\xDF", "156\xDF", "168\xDF", "180\xDF",
    "192\xDF", "204\xDF", "216\xDF", "228\xDF", "240\xDF",
    "252\xDF", "264\xDF", "276\xDF", "288\xDF", "300\xDF",
    "312\xDF", "324\xDF", "336\xDF", "348\xDF", "360\xDF"
};
    INT16S displayPosition = postion; // output value counting to positon value
    
    while (1)
    {
        

        xEventGroupValue = xEventGroupWaitBits(xUIEventGroup,xBitsToWaitFor,pdFALSE,pdFALSE,portMAX_DELAY);
        xTaskNotifyStateClear(xDigiSwitchTaskHandle);
        ulTaskNotifyTake(pdFALSE , portMAX_DELAY);
        
        lcdSendMoveCursor(1,1,portMAX_DELAY);
        

        if(xEventGroupValue & ENCODER_FLOOR_SELECT)
        {
            if(postion < 0 )
                postion = 0;
            if(postion > 19 )
                postion = 19;
            
            if(displayPosition < postion)
            {
                displayPosition++;
            }
            if(displayPosition > postion)
            {
                displayPosition--;
            }
            lcdSendWriteString(pcFloor[displayPosition],2);
        }

        if(xEventGroupValue & ENCODER_360)
        {
            if(postion < 0 )
                postion = 0;
            if(postion > 29 )
                postion = 29;

            if(EncodeDirection)
            {
                if(displayPosition > postion)
                {
                    displayPosition--;
                }
                if(displayPosition < postion)
                {

                    xTimerStop(xDigiSwitchTimeOutTimer,0);
                    lcdSendMoveCursor(0,0,portMAX_DELAY);
                    lcdSendWriteString("wrong way mate!",2);
                    lcdSendMoveCursor(0,1,portMAX_DELAY);
                    postion = displayPosition;
                    xTimerStart(xDigiSwitchTimeOutTimer,0);
                }
                else
                {
                    lcdSendWriteString(pcDegree[displayPosition],2);
                }
            }
            else
            {
                if(displayPosition < postion)
                {
                    displayPosition++;
                }
                if(displayPosition > postion)
                {
                    
                    xTimerStop(xDigiSwitchTimeOutTimer,0);
                    lcdSendMoveCursor(0,0,portMAX_DELAY);
                    lcdSendWriteString("wrong way mate!",2);
                    lcdSendMoveCursor(0,1,portMAX_DELAY);
                    postion = displayPosition;
                    xTimerStart(xDigiSwitchTimeOutTimer,0);
                }
                else
                {
                    lcdSendWriteString(pcDegree[displayPosition],2);
                }
                if(postion == 29)
                {   
                    xEventGroupClearBits(xUIEventGroup,ENCODER_360);
                    xEventGroupSetBits(xUIEventGroup,ENCODER_TURN_COMPLET);
                }
            }
            
            
            
        }

        
        
        
    }
}


