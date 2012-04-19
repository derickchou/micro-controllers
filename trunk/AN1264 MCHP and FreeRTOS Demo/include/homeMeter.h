/*****************************************************************************
 * Microchip RTOS and Stacks Demo
 *****************************************************************************
 * FileName:        homeMeter.h
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
 * 1.0      D.Wenn               09/29/08    Initial version   
 *****************************************************************************/

#ifndef __HOME_METER_H
#define __HOME_METER_H

// definition of the home meter object used to store information
// about the gas and electricity in this application
typedef struct {
	DWORD	electric_units;		// electricity units used
	DWORD	electric_total;		// electricity total cost
	DWORD	electric_cost;		// electricty cost per unit ($0.01)
	DWORD	gas_units;			// gas units used
	DWORD	gas_total;			// gas total cost
	DWORD	gas_cost;			// gas cost per unit		($0.01)
	WORD	electric_on;		// electricity supply enabled
	WORD	gas_on;				// gas supply enabled
	WORD	setpoint;			// thermometer setpoint
	WORD	temperature;		// current temperature (0.1C)
} structMeter;

// the one global meter object
extern structMeter gMeter;

// structure used to pass updates to the meter task
// other task send messages of this type on a queue
typedef struct {
	WORD	cmd;
	union {
		BYTE		bVal[4];
		WORD		wVal[2];
		DWORD		dVal;
	} data;
} METER_MSG;

#define METER_QUEUE_SIZE	4

// types of meter messages
#define MSG_METER_UPDATE_TEMPERATURE		1	// new temperature reading
#define MSG_METER_UPDATE_SETPOINT			2	// new setpoint value
#define MSG_METER_UPDATE_ELECTRIC			3	// electric used update
#define MSG_METER_UPDATE_GAS				4	// gas used update
#define MSG_METER_UPDATE_ELECTRIC_STATE		5	// electric state change
#define MSG_METER_UPDATE_GAS_STATE			6	// gas state change
#define MSG_METER_UPDATE_ELECTRIC_COST		7	// cost per unit
#define MSG_METER_UPDATE_GAS_COST			8	// cost per unit

// queue to send incoming meter update messages
extern xQueueHandle hMETERQueue;

// semaphore to guard access to the meter object
extern xSemaphoreHandle METERSemaphore;

// the meter task
extern void taskMeter(void* pvParameter);

// size of the stack for this task
#define STACK_SIZE_METER		(configMINIMAL_STACK_SIZE * 1)
// handle for the meter task
extern xTaskHandle hMETERTask;

#endif // __HOME_METER_H
