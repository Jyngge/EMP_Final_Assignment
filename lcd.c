/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: LCD_functions.c
*
* PROJECT....: Assignment 3
*
* DESCRIPTION: 
*
* Change Log:
******************************************************************************
* Date    Id    Change
* 040325  MAJUR Module created.
* 110325  MAJUR removed functions which caused issues
*****************************************************************************/


/***************** Include files **************/

#include <lcd.h>
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/***************** Defines ********************/
#define object_max_size 16


typedef struct {
    INT16U x;
    INT16U y;
    INT8U ucUpdateBuffer[object_max_size];
    INT8U ucBuffer[object_max_size];
    INT16U ulSize;
} object_t;


/***************** Constants ******************/


static INT8U LCD_init_sequence[] =
{
    RESET_DISPLAY,       
    RESET_DISPLAY,       
    RESET_DISPLAY,       
    SET_4BIT_MODE,
    SET_4BIT_MODE + SET_2_LINE_DISPLAY,
    SET_DISPLAY_MODE,
    SET_CURSOR_INCREMENT,
    CLEAR_DISPLAY,    
    HOME,             
    SEQUENCE_TERMINATOR
};

/***************** Variables ******************/
SemaphoreHandle_t xLcdQueueMutex;
INT8U cursor_position = 0;
QueueHandle_t xLcdFunctionQueue;
INT8U uCurrentPage = 0;
INT8U LCDintialized = 0;

/***************** Functions ******************/


void xPutLcdFunctionQueue(void *pvFunction, void *pvParameter1, void *pvParameter2)
{
    LcdFunction_t instruction = {
    .pvFunction = (FunctionPointer_t) pvFunction,
    .pvParameter1 = pvParameter1,
    .pvParameter2 = pvParameter2
};

    xQueueSend(xLcdFunctionQueue, &instruction, portMAX_DELAY);
}


void lcd_init_function(void)
/**********************************************
 * Input    :
 * Output   :
 * Function : Initialize the LCD display
 **********************************************/
{
    INT16U i = 0;

    while (LCD_init_sequence[i] != SEQUENCE_TERMINATOR)
        lcd_ctrl_write(&LCD_init_sequence[i++]);
    for(i = 0; i<16000;i++);
    LCDintialized = 0;
}

void lcd_char_write(INT8U character)
/**********************************************
 * Input    : ASCII character
 * Output   :
 * Function : write character to current cursor position
 **********************************************/
{
    char highbyte = *character & 0xF0;
    char lowbyte = *character << 4;

    cursor_position++;

    GPIO_PORTC_DATA_R = highbyte;
    GPIO_PORTD_DATA_R |= (1<<2);            // Select IR register
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low

    GPIO_PORTC_DATA_R = lowbyte;
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low
    
    vTaskDelay(pdMS_TO_TICKS(25));
}


void lcd_ctrl_write(INT8U instruction)
/**********************************************
 * Input    : Instruction. refer to defines
 * Output   :
 * Function : Load instruction in the IR register
 **********************************************/
{
    char lowByte = *instruction << 4;
    char highByte = *instruction & 0xF0;


    GPIO_PORTD_DATA_R &= ~(1 << 2);         // Select DR Register, write

    GPIO_PORTC_DATA_R = highByte;
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low

    GPIO_PORTC_DATA_R = lowByte;
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low

    if(LCDintialized)
    {
        vTaskDelay(pdMS_TO_TICKS(25));
    }
    else
    {
        int i;
        for(i = 0; i<25000;i++);
    }
    
}


void lcd_clear_display(void)
/**********************************************
 * Input    :
 * Output   :
 * Function : Clear display
 **********************************************/
{
    lcd_ctrl_write((INT8U)CLEAR_DISPLAY);
}

void lcd_home(INT8U page)
/**********************************************
 * Input    :
 * Output   :
 * Function : Moves cursor to position 0
 **********************************************/
{
    if(page)
    {
        lcd_cursor_position(0x40,0);  // Move cursor home on page 1
        cursor_position = 0x40;  // Reset cursor position
    }
    else
    {
        lcd_ctrl_write(HOME);       // Move cursor to home on page 0
        cursor_position = 0;  // Reset cursor position
    }
    
}
void vLcdSwapPage(void)
/**********************************************
 * Input    : 
 * Output   :
 * Function : Swaps between page 1 and page 2
 *           (0x20 and 0x40)
 **********************************************/
{
    if(uCurrentPage){
        lcd_shift_display_left(0x20); // Shift display left to show page 1
        uCurrentPage = 0;
    } else {
        lcd_shift_display_right(0x20); // Shift display right to show page 2
        uCurrentPage = 1;
    }
}

