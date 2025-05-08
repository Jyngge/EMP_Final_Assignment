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

/***************** Defines ********************/
#define COLUMN0     0x10    //PA4
#define COLUMN1     0x08    //PA3
#define COLUMN2     0x04    //PA2
#define ROW0        0x08    //PE3
#define ROW1        0x02    //PE1
#define ROW2        0x04    //PE2
#define ROW3        0x01    //PE0
#define ROW_MASK    0x0F    //PE3, PE1, PE2, PE0
#define COLUMN_MASK 0x1C    //PA4, PA3, PA2

/***************** Constants ******************/
const static INT8U keypad[4][3] = 
{   //not the physical layout of the keypad
    {'*', '0', '#'},  // Row 0
    {'1', '2', '3'},  // Row 1
    {'4', '5', '6'},  // Row 2
    {'7', '8', '9'}   // Row 3
};

/***************** Variables ******************/
INT16U keyPressed;                   // Key pressed
INT16U row, column;                  // Row and column indices
QueueHandle_t xKeypadQueue;         // Queue to send key presses
TaskHandle_t xKeypadTaskHandle;     // Handle for the keypad task
INT16U interruptFlag;

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
    GPIO_PORTE_IEV_R |= ROW_MASK;       // Interrupt on falling edge
    GPIO_PORTE_ICR_R |= ROW_MASK;       // Clear any prior interrupt
    
    GPIO_PORTA_DATA_R |= COLUMN_MASK;   // Set all columns high to trip interupt
    

    xKeypadQueue = xQueueCreate(5, sizeof(INT8U));
    if (xKeypadQueue == NULL) {
        lcd_string_write("keypadQueue creation failed!");
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

    while(1) {
        
        xStatus = xQueueReceive(xKeypadQueue, &keyPressedTest, portMAX_DELAY);
        if(xStatus == pdPASS)
        {
            lcd_clear_display();
            lcd_char_write(keyPressedTest);
        } else {
            lcd_string_write("Error receiving key press!");
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
    interruptFlag = GPIO_PORTE_MIS_R & ROW_MASK;
    GPIO_PORTE_IM_R &= ~ROW_MASK;
    GPIO_PORTE_ICR_R = ROW_MASK;                          // Clear the interrupt flags for rows
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;   
    vTaskNotifyGiveFromISR(xKeypadTaskHandle, &xHigherPriorityTaskWoken);
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
    

    while (1)
    {
        dummyTake = ulTaskNotifyTake(pdFalse, portMAX_DELAY);
        GPIO_PORTE_IM_R &= ~ROW_MASK;

        i = 4;

        while(interruptFlag >1)
        {
            interruptFlag >>=1;
            i--;
        }

        row = i;
        i = 0;
        
        while(GPIO_PORTE_DATA_R & COLUMN_MASK)
        {
            GPIO_PORTA_DATA_R &= ~(1<<(2 + i));
            i++;
        }

        column = i;



        GPIO_PORTA_DATA_R |= COLUMN_MASK;                     // Set column 3 high
        keyPressed = keypad[row][column];                     // Get the key pressed from the keypad array
        xQueueSend(xKeypadQueue, &keyPressed, portMAX_DELAY); // Send key press to queue
        while(GPIO_PORTE_DATA_R & ROW_MASK)
        {
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
        GPIO_PORTE_IM_R |= ROW_MASK;                          // Re-enable interrupts on rows
    }
}

