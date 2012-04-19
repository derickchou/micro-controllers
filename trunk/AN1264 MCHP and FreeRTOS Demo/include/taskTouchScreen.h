/*****************************************************************************
 * Microchip RTOS and Stacks Demo
 *****************************************************************************
 * FileName:        taskTouchScreen.h
 * Dependencies:    
 * Processor:       PIC24, PIC32
 * Compiler:       	MPLAB C30 V3.00, MPLAB C32
 * Linker:          MPLAB LINK30, MPLAB LINK32
 * Company:         Microchip Technology Incorporated
 *
 * Software License Agreement
 *
 * Copyright © 2008 Microchip Technology Inc.  All rights reserved.
 * Microchip licenses to you the right to use, modify, copy and distribute
 * Software only when embedded on a Microchip microcontroller or digital
 * signal controller, which is integrated into your product or third party
 * product (pursuant to the sublicense terms in the accompanying license
 * agreement).  
 *
 * You should refer to the license agreement accompanying this Software
 * for additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY
 * OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR
 * PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR
 * OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION,
 * BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT
 * DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL,
 * INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
 * COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY
 * CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF),
 * OR OTHER SIMILAR COSTS.
 *
 * Version  Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 1.0      D.Wenn               09/22/08    Modified from QVGA library 1.52
 *											 to be RTOS compliant    
 *****************************************************************************/

#ifndef __TASKTOUCHSCREEN_H
#define __TASKTOUCHSCREEN_H

// this sw is designed to run on the GRAPHICS_PICTAIL_VERSION 2
#define FLIP_Y

// Default calibration values
#define YMINCAL         0x85
#define YMAXCAL         0x37D
#define XMINCAL         0x6C
#define XMAXCAL         0x363

// Max/Min ADC values for each direction
extern volatile WORD _calXMin;
extern volatile WORD _calXMax;
extern volatile WORD _calYMin;
extern volatile WORD _calYMax;

// Addresses for calibration and version values in FLASH
#define ADDRESS_VERSION  (unsigned long)0x00000000
#define ADDRESS_XMIN     (unsigned long)0x00000002
#define ADDRESS_XMAX     (unsigned long)0x00000004
#define ADDRESS_YMIN     (unsigned long)0x00000006
#define ADDRESS_YMAX     (unsigned long)0x00000008

// different touch screen scan rates
#define TOUCH_SLOW_SCAN		(100 / portTICK_RATE_MS)
#define TOUCH_FAST_SCAN		(10 / portTICK_RATE_MS)

// Current ADC values for X and Y channels and potentiometer R6
extern volatile SHORT adcX;
extern volatile SHORT adcY;
extern volatile SHORT adcPot;
extern volatile SHORT adcTemp;

// size of the stack for this task
#define STACK_SIZE_TOUCHSCREEN		(configMINIMAL_STACK_SIZE * 3)
// handle to the task
extern xTaskHandle hTOUCHTask;

/*********************************************************************
* Function: SHORT TouchGetX(void)
*
* PreCondition: none
*
* Input: none
*
* Output: x coordinate
*
* Side Effects: none
*
* Overview: returns x coordinate if touch screen is pressed
*           and -1 if not
*
* Note: none
*
********************************************************************/
SHORT TouchGetX(void);

/*********************************************************************
* Function: SHORT TouchGetY(void)
*
* PreCondition: none
*
* Input: none
*
* Output: y coordinate
*
* Side Effects: none
*
* Overview: returns y coordinate if touch screen is pressed
*           and -1 if not
*
* Note: none
*
********************************************************************/
SHORT TouchGetY(void);

/*********************************************************************
* Function: void TouchCalibration()
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: Runs the calibration routine. 
*
* Note: none
*
********************************************************************/
void TouchCalibration();

/*********************************************************************
* Function: void TouchStoreCalibration(void)
*
* PreCondition: EEPROMInit() must be called before
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: stores calibration parameters into EEPROM
*
* Note: none
*
********************************************************************/
void TouchStoreCalibration(void);

/*********************************************************************
* Function: void TouchLoadCalibration(void)
*
* PreCondition: EEPROMInit() must be called before
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: loads calibration parameters from EEPROM
*
* Note: none
*
********************************************************************/
void TouchLoadCalibration(void);

/*********************************************************************
* Macros: ADCGetX()
*
* PreCondition: none
*
* Input: none
*
* Output: ADC result
*
* Side Effects: none
*
* Overview: returns ADC value for x direction if touch screen is pressed
*           and -1 if not
*
* Note: none
*
********************************************************************/
#define ADCGetX() adcX

/*********************************************************************
* Macros: ADCGetY()
*
* PreCondition: none
*
* Input: none
*
* Output: ADC result
*
* Side Effects: none
*
* Overview: returns ADC value for y direction if touch screen is pressed
*           and -1 if not
*
* Note: none
*
********************************************************************/
#define ADCGetY() adcY

/*********************************************************************
* Macros: ADCGetPot()
*
* PreCondition: none
*
* Input: none
*
* Output: ADC result
*
* Side Effects: none
*
* Overview: returns ADC value for potentiometer
*
* Note: none
*
********************************************************************/
#define ADCGetPot() adcPot

/*********************************************************************
 * Function:        void taskTouchScreen(void* pvParameter)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          Does not return
 *
 * Side Effects:    None
 *
 * Overview:        Touchscreen handling routine. Touchscreen events
 *					are detected and passed as messages to the graphics
 *					display task. This isolates the human interface
 *					events from the graphics drawing
 *
 * Note:            
 ********************************************************************/
void taskTouchScreen(void* pvParameter);

#endif // __TASKTOUCHSCREEN_H
