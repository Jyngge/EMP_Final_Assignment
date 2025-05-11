/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: LCD_functions.h
*
* PROJECT....: Assignment 3
*
* DESCRIPTION:
*
* Change Log:
******************************************************************************
* Date    Id    Change
* 040325  MA    Module created.
*
*****************************************************************************/

#ifndef LCD_H_
#define LCD_H_
#include "emp_type.h"
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

typedef void (*FunctionPointer_t)(INT8U *);
typedef struct 
{
    FunctionPointer_t pvFunction;
    INT8U *pvParameter;
}LcdFunction_t;


/***************** Functions ******************/
void lcd_char_write(INT8U character);
void lcd_ctrl_write(INT8U instruction);
void lcd_init_function(void);
void lcd_clear_display(void);
void lcd_home(INT8U page);
void lcd_cursor_position(INT8U x, INT8U y);
void lcd_shift_display_right(INT8U shift);
void lcd_shift_display_left(INT8U shift);
void lcd_cursor_blink_on(void);
void lcd_string_write(INT8U* charPTR);
void lcd_time_write(INT8U* stringBuffer);
void lcd_time_alive(INT8U i,INT8U state);
void vLCDTask(void *pvParameters);

#endif /* LCD_H_ */
