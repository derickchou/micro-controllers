/*****************************************************************************
 * Microchip RTOS and Stacks Demo
 *****************************************************************************
 * FileName:        LEDUtils.c
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
 * 1.0      D.Wenn               09/11/08    Initial Version    
 *****************************************************************************/

#include "HardwareProfile.h"
#include "FreeRTOS.h"

/*********************************************************************
 * Function:        void LEDInit(void)
 *
 * PreCondition:    RTOS is not running
 *
 * Input:           None
 *                  
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Initialize the LED functions
 *
 * Note:            
 ********************************************************************/
void LEDInit(void)
{
	// set the port bits
	LED0_IO = 0;
	LED1_IO = 0;
	LED4_IO = 0;
	LED5_IO = 0;
	LED6_IO = 0;
	
	// set the tris registers
	LED0_TRIS = 0;	
	LED1_TRIS = 0;
	LED4_TRIS = 0;
	LED5_TRIS = 0;
	LED6_TRIS = 0;		
}

/*********************************************************************
 * Function:        void LEDSetState(unsigned int bitfield, unsigned int state)
 *
 * PreCondition:    None
 *
 * Input:           bitfield leds to set, state 0 or 1
 *                  
 * Output:          None
 *
 * Side Effects:    Mutex
 *
 * Overview:        Set an LED into a particular mode, no checking for
 *					illegal values is performed
 *
 * Note:            
 ********************************************************************/
void LEDSetState(unsigned int bitfield, unsigned int state)
{
#if defined(__C30__) // PIC24
	portENTER_CRITICAL();
	if (state)
		LATA |= bitfield;
	else
		LATA &= ~bitfield;
	portEXIT_CRITICAL();
#else // PIC32
	if (state)
		LATASET = bitfield;
	else
		LATACLR = bitfield;
#endif	
}

/*********************************************************************
 * Function:        void LEDToggle(unsigned int bitfield)
 *
 * PreCondition:    None
 *
 * Input:           bitfield leds to toggle
 *                  
 * Output:          None
 *
 * Side Effects:    Mutex
 *
 * Overview:        toggle a number of leds
 *
 * Note:            
 ********************************************************************/
void LEDToggle(unsigned int bitfield)
{
#if defined(__C30__) // PIC24
	portENTER_CRITICAL();
	LATA ^= bitfield;
	portEXIT_CRITICAL();
#else // PIC32 mutex not required
	LATAINV = bitfield;
#endif
}

