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
* 110325  MAJUR Removed functions which caused issues
* 140525  MAJUR Complet refactor of code to implement FreeRtos
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
    SET_CURSOR_INCREMENT,
    CLEAR_DISPLAY,    
    HOME,             
    SEQUENCE_TERMINATOR
};

/***************** Variables ******************/
SemaphoreHandle_t xLcdQueueMutex;
INT8U cursor_position = 0;
QueueHandle_t xLcdFunctionQueue;

/***************** Functions ******************/
//typedef struct {
//    INT8U x;
//    INT8U y;
//    INT8U *ucUpdateBuffer;
//    INT8U *ucBuffer;
//} LcdStringObject_t;
//
//typedef struct {
//    INT16U value;
//    INT16U updateValue;
//    INT8U x;
//    INT8U y;
//} LcdIntObject_t;
//
//typedef LcdIntObject_t* LcdIntObjectHandle_t;
//typedef LcdStringObject_t* LcdStringObjectHandle_t;
//
//LcdStringObjectHandle_t xLcdStringObjectCreate(INT8U x, INT8U y, INT8U *ucBuffer)
//{
//    LcdStringObjectHandle_t pxLcdObject = (LcdStringObjectHandle_t) pvPortMalloc(sizeof(LcdStringObject_t));
//    if (pxLcdObject == NULL)
//    {
//        return NULL;
//    }
//
//    pxLcdObject->x = x;
//    pxLcdObject->y = y;
//    pxLcdObject->ucBuffer = (INT8U *) pvPortMalloc(object_max_size + 1);
//    pxLcdObject->ucUpdateBuffer = (INT8U *) pvPortMalloc(object_max_size + 1);
//
//    if (pxLcdObject->ucBuffer == NULL || pxLcdObject->ucUpdateBuffer == NULL)
//    {
//        vPortFree(pxLcdObject->ucBuffer);
//        vPortFree(pxLcdObject->ucUpdateBuffer);
//        vPortFree(pxLcdObject);
//        return NULL;
//    }
//
//    memset(pxLcdObject->ucBuffer, 0, object_max_size + 1);
//    memset(pxLcdObject->ucUpdateBuffer, 0, object_max_size + 1);
//
//    return pxLcdObject;
//}
//
//void xLcdStringObjectDelete(LcdStringObjectHandle_t pxLcdObject)
//{
//    vPortFree(pxLcdObject->ucBuffer);
//    vPortFree(pxLcdObject->ucUpdateBuffer);
//    vPortFree(pxLcdObject);
//}
//
//void vLcdStringObjectWrite(LcdStringObjectHandle_t pxLcdObject, INT8U *ucString)
//{
//    INT8U i = 0;
//    while (ucString[i] != '\0' && i < object_max_size)
//    {
//        pxLcdObject->ucUpdateBuffer[i] = ucString[i++];
//    }
//}
//
//LcdIntObjectHandle_t xLcdIntObjectCreate(INT8U x, INT8U y, INT16U value)
//{
//    LcdIntObjectHandle_t pxLcdObject = (LcdIntObjectHandle_t) pvPortMalloc(sizeof(LcdIntObject_t));
//    if (pxLcdObject == NULL)
//    {
//        return NULL;
//    }
//    
//    pxLcdObject->x = x;
//    pxLcdObject->y = y;
//    pxLcdObject->value = 0;
//    pxLcdObject->updateValue = value;
//
//    return pxLcdObject;
//}
//
//void xLcdIntObjectDelete(LcdIntObjectHandle_t pxLcdObject)
//{
//    vPortFree(pxLcdObject);
//}
//
//void vLcdIntObjectWrite(LcdIntObjectHandle_t pxLcdObject, INT16U value)
//{
//    pxLcdObject->updateValue = value;
//}




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

    cursor_position++;                      // Increment cursor position

    GPIO_PORTC_DATA_R = character & 0xF0;;  // Send high nibble
    GPIO_PORTD_DATA_R |= (1<<2);            // Select DR Register, write
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low

    GPIO_PORTC_DATA_R = character << 4;;    // Send low nibble
    GPIO_PORTD_DATA_R |= (1<<3);            // Set E High
    GPIO_PORTD_DATA_R &= ~(1<<3);           // Set E Low
    
    xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(30));
    //vTaskDelay(pdMS_TO_TICKS(25));
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
    vTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(30));
    //vTaskDelay(pdMS_TO_TICKS(25));
}


void vLcdClearDisplay(void)
/**********************************************
 * Input    :
 * Output   :
 * Function : Clear display
 **********************************************/
{
    vLcdControlWrite((INT8U)CLEAR_DISPLAY);
    cursor_position = 0;
}

void vLcdHome()
/**********************************************
 * Input    :
 * Output   :
 * Function : Moves cursor to position 0
 **********************************************/
{
    vLcdControlWrite(HOME);     
    cursor_position = 0;     
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
    
    if(target_position < cursor_position)
    {
        
        while (cursor_position - target_position)
        {
            vLcdControlWrite(MOVE_CURSOR_LEFT);
            cursor_position--;
        }
    }
    else if(target_position > cursor_position)
    {
        
        while (target_position - cursor_position)
        {
            vLcdControlWrite(MOVE_CURSOR_RIGHT);
            cursor_position++;
        }
    }
   
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
    while (charPTR[i] != '\0')
    {
        vLcdCharecterWrite(charPTR[i++]);

    }
}
void lcdSendCommand(LcdCommand_t cmd, TickType_t ticksToWait)
{
    LcdMessage_t msg;
    msg.cmd = cmd;
    xQueueSend(xLcdFunctionQueue, &msg, ticksToWait);
}

void lcdSendWriteString(INT8U *str, TickType_t ticksToWait) {
    LcdMessage_t msg;
    msg.cmd = lcdWriteString;
    msg.params.string = str;
    xQueueSend(xLcdFunctionQueue, &msg, ticksToWait);
}

void lcdSendMoveCursor(INT8U x, INT8U y, TickType_t ticksToWait) {
    LcdMessage_t msg;
    msg.cmd = lcdMoveCursor;
    msg.params.x = x;
    msg.params.y = y;
    xQueueSend(xLcdFunctionQueue, &msg, ticksToWait);
}

void lcdSendWriteChar(INT8U c, TickType_t ticksToWait) {
    LcdMessage_t msg;
    msg.cmd = lcdWriteChar;
    msg.params.charecter = c;
    xQueueSend(xLcdFunctionQueue, &msg, ticksToWait);
}

void vLcdTaskTester(void *pvParameters)
{
    INT8U x = 0;
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
        lcdSendCommand(lcdClearDisplay,portMAX_DELAY);
        lcdSendMoveCursor(x++,0,portMAX_DELAY);
        lcdSendWriteChar('a',portMAX_DELAY);
    }

}


void vLCDTask(void *pvParameters)
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
        vLcdCharecterWrite(instruction.params.string);
        if(instruction.params.string != NULL)
        {
            vPortFree(instruction.params.string);
        }
        break;
        default:
            break;
        }
       
    }

}




