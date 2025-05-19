/*
 * UI.h
 *
 *  Created on: 9. maj 2025
 *      Author: bub
 */

#ifndef UI_H_
#define UI_H_

void vUIInit(void);
void vUITask(void);

typedef enum{
    idle,        
    moving,      
    inputCode,   
    selectFloor, 
    broken,      
    pot,         
    Digi360      
}ElevatorState_t;



#endif /* UI_H_ */
