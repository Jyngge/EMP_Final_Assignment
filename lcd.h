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
#define RESET_DISPLAY           0x30        
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

typedef void (*FunctionPointer_t)(void *,void *);
typedef struct
{
    FunctionPointer_t pvFunction;
    void *pvParameter1;
    void *pvParameter2;
}LcdFunction_t;


/***************** Functions ******************/
<<<<<<< HEAD

void xPutLcdFunctionQueue(void *pvFunction, void *pvParameter1, void *pvParameter2);

void vLcdInit(void);
void vLcdCharecterWrite(INT8U character);
void vLcdControlWrite(INT8U *instruction);
void vLcdClearDisplay(void);
void vLcdHome(INT8U page);
void vLcdMoveCursor(INT8U x , INT8U y);
void vLcdShiftDisplayRight(INT8U shift);
void vLcdShiftDisplayLeft(INT8U shift);
void vLcdStringWrite(INT8U* charPTR);
void vlcdCursorOn(void);
void vlcdtestTask(void *pvParameters);



=======
void lcd_char_write(INT8U *character);
void lcd_ctrl_write(INT8U *instruction);
void lcd_init_function(void);
void lcd_clear_display(void);
void lcd_home(INT8U page);
void lcd_cursor_position(INT8U *x, INT8U *y);
void lcd_shift_display_right(INT8U shift);
void lcd_shift_display_left(INT8U shift);
void lcd_cursor_blink_on(void);
void lcd_string_write(INT8U* charPTR);
void lcd_time_write(INT8U* stringBuffer);
void lcd_time_alive(INT8U i,INT8U state);
void xPutLcdFunctionQueue(void *pvFunction, void *pvParameter1, void *pvParameter2);
void vLCDTask(void *pvParameters);
>>>>>>> 1a80f9f8fb1b2ea3a51933d18b18e4de410c9a53

#endif /* LCD_H_ */
