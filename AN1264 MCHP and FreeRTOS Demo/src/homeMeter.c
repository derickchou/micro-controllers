/*****************************************************************************
 * Microchip RTOS and Stacks Demo
 *****************************************************************************
 * FileName:        homeMeter.c
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

#include "Graphics/Graphics.h"
#include "taskGraphics.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "taskUART.h"
#include "homeMeter.h"

// queue for incoming data updates
xQueueHandle hMETERQueue;
// handle for the task
xTaskHandle hMETERTask;

/*********************************************************************
 * Function:        void taskMeter(void* pvParameter)
 *
 * PreCondition:    None
 *
 * Input:           void* pvParameter, any parameter from the creator
 *                  
 * Output:          Does not return
 *
 * Side Effects:    None
 *
 * Overview:        Meter handling task, this task awaits for
 *					asynchronous events related to the meter
 *					updates the meter data structure accordingly
 *					and then sends asynchronous update messages
 *					to other tasks. Synchronous tasks (such as TCPIP)
 *					can retrieve the data when they need to. Asynch
 *					tasks, such as QVGA must be told to update
 *
 * Note:            
 ********************************************************************/
void taskMeter(void* pvParameter)
{
	static METER_MSG msg;
	static GRAPHICS_MSG gMsg;
	
	// initialise the meter object
	xSemaphoreTake(METERSemaphore, portMAX_DELAY);
	gMeter.setpoint = 190;		// 19.0 celsius
	gMeter.electric_cost = 12;	// 12 cents per unit
	gMeter.electric_units = 0;
	gMeter.electric_total = 0;
	gMeter.electric_on = 1;
	gMeter.gas_cost = 37;		// 37 cents per unit
	gMeter.gas_units = 0;
	gMeter.gas_total = 0;
	gMeter.gas_on = 1;
	gMeter.temperature = 0;	
	xSemaphoreGive(METERSemaphore);
	
	while (1) {
		// wait for an incoming message
		xQueueReceive(hMETERQueue, &msg, portMAX_DELAY);
		// take control of the meter information structure
		xSemaphoreTake(METERSemaphore, portMAX_DELAY);
		
		switch(msg.cmd) {
			case MSG_METER_UPDATE_TEMPERATURE:
				// update the meter
				gMeter.temperature = msg.data.wVal[0];
				// send an update to the QVGA display
				gMsg.cmd = MSG_UPDATE_TEMPERATURE;
				gMsg.data.wVal[0] = gMeter.temperature;
				xQueueSend(hQVGAQueue, &gMsg, 0);
				break;
		
			case MSG_METER_UPDATE_ELECTRIC:
				// update the meter only if it is enabled
				if (gMeter.electric_on == 1) {
					gMeter.electric_units++;
					gMeter.electric_total += gMeter.electric_cost; 
				
					// send update to the QVGA display (Note preformatting)
					gMsg.cmd = MSG_UPDATE_ELECTRIC_UNITS;
					gMsg.data.dVal[0] = gMeter.electric_units;
					xQueueSend(hQVGAQueue, &gMsg, 0);
					gMsg.cmd = MSG_UPDATE_ELECTRIC_TOTAL;
					gMsg.data.wVal[0] = gMeter.electric_total % 100;
					gMsg.data.wVal[1] = gMeter.electric_total / 100;
					xQueueSend(hQVGAQueue, &gMsg, 0);				
				}
				break;
			
			case MSG_METER_UPDATE_GAS:
				// update the meter only if it is enabled
				if (gMeter.gas_on == 1) {
					gMeter.gas_units++;
					gMeter.gas_total += gMeter.gas_cost;
					
					// send update to the display
					gMsg.cmd = MSG_UPDATE_GAS_UNITS;
					gMsg.data.dVal[0] = gMeter.gas_units;
					xQueueSend(hQVGAQueue, &gMsg, 0);
					gMsg.cmd = MSG_UPDATE_GAS_TOTAL;
					gMsg.data.wVal[0] = gMeter.gas_total % 100;
					gMsg.data.wVal[1] = gMeter.gas_total / 100;
					xQueueSend(hQVGAQueue, &gMsg, 0);			
				}
				break;
				
			case MSG_METER_UPDATE_ELECTRIC_COST:
				// update the unit cost
				gMeter.electric_cost = msg.data.wVal[0];
				break;
				
			case MSG_METER_UPDATE_GAS_COST:
				// update the unit cost
				gMeter.gas_cost = msg.data.wVal[0];
				break;
				
			default:
				UARTprintf("METER: Unhandled message!\r\n");
				break;	
		}	
		// release the data
		xSemaphoreGive(METERSemaphore);	
	}	
}

