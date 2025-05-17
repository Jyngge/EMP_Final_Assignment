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

#define ENCODER_FLOOR_SELECT    0x01
#define ENCODER_360             0x02
#define ENCODER_EVENT_FLAGS     0x03


TaskHandle_t xDigiSwitchTaskHandle;
INT16S postion = 2;
INT16U A = 0;
INT16U B = 0;
INT16U dir = 0;
extern QueueHandle_t xLcdFunctionQueue;
extern EventGroupHandle_t xUIEventGroup;
extern INT16U ulTargetFloor;

// object based drawing

void vDigiswitchInit(void)
{
    GPIO_PORTA_DEN_R |= PIN5 | PIN6; // enable pins 5 and 6 on port A
    GPIO_PORTA_DIR_R &= ~(PIN5 | PIN6); // set pins 5 and 6 on port A to input
    GPIO_PORTA_PUR_R |= PIN5 | PIN6; // enable pull-up resistors on pins 5 and 6
    GPIO_PORTA_IS_R &= ~(PIN5); // set pins 5 and 6 on port A to edge-sensitive
    GPIO_PORTA_IBE_R |= PIN5;
    GPIO_PORTA_IM_R |= PIN5; // enable interrupts on pins 5
    GPIO_PORTA_ICR_R |= PIN5; // clear interrupt flag for pin 5

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

void vRoteryEncoderResume(void)
{
    GPIO_PORTA_ICR_R |= PIN5;
    GPIO_PORTA_IM_R |= PIN5;
    vTaskResume(xDigiSwitchTaskHandle);
}

void vRoteryEncoderSuspend(void)
{
    GPIO_PORTA_IM_R &= ~(PIN5);
    vTaskSuspend(xDigiSwitchTaskHandle);
}


void digiswitchInterruptHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    A = sReadA();   // read pin 5
    B = sReadB();   // read pin 6
    if(A == B)      // check if interrupt was triggered by pin 5
        postion--;
    else
        postion++;
        
    xTaskNotifyFromISR(xDigiSwitchTaskHandle,NULL,eIncrement,&xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    GPIO_PORTA_ICR_R = PIN5;
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
    
    static INT8U buffer[5];
    buffer[4] = '\0';
    INT16S displayPosition = postion; // output value counting to positon value
    
    while (1)
    {
        

        xEventGroupValue = xEventGroupWaitBits(xUIEventGroup,xBitsToWaitFor,pdFALSE,pdFALSE,portMAX_DELAY);
        xTaskNotifyStateClear(xDigiSwitchTaskHandle);
        ulTaskNotifyTake(pdFALSE , portMAX_DELAY);

        if(xEventGroupValue & ENCODER_FLOOR_SELECT)
        {
            if(postion == 13)
                postion++;
            if(postion < 0 )
                postion = 0;
            if(postion > 20 )
                postion = 20;
        }

        if(xEventGroupValue & ENCODER_360)
        {
            if(postion < 0 )
                postion = 0;
            if(postion > 360 )
                postion = 360;
            
        }
            
        if(displayPosition < postion)
        {
            displayPosition++;
        }
        if(displayPosition > postion)
        {
            displayPosition--;
        }

        vIntToString(buffer,displayPosition);
        lcdSendMoveCursor(1,1,portMAX_DELAY);
        lcdSendWriteString(buffer,2);
    }
}


