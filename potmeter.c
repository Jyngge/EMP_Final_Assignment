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
#include <stdio.h>
#include "potmeter.h"
#include "event_groups.h"
#include "digiswitch.h"
#define PIN5    0x20
#define GPIO_LOCK_KEY           0x4C4F434B

float voltage = 0;
QueueHandle_t xStringQueue;
extern QueueHandle_t xControlQueue;
extern EventGroupHandle_t xUIEventGroup;
TaskHandle_t xPotMatchTaskHandle;
extern INT16U potTargetValue;
extern TaskHandle_t xMovingStateHandler; // UI task handle


void vPotmeterInit(void)
{

    GPIO_PORTB_AFSEL_R |= PIN5;
    GPIO_PORTB_AMSEL_R |= PIN5;
    GPIO_PORTB_DEN_R &= ~PIN5;
    GPIO_PORTB_DIR_R &= ~PIN5;
    GPIO_PORTB_PUR_R &= ~PIN5;
    GPIO_PORTB_PDR_R &= ~PIN5;
    GPIO_PORTB_IM_R &= ~PIN5;
}



void vPotMatchTask(void *pvParameters)
{
    INT16U adcValue;
    char buffer[6];
    buffer[5] = '\0';
    INT16U voltageTenths;
    while(1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        while (1)
        {

            adcValue = get_adc();
            voltage = (adcValue / 4095.0f) * 3.3f;
            // Optionally, show the current value on the LCD for feedback
            
            int v_int = (int)voltage;
            int v_dec = (int)((voltage - v_int) * 10 + 0.5f);
            snprintf(buffer, sizeof(buffer), "%d.%dV ", v_int, v_dec);
            lcdSendMoveCursor(8, 1, portMAX_DELAY);
            lcdSendWriteString((INT8U*)buffer, portMAX_DELAY);
            
            voltageTenths = (INT16U)(voltage * 10.0f + 0.5f); // <-- Add this line
            if (voltageTenths == potTargetValue)
            {
                // Notify UI task that the value matches
                xTaskNotifyGive(xMovingStateHandler);
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(300));
        }
    }
}




