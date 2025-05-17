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

TaskHandle_t xDigiSwitchTaskHandle;
INT16S postion = 2;
INT16U A = 0;
INT16U B = 0;
INT16U dir = 0;

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
    BOOLEAN

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



void sIntToString(INT8U *buffer,INT16U displayPosition)
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
    static INT8U buffer[5];
    
    buffer[4] = '\0';
    INT16S displayPosition = postion; // output value counting to positon value
    
    while (1)
    {
        ulTaskNotifyTake(pdFALSE , portMAX_DELAY);
        if(displayPosition < postion)
        {
            displayPosition++;
        }
        if(displayPosition > postion)
        {
            displayPosition--;
        }
        sIntToString(buffer,displayPosition);

        // need to take a mutex
        lcdSendMoveCursor(1,1,2);
        lcdSendWriteString(buffer,2);

            
    }
}


