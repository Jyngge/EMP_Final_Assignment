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


/***************** Functions ******************/
void lcd_char_write(INT8U character);
void lcd_ctrl_write(INT8U instruction);
void lcd_init_function(void);
void lcd_clear_display(void);
void lcd_home(void);
void lcd_cursor_position(INT8U position);
void lcd_shift_display_right(INT8U shift);
void lcd_shift_display_left(INT8U shift);
void lcd_cursor_blink_on(void);
void lcd_string_write(INT8U* charPTR);
void lcd_time_write(INT8U* stringBuffer);
void lcd_time_alive(INT8U i,INT8U state);
void vLCDTask(void *pvParameters);

#endif /* LCD_H_ */
