/*****************************************************************************
 * Microchip RTOS and Stacks Demo
 *****************************************************************************
 * FileName:        taskGraphics.h
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
 * 1.0      D.Wenn               09/24/08    Initial version   
 *****************************************************************************/

#ifndef __TASK_GRAPHICS_H
#define __TASK_GRAPHICS_H

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"

///////////////////////////////////////////////////////////////////
// objects to be passed to the graphics display routines are
// encapsulated in this data type. All incoming messages are
// expressed using this structure and then passed onto the
// incoming queue
typedef struct{
	WORD	cmd;
	union {
		BYTE		bVal[8];
		WORD		wVal[4];
		DWORD		dVal[2];
		GOL_MSG 	golMsg;
	} data;
} GRAPHICS_MSG;

// the queue used to send messages to the graphics task
extern xQueueHandle hQVGAQueue;

// number of entries in this queue
#define QVGA_QUEUE_SIZE	6

///////////////////////////////////////////////////////////////////
// Defines for the possible messages to be sent
#define MSG_DEFAULT					0	// do nothing
#define MSG_UPDATE_DISPLAY			1	// redraw the display
#define MSG_UPDATE_TEMPERATURE		2	// update the actual temperature
#define MSG_TOUCH_EVENT				3	// touchscreen activity
#define MSG_UPDATE_ELECTRIC_TOTAL	4	// electricity changed
#define MSG_UPDATE_ELECTRIC_UNITS	5	// units used
#define MSG_UPDATE_ELECTRIC_COST	6	// unit cost
#define MSG_UPDATE_GAS_TOTAL		7	// gas changed
#define MSG_UPDATE_GAS_UNITS		8	// units used
#define MSG_UPDATE_GAS_COST			9	// gas unit cost

// these defined is used to scale the task usage bar graph display
#if defined(__PIC24F__)
#define MAX_BAR_LENGTH	12
#else
#define MAX_BAR_LENGTH 	30
#endif

// Graphics task itself
extern void taskGraphics(void* pvParameter);

// size of the stack for this task
#define STACK_SIZE_GRAPHICS		(configMINIMAL_STACK_SIZE * 3)
// handle to the task
extern xTaskHandle hGRAPHICSTask;

#endif // __TASK_GRAPHICS_H

