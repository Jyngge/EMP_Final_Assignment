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
* 100525  Majur22     Module created.
*
*****************************************************************************/
/***************** Include files **************/
#include "potmeter.h"
#define PIN5    0x20

TaskHandle_t xPotValueHandler;

QueueHandle_t xStringQueue;
extern QueueHandle_t xControlQueue;

extern TaskHandle_t xMovingStateHandler;



void vPotmeterInit(void)
{
    GPIO_PORTB_DEN_R &= PIN5;
    GPIO_PORTB_DIR_R &= PIN5;
    GPIO_PORTB_PUR_R &= PIN5;
    GPIO_PORTB_PDR_R &= PIN5;
    GPIO_PORTB_IM_R &= PIN5;
}


void vPotmeterTask(void *pvParameters)
{

    BaseType_t xStatus;
    INT16U sPotValue;
    INT16U sCompValue;
    
    INT8U controlMessage;
    INT8U *stringMessage;
    while(1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        sPotValue = get_adc();
        vTaskDelay(pdMS_TO_TICKS(200));
        xQueueSend( xStringQueue,&sPotValue,portMAX_DELAY);


    }
}





