/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: keypad.c
*
* PROJECT....: Final assignment
*
* DESCRIPTION: 
*
* Change Log:
******************************************************************************
* Date    Id          Change
* 060525  Majur22     Module created.
*
*****************************************************************************/

/***************** Include files **************/

#include "keypad.h"
#include <limits.h>


/***************** Defines ********************/
#define COLUMN0     0x10    //PA4 //f //x1
#define COLUMN1     0x08    //PA3 //e //x2
#define COLUMN2     0x04    //PA2 //d //x3
#define ROW1        0x08    //PE3 //k //y1
#define ROW2        0x04    //PE2 //j //y2
#define ROW3        0x02    //PE1 //h //y3
#define ROW0        0x01    //PE0 //g //y4
#define ROW_MASK    0x0F    //PE3, PE1, PE2, PE0
#define COLUMN_MASK 0x1C    //PA4, PA3, PA2



/***************** Constants ******************/
const static INT8U keypad[4][3] = 
{   //not the physical layout of the keypad
        // Row 0
    {'1', '2', '3'},    // Row 1
    {'4', '5', '6'},    // Row 2
    {'7', '8', '9'},     // Row 3
    {'*', '0', '#'}
};

/***************** Variables ******************/
INT16U keyPressed;                   // Key pressed
INT16U row, column;                  // Row and column indices
QueueHandle_t xKeypadQueue;         // Queue to send key presses
extern QueueHandle_t xLcdFunctionQueue;
TaskHandle_t xKeypadTaskHandle;     // Handle for the keypad task


INT32U dummyGive;
INT32U dummyTake;
/***************** Functions ******************/

void vKeypadInit(void)
/**********************************************
 * Input    :    
 * Output   : 
 * Function : Initializes GPIO pins for keypad and sets up the queue
 **********************************************/
{
    
    GPIO_PORTE_DEN_R |= ROW_MASK;       // Enable digital function on rows
    GPIO_PORTA_DEN_R |= COLUMN_MASK;    // Enable digital function on columns
    
    GPIO_PORTA_DIR_R |= COLUMN_MASK;    // Set columns as output
    GPIO_PORTE_DIR_R &= ~ROW_MASK;      // Set rows as input
    
    GPIO_PORTE_IM_R |= ROW_MASK;        // Enable interrupt on rows
    GPIO_PORTE_IS_R &= ~ROW_MASK;       // Set interrupt to edge-sensitive
    GPIO_PORTE_IBE_R &= ~ROW_MASK;      // Interrupt on a single edge
    GPIO_PORTE_IEV_R |= ROW_MASK;       // Trigger on rising edge
    GPIO_PORTE_ICR_R |= ROW_MASK;       // Clear any prior interrupt
    
    GPIO_PORTA_DATA_R |= COLUMN_MASK;   // Set all columns high to trip interupt
    

    xKeypadQueue = xQueueCreate(5, sizeof(INT8U));
    if (xKeypadQueue == NULL) {
        vLcdStringWrite("Error Creating Queue");
        while (1); 
    }
}




void vKeypadTestTask(void *pvParameters)
/**********************************************
 * Input    :    
 * Output   : 
 * Function : Just a test task to display the key pressed on the LCD.
 *            please delete this task in the final version.
 **********************************************/
{
    INT8U keyPressedTest;
    BaseType_t xStatus;
    LcdMessage_t instruction;
    while(1) {
        
        xStatus = xQueueReceive(xKeypadQueue, &keyPressedTest, portMAX_DELAY);
        if(xStatus == pdPASS)
        {
            instruction.cmd = lcdWriteChar;
            instruction.params.charecter = keyPressedTest;
            xQueueSend(xLcdFunctionQueue,&instruction,portMAX_DELAY);
        } 
        else 
        {

        }


    }
}

void vKeypadInterruptHandler(void)
/**********************************************
 * Input    :    
 * Output   : 
 * Function : Performs the context switch to the keypad task
 **********************************************/
{
    GPIO_PORTE_IM_R &= ~ROW_MASK;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;   
    xTaskNotifyFromISR(xKeypadTaskHandle,GPIO_PORTE_RIS_R & (ROW_MASK), eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
    GPIO_PORTE_ICR_R |= ROW_MASK;
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}



void vKeypadScanTask(void *pvParameters)
/**********************************************
 * Input    :    
 * Output   : 
 * Function : Waits for a notification from the interrupt handler
 *            and scans the keypad when triggered.
 **********************************************/
{
    INT16U i;
    INT32U RecievdValue;
    

    while(1)
    {
        
        xTaskNotifyWait(0x00, ULONG_MAX, &RecievdValue, portMAX_DELAY); // Wait for notification from interrupt handler
        
        i = 0;

        while(RecievdValue >>=1)
        {
            i++;
        }

        row = 3-i;

        i = 3;
        
        while(GPIO_PORTE_DATA_R & ROW_MASK)
        {
            GPIO_PORTA_DATA_R &= ~(1<<(1 + i));
            i--;
        }

        column = 2-i;

        GPIO_PORTA_DATA_R |= COLUMN_MASK;                     // Set column 3 high
        keyPressed = keypad[row][column];                     // Get the key pressed from the keypad array
        xQueueSend(xKeypadQueue, &keyPressed, portMAX_DELAY); // Send key press to queue

        while(GPIO_PORTE_DATA_R & ROW_MASK)
        {
            vTaskDelay(pdMS_TO_TICKS(25));
        }

        //vTaskDelay(pdMS_TO_TICKS(300));
        GPIO_PORTE_ICR_R |= ROW_MASK;
        GPIO_PORTE_IM_R |= ROW_MASK;                          // Re-enable interrupts on rows
                                 
    }
}
