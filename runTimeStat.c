/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: runTimeStat.hc
*
* PROJECT....: Final assignment
*
* DESCRIPTION: For Debugging purposes, this module is used to measure the time
*              taken by each task.
*
* Change Log:
******************************************************************************
* Date    Id          Change
* 110525  Majur22     Module created.
* 
*****************************************************************************/

/***************** Include files **************/
#include "runTimeStat.h"

/***************** Defines ********************/

#define MAX_TASK_NAME_LENGTH 20

/***************** Constants ******************/

/***************** Variables ******************/

/***************** Function *******************/

static void prvStatsTask( void *pvParameters )
{
    UBaseType_t xStatus;
    TickType_t xLastExecutionTime;
    TaskStatus_t xTaskStatusArray[MAX_TASKS]; 
    TickType_t xBlockTime = pdMS_TO_TICKS(1000);
    xLastExecutionTime = xTaskGetTickCount();

    while(1)
    {
        xTaskDelayUntil( &xLastExecutionTime, xBlockTime );
        xStatus = uxTaskGetSystemState( xTaskStatusArray, uxTaskGetNumberOfTasks(), NULL );
        if(xStatus == pdFALSE)
        {
            lcd_string_write("Failed to get task status");
            while(1);
        }
        // pass over UART
    }

}
