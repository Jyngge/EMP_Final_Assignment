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
#include <string.h>

/***************** Defines ********************/

#define MAX_FRAME_LENGTH    16
#define MAX_OBJECT_LENGTH   10
#define MAX_OBJECTS         3
typedef void (*UIPageFuncton_t)(void * pvParameter);

typedef struct
{
    INT16U x;
    INT16U y;
    INT8U ucBuffer[MAX_OBJECT_LENGTH];
    INT8U ucUpdateBuffer[MAX_OBJECT_LENGTH];
} UIObject_t;

typedef struct
{
    INT8U ucStaticString[MAX_FRAME_LENGTH];
    UIObject_t objectBuffer[MAX_OBJECTS];

}UIPage_t;


/***************** Constants ******************/
/***************** Variables ******************/

extern QueueHandle_t xKeypadQueue;
extern QueueHandle_t xLcdFunctionQueue;
extern QueueHandle_t xButtonEventQueue_SW4;
extern QueueHandle_t xButtonEventQueue_SW0;
extern INT8U cursor_position;



//typedef struct 
//{
//    INT8U *FrameBuffer[MAX_FRAME_NUMBER][MAX_FRAME_HIGHT][MAX_FRAME_LENGTH];
//    INT8U CurrentFrame;   
//} frameBuffer_t;
// function pointer typedefs


/***************** Functions ******************/
size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t srclen = strlen(src);

    if (size != 0)
    {
        size_t copylen = (srclen >= size) ? size - 1 : srclen;
        memcpy(dst, src, copylen);
        dst[copylen] = '\0';
    }

    return srclen;
}

void vUIUpdateObject(UIObject_t *object)
{

    xPutLcdFunctionQueue(lcd_cursor_position, &object->x, &object->y);
    xPutLcdFunctionQueue(lcd_string_write, object->ucUpdateBuffer, NULL);    
    strlcpy(object->ucBuffer, object->ucUpdateBuffer, MAX_OBJECT_LENGTH);
}

void vUIDrawPage(UIPage_t *page)
{  

    INT16U i;

    xPutLcdFunctionQueue(lcd_string_write, page->ucStaticString, NULL); // write the static string
    for(i = 0; i < MAX_OBJECTS; i++)
    {
        if(page->objectBuffer[i].ucBuffer[0] != '\0')
        {
            xPutLcdFunctionQueue(lcd_cursor_position, &(page->objectBuffer[i].x), &(page->objectBuffer[i]).y);
            xPutLcdFunctionQueue(lcd_string_write, page->objectBuffer[i].ucBuffer, NULL);
        }
    }
}


void vUIInit(void)
/**********************************************
 * Input    :    
 * Output   : 
 * Function : Initializes UI resources
 **********************************************/
{
    // Initialize the LCD and other UI components here if needed
    
}

void vUITask(void)
/**********************************************
 * Input    :    
 * Output   : 
 * Function : Task to handle UI updates and interactions
 **********************************************/
{
    BaseType_t xStatus;
    UIObject_t xObject1 = {0,1,"","Empty"};
    UIPage_t xInputPage = {"Input Page:", {xObject1}};
    button_event_t buttonPress;
    INT8U i = 1;
    xQueueReset(xButtonEventQueue_SW0);
    xQueueReset(xKeypadQueue);
    vUIDrawPage(&xInputPage);
    vUIUpdateObject(&xObject1);
    while(1)
    {
        while(buttonPress != BE_SINGLE_PUSH)
        {
            // Wait for an event from the button
            xStatus = xQueueReceive(xKeypadQueue, &xObject1.ucUpdateBuffer[MAX_OBJECT_LENGTH - i], pdMS_TO_TICKS(100));
            if ((xStatus == pdPASS) && (i < MAX_OBJECT_LENGTH))
            {
                if(i == MAX_OBJECT_LENGTH)
                {
                    i = 1;
                }
                i++;
                vUIUpdateObject(&xObject1);
            }
            xQueueReceive(xButtonEventQueue_SW0, &buttonPress, 0);
        }

        if(xObject1.ucBuffer == "1234")
        {
            // Password is correct, proceed to the next page
            // state change
            strncpy(xObject1.ucUpdateBuffer,"Accepted",8);
            vUIUpdateObject(&xObject1);
            vTaskDelay(pdMS_TO_TICKS(1000));
            xPutLcdFunctionQueue(lcd_clear_display, NULL, NULL);
        }
        else
        {
            strncpy(xObject1.ucUpdateBuffer,"Incorrect",8);
            vUIUpdateObject(&xObject1);
            vTaskDelay(pdMS_TO_TICKS(1000));
            xPutLcdFunctionQueue(lcd_clear_display, NULL, NULL);
        }
        
        
    }    

}
