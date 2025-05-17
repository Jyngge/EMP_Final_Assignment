/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: lcd.c
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


/***************** Include files **************/

#include <lcd.h>



/***************** Defines ********************/
#define object_max_size 15




/***************** Constants ******************/

static INT8U LCD_init_sequence[] =
{
    RESET_DISPLAY,       
    RESET_DISPLAY,       
    RESET_DISPLAY,       
    SET_4BIT_MODE,
    SET_4BIT_MODE + SET_2_LINE_DISPLAY,
    SET_DISPLAY_MODE,
    SET_CURSOR_INCREMENT_RIGHT,
    CLEAR_DISPLAY,    
    HOME,             
    SEQUENCE_TERMINATOR
};

/***************** Variables ******************/
SemaphoreHandle_t xLcdQueueMutex;
QueueHandle_t xLcdFunctionQueue;
INT8U incrementMode = 1;

/***************** Functions ******************/

void vLcdInit(void)
/**********************************************
 * Input    :
 * Output   :
 * Function : Initialize the LCD display, section set critical.
 **********************************************/
{
    vTaskSuspendAll();
    INT16U i = 0;
    INT16U j = 0;
    while (LCD_init_sequence[i] != SEQUENCE_TERMINATOR)
    {
        vLcdControlWrite(LCD_init_sequence[i++]);
        for(j = 0; j<3000;j++);
    }
    xTaskResumeAll();
}

void vLcdCharecterWrite(INT8U character)
/**********************************************
 * Input    : ASCII character
 * Output   :
 * Function : write character to current cursor position
 **********************************************/
{  
    TickType_t xLastWakeTime;

    GPIO_PORTC_DATA_R = character & 0xF0;;  // Send high nibble
    GPIO_PORTD_DATA_R |= (1<<2);            // Select DR Register, write
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low

    GPIO_PORTC_DATA_R = character << 4;;    // Send low nibble
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low
    
    xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(25));
    
}


void vLcdControlWrite(INT8U instruction)
/**********************************************
 * Input    : Control instruction. Refer to command defines in header file
 * Output   :
 * Function : Load instruction in the IR register
 **********************************************/
{
    TickType_t xLastWakeTime; 

    GPIO_PORTD_DATA_R &= ~(1 << 2);         // Select DR Register, write

    GPIO_PORTC_DATA_R = instruction & 0xF0;;
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low

    GPIO_PORTC_DATA_R = instruction << 4;;
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low

    xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(25));
}


void vLcdClearDisplay(void)
/**********************************************
 * Input    :
 * Output   :
 * Function : Clear display
 **********************************************/
{
    vLcdControlWrite((INT8U)CLEAR_DISPLAY);
}

void vLcdHome()
/**********************************************
 * Input    :
 * Output   :
 * Function : Moves cursor to position 0
 **********************************************/
{
    vLcdControlWrite(HOME);
}


void vLcdMoveCursor(INT8U x , INT8U y)
/**********************************************
 * Input    : Target position
 * Output   :
 * Function : Moves cursor to target position from current position.
 *            Line 1: 00 01 02 03... 39
 *            line 2: 40 41 42 43... 79
 **********************************************/
{
    INT8U target_position = (x)+(y)*0x28;
    vLcdControlWrite(0x80 | target_position);
}


void vLcdShiftDisplayRight(INT8U shift)
/**********************************************
 * Input    : Shift amount
 * Output   :
 * Function : Shift display right
 **********************************************/
{
    while (shift--)
        vLcdControlWrite(SHIFT_DISPLAY_RIGHT);   
}
void vLcdShiftDisplayLeft(INT8U shift)
/**********************************************
 * Input    : Shift amount
 * Output   :
 * Function : Shift display left
 **********************************************/
{
    while (shift--)
        vLcdControlWrite(SHIFT_DISPLAY_LEFT);
}

void lcd_cursor_on(void)
/**********************************************
 * Input    :
 * Output   :
 * Function : Display cursor
 **********************************************/
{
    vLcdControlWrite(0x0E);
}

