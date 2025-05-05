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
/***************** Defines ********************/

// Display instructions
#define CLEAR_DISPLAY           0x01
#define HOME                    0x02
#define MOVE_CURSOR_RIGHT       0x14
#define MOVE_CURSOR_LEFT        0x10
#define SHIFT_DISPLAY_RIGHT     0x1C
#define SHIFT_DISPLAY_LEFT      0x18
#define SET_4BIT_MODE           0x20        // write 2 times to the IR reg
#define SET_2_LINE_DISPLAY      0x08        // most be used with the 2nd instance of the 4 bit_mode init
#define SET_DISPLAY_MODE        0x0C        // set display ON, Cursor OFF, Blink OFF
#define SET_CURSOR_INCREMENT    0x06


// Misc. defines
#define CHAR_CLEAR              0x20
#define CHAR_COLON              0x3A
#define CENTER_DISPLAY          0x04        // to center clock string on display

#define SEQUENCE_TERMINATOR     0xFF        // sequence terminator for init sequence


/***************** Constants ******************/

const INT8U LCD_init_sequence[] =
{
  SET_4BIT_MODE,
  SET_4BIT_MODE + SET_2_LINE_DISPLAY,
  SET_DISPLAY_MODE,
  SET_CURSOR_INCREMENT,
  CLEAR_DISPLAY,    
  HOME,             
  SEQUENCE_TERMINATOR
};

/***************** Variables ******************/


INT8U cursor_position = 0;                  // tracks cursor position

QueueHandle_t xStringQueue;
QueueHandle_t xControlQueue;


/***************** Functions ******************/


void lcd_init_function(void)
/**********************************************
 * Input    :
 * Output   :
 * Function : Initialize the LCD display
 **********************************************/
{
    INT8U i = 0;

    while (LCD_init_sequence[i] != SEQUENCE_TERMINATOR)
        lcd_ctrl_write(LCD_init_sequence[i++]);
}

void lcd_char_write(INT8U character)
/**********************************************
 * Input    : ASCII character
 * Output   :
 * Function : write character to current cursor position
 **********************************************/
{
    char highbyte = character & 0xF0;
    char lowbyte = character << 4;

    cursor_position++;

    GPIO_PORTC_DATA_R = highbyte;
    GPIO_PORTD_DATA_R |= (1<<2);            // Select IR register
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low

    GPIO_PORTC_DATA_R = lowbyte;
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low
    
    vTaskDelay(5);       // wait 1 ms
}


void lcd_ctrl_write(INT8U instruction)
/**********************************************
 * Input    : Instruction. refer to defines
 * Output   :
 * Function : Load instruction in the IR register
 **********************************************/
{
    char lowByte = instruction << 4;
    char highByte = instruction & 0xF0;


    GPIO_PORTD_DATA_R &= ~(1 << 2);         // Select DR Register, write

    GPIO_PORTC_DATA_R = highByte;
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low

    GPIO_PORTC_DATA_R = lowByte;
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low

    vTaskDelay(5);       // wait 1 ms

}


void lcd_clear_display(void)
/**********************************************
 * Input    :
 * Output   :
 * Function : Clear display
 **********************************************/
{
    lcd_ctrl_write(CLEAR_DISPLAY);
}

void lcd_home(void)
/**********************************************
 * Input    :
 * Output   :
 * Function : Moves cursor to position 0
 **********************************************/
{
    lcd_ctrl_write(HOME);
    
    cursor_position = 0;
}

void lcd_cursor_position(INT8U position_target)
/**********************************************
 * Input    : Target position
 * Output   :
 * Function : Moves cursor to target position from current position
 **********************************************/
{

    if (position_target > cursor_position)
    {
        while (position_target != cursor_position)
        {
            lcd_ctrl_write(MOVE_CURSOR_RIGHT);
            cursor_position++;
        }
    }
    else
    {
        while (position_target != cursor_position)
        {
            lcd_ctrl_write(MOVE_CURSOR_LEFT);
            cursor_position--;
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
    lcd_cursor_position(CENTER_DISPLAY);
    lcd_string_write(stringBuffer);
    lcd_home();

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
        lcd_char_write(charPTR[i++]);
    }
    lcd_home();
}



void vLCDTask(void *pvParameters)
{
    INT8U *receivedString;
    INT8U receivedControl;
    BaseType_t xStatus;

    lcd_init_function(); // Initialize the LCD

    while (1)
    {
        // Check for control instructions
        xStatus = xQueueReceive(xControlQueue, &receivedControl, 0);
        if (xStatus == pdPASS)
        {
            lcd_ctrl_write(receivedControl); // Send control instruction to LCD
        }

        // Check for strings
        xStatus = xQueueReceive(xStringQueue, &receivedString, 0);
        if (xStatus == pdPASS)
        {
            lcd_string_write(receivedString); // Write the string to the LCD
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to avoid busy-waiting
    }
}
