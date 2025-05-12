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

/***************** Defines ********************/

#define MAX_FRAME_LENGTH    12
#define MAX_OBJECT_LENGTH   8
/***************** Constants ******************/
/***************** Variables ******************/
extern QueueHandle_t xButtonEventQueue;
extern QueueHandle_t xKeypadQueue;
extern QueueHandle_t xLcdFunctionQueue;
//typedef struct 
//{
//    INT8U *FrameBuffer[MAX_FRAME_NUMBER][MAX_FRAME_HIGHT][MAX_FRAME_LENGTH];
//    INT8U CurrentFrame;   
//} frameBuffer_t;

typedef struct 
{
    INT16U x;
    INT16U y;
    INT8U ucUpdateBuffer[MAX_OBJECT_LENGTH];
    INT8U ucBuffer[MAX_OBJECT_LENGTH];
} UIObject_t;

/***************** Functions ******************/




void vUIUpdateObject(UIObject_t *object, INT8U *ucStringBuffer)
{
    // mutex
    INT16U i;
    
    LcdFunction_t xLcdFunctionMoveCursor = {lcd_cursor_position, object->x, object->y};
    LcdFunction_t 
    QueueSend(xLcdFunctionQueue, &xLcdFunctionMoveCursor, portMAX_DELAY);

    for(i = 0; i < MAX_OBJECT_LENGTH; i++)
    {
        if(ucBuffer[i] == ucUpdateBuffer[i])
        {
            
            break;
        }
    }

}

void vDrawFrame(UIFrame_t *frame)
/**********************************************
 * Input    : frameNumber - The frame number to draw
 *            frameBuffer - The buffer containing the frame data
 * Output   : 
 * Function : Draws the specified frame on the LCD
 **********************************************/
{
    
    lcd_ctrl_write
    lcd_string_write(frame->frameBuffer[0]); // Write the first line of the frame
    
    lcd_command_write(0xC0);                 // Move the cursor to the second line
    lcd_string_write(frame->frameBuffer[0]); // Write the second line of the frame
    lcd_char_write(0x0D);                 // Move the cursor to the beginning of the line
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


}