void vLcdStringWrite(INT8U* charPTR)
/**********************************************
 * Input    : Character array. needs termination character
 * Output   :
 * Function : Writes character array to LCD display at current cursor position
 **********************************************/
{

    int i =0;
    while (*(charPTR+i) != '\0')
    {
        vLcdCharecterWrite(*(charPTR+(i++)));
    }
}
void lcdSendCommand(LcdCommand_t cmd, TickType_t ticksToWait)
/**********************************************
 * Input    : Enum command
 * Output   :
 * Function : lcd command API 
 **********************************************/
{
    LcdMessage_t msg;
    msg.cmd = cmd;
    xQueueSend(xLcdFunctionQueue, &msg, ticksToWait);
}


void lcdSendWriteString(const INT8U *str, TickType_t ticksToWait) 
/**********************************************
 * Input    : Heap alocated charecter pointer, Static charecter pointer,  string literal
 * Output   : 
 * Function : lcd String write API
 **********************************************/
{
    size_t len = strlen(str) + 1;
    INT8U *heapStr = (INT8U *)pvPortMalloc(len);
    if (heapStr == NULL) {
        vLcdMoveCursor(0,1);
        vLcdControlWrite(SET_CURSOR_INCREMENT_RIGHT);
        while(1)
        {
            vLcdStringWrite("MemoryError1");
        }
    }
    strcpy(heapStr, str);
    LcdMessage_t msg;
    msg.cmd = lcdWriteString;
    msg.params.string = heapStr;
    xQueueSend(xLcdFunctionQueue, &msg, ticksToWait);
}

void lcdSendMoveCursor(INT8U x, INT8U y, TickType_t ticksToWait) 
/**********************************************
 * Input    : LCD screen cordinates
 * Output   : 
 * Function : lcd move cursor move API
 **********************************************/
{
    LcdMessage_t msg;
    msg.cmd = lcdMoveCursor;
    msg.params.x = x;
    msg.params.y = y;
    xQueueSend(xLcdFunctionQueue, &msg, ticksToWait);
}

void lcdSendWriteChar(INT8U c, TickType_t ticksToWait) 
/**********************************************
 * Input    : ASCII charecter
 * Output   : 
 * Function : lcd charecter write API
 **********************************************/
{
    LcdMessage_t msg;
    msg.cmd = lcdWriteChar;
    msg.params.charecter = c;
    xQueueSend(xLcdFunctionQueue, &msg, ticksToWait);
}

void vLcdTaskTester(void *pvParameters)
/**********************************************
 * Input    : 
 * Output   : 
 * Function : LCD test task, pls delete.
 **********************************************/
{
    INT8U x = 0;
    INT8U *string = pvPortMalloc(sizeof(INT8U)*10);
    strcpy(string,"Heap!");
    while(1)
    {
        
        vTaskDelay(pdMS_TO_TICKS(2000));
        lcdSendCommand(lcdClearDisplay,portMAX_DELAY);
        lcdSendMoveCursor(x,0,portMAX_DELAY);
        lcdSendWriteString(string,portMAX_DELAY);
        lcdSendMoveCursor(x++,1,portMAX_DELAY);
        lcdSendWriteString("Literal!",portMAX_DELAY);
    }

}


void vLCDTask(void *pvParameters)
/**********************************************
 * Input    : 
 * Output   : 
 * Function : Gatekeeper task for LCD display. use API functions for communication.
 **********************************************/
{
    BaseType_t xStatus;
    LcdMessage_t instruction;
    vLcdInit();
    while(1)
    {
        xQueueReceive(xLcdFunctionQueue,&instruction,portMAX_DELAY);

        switch (instruction.cmd)
        {
        case lcdClearDisplay:
            vLcdClearDisplay();
        break;
        case lcdHome:
            vLcdHome();
        break;
        case lcdMoveCursor:
            vLcdMoveCursor(instruction.params.x, instruction.params.y);
        break;
        case lcdWriteChar:
            vLcdCharecterWrite(instruction.params.charecter);
        break;
        case lcdWriteString:
            vLcdStringWrite(instruction.params.string);
            if(instruction.params.string != NULL)
            {
                vPortFree(instruction.params.string);
            }
        break;
        case lcdIncrementCursorRight:
            vLcdControlWrite(SET_CURSOR_INCREMENT_RIGHT);
            incrementMode = 1;
        break;

        case lcdIncrementCursorLeft:
            vLcdControlWrite(SET_CURSO_INCREMENT_LEFT);
            incrementMode = 0;
            break;
        default:
            break;
        }
       
    }

}