void lcd_cursor_position(INT8U x , INT8U y)
/**********************************************
 * Input    : Target position
 * Output   :
 * Function : Moves cursor to target position from current position
 **********************************************/
{
    INT8U target_position = (*x)+(*y)*0x28;
    

    if(target_position < cursor_position)
    {
        while (cursor_position - target_position)
        {
            lcd_ctrl_write(MOVE_CURSOR_LEFT);
            cursor_position--;
        }
    }
    else if(target_position > cursor_position)
    {
        while (target_position - cursor_position)
        {
            lcd_ctrl_write(MOVE_CURSOR_RIGHT);
            cursor_position++;
        }
    }
   
}


void lcd_shift_display_right(INT8U shift)
/**********************************************
 * Input    : Shift amount
 * Output   :
 * Function : Shift display right
 **********************************************/
{
    while (shift--)
        lcd_ctrl_write(SHIFT_DISPLAY_RIGHT);   
}
void lcd_shift_display_left(INT8U shift)
/**********************************************
 * Input    : Shift amount
 * Output   :
 * Function : Shift display left
 **********************************************/
{
    while (shift--)
        lcd_ctrl_write(SHIFT_DISPLAY_LEFT);
}

void lcd_cursor_blink_on(void)
/**********************************************
 * Input    :
 * Output   :
 * Function : Display cursor
 **********************************************/
{
    lcd_ctrl_write(0x0E);
}

void lcd_time_write(INT8U* stringBuffer)
/**********************************************
 * Input    :
 * Output   :
 * Function : writes buffer to display. at center if buffer length = 8
 **********************************************/
{
    lcd_cursor_position(CENTER_DISPLAY,0);
    lcd_string_write(stringBuffer);
    lcd_home(0);

}



void lcd_string_write(INT8U* charPTR)
/**********************************************
 * Input    : Character array. needs termination character
 * Output   :
 * Function : Writes character array to LCD display at current cursor position
 **********************************************/
{

    int i =0;
    while (charPTR[i] != '\0')
    {
        lcd_char_write(charPTR + i++);

    }
    lcd_home(0); // Move cursor home after writing the string
}



void vLCDTask(void *pvParameters)
{
    LcdFunction_t instruction;
    BaseType_t xStatus;
    lcd_init_function(); // Initialize the LCD
    while (1)
    {
        xStatus = xQueueReceive(xLcdFunctionQueue, &instruction, portMAX_DELAY);
        if (xStatus == pdPASS)
        {
            instruction.pvFunction(instruction.pvParameter1, instruction.pvParameter2);
        }   
    }
}


typedef struct
{
    INT8U x;
    INT8U y;
    INT8U ucBuffer[8];
    INT8U ucUpdateBuffer[8];
} UIObject_t;



void lcdSendTask(void *pvParameters)
{
    UIObject_t *object = (UIObject_t *) pvPortMalloc(sizeof(UIObject_t));
    object->x = 0;
    object->y = 0;
    strcpy(object->ucBuffer, "Test");
    strcpy(object->ucUpdateBuffer, "LCD");

    INT8U testChar = 'A';
    INT8U testChar2 = 'B';

    while(1)
    {
        
        xPutLcdFunctionQueue(lcd_cursor_position, &object->x, &object->y);
        xPutLcdFunctionQueue(lcd_string_write, object->ucBuffer, NULL);;
        takeMutex(xLcdQueueMutex);
        vTaskDelay(pdMS_TO_TICKS(1000));

    }
}

void vLcdTestTask(void *pvParameters)
{
    UIObject_t object;
    object.x = 0;
    object.y = 0;

    while (1)
    {   
        
        lcd_clear_display();
        lcd_home(0);

        lcd_cursor_position(&object.x,&object.y);
        lcd_char_write("H");
        vTaskDelay(pdMS_TO_TICKS(1000));

        object.x = 2;
        lcd_cursor_position(&object.x,&object.y);
        lcd_char_write("o");
        vTaskDelay(pdMS_TO_TICKS(1000));

        object.y = 1;
        lcd_cursor_position(&object.x,&object.y);
        lcd_char_write("l");
        vTaskDelay(pdMS_TO_TICKS(1000));

        object.x = 0;
        lcd_cursor_position(&object.x,&object.y);
        lcd_char_write("a");
        vTaskDelay(pdMS_TO_TICKS(1000));

        lcd_clear_display();
        lcd_home(0);

        lcd_string_write("FreeRTOS");
        vTaskDelay(pdMS_TO_TICKS(1000));

    }
}
