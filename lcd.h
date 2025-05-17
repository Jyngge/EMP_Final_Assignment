/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: lcd.h
*
* PROJECT....: Assignment 3
*
* DESCRIPTION:
* https://github.com/Jyngge  - MAJUR
* Change Log:
******************************************************************************
* Date    Id    Change
* 040325  MAJUR Module created.
* 110325  MAJUR Removed functions which caused issues
* 140525  MAJUR Complet refactor of code to implement FreeRtos
* 160525  MAJUR Added API functions
*****************************************************************************/

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Display instructions
#define RESET_DISPLAY               0x30        
#define CLEAR_DISPLAY               0x01
#define HOME                        0x02
#define MOVE_CURSOR_RIGHT           0x14
#define MOVE_CURSOR_LEFT            0x10
#define SHIFT_DISPLAY_RIGHT         0x1C
#define SHIFT_DISPLAY_LEFT          0x18
#define SET_4BIT_MODE               0x20        // write 2 times to the IR reg
#define SET_2_LINE_DISPLAY          0x08        // most be used with the 2nd instance of the 4 bit_mode init
#define SET_DISPLAY_MODE            0x0C        // set display ON, Cursor OFF, Blink OFF
#define SET_CURSOR_INCREMENT_RIGHT  0x06
#define SET_CURSO_INCREMENT_LEFT    0x04

// Misc. defines
#define CHAR_CLEAR              0x20
#define CHAR_COLON              0x3A
#define CENTER_DISPLAY          0x04        // to center clock string on display
#define SEQUENCE_TERMINATOR     0xFF        // sequence terminator for init sequence


typedef enum lcdCommand
{
    lcdClearDisplay,
    lcdHome,
    lcdMoveCursor,
    lcdWriteChar,
    lcdWriteString,
    lcdIncrementCursorRight,
    lcdIncrementCursorLeft
} LcdCommand_t;

typedef struct {
    LcdCommand_t cmd;
    union 
    {
        INT8U* string;
        struct {INT8U x, y;};
        INT8U charecter;
        INT16U value;
    } params;
} LcdMessage_t;


/***************** Functions ******************/


void lcdSendWriteString(const INT8U *str, TickType_t ticksToWait);
void lcdSendMoveCursor(INT8U x, INT8U y, TickType_t ticksToWait);
void lcdSendWriteChar(INT8U c, TickType_t ticksToWait);
void lcdSendCommand(LcdCommand_t cmd, TickType_t ticksToWait);

//void vLcdInit(void);
//void vLcdCharecterWrite(INT8U character);
//void vLcdControlWrite(INT8U instruction);
//void vLcdClearDisplay(void);
//void vLcdHome();
//void vLcdMoveCursor(INT8U x , INT8U y);
//void vLcdShiftDisplayRight(INT8U shift);
//void vLcdShiftDisplayLeft(INT8U shift);
//void vlcdCursorOn(void);

void vLcdStringWrite(INT8U* charPTR);
void vLcdTaskTester(void *pvParameters);
void vLCDTask(void *pvParameters);


#endif /* LCD_H_ */
