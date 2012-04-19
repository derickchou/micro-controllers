/*****************************************************************************
 * Microchip RTOS and Stacks Demo
 *****************************************************************************
 * FileName:        taskMIWI.c
 * Dependencies:    taskMIWI.h, FreeRTOS
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
 * 1.0      D.Wenn               09/12/08    Initial Version based upon
 *                                           MiWi P2P SimpleExampleNode.c
 *****************************************************************************/

#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "WirelessProtocols\P2P\P2P.h"
#include "Transceivers\Transceivers.h"
#include "Common\SymbolTime.h"
#include "WirelessProtocols\MCHP_API.h"
#include "HardwareProfile.h"
#include "taskUART.h"
#include "LEDUtils.h"
#include "homeMeter.h"
#include "taskMIWI.h"

///////////////////////////////////////////////////////////////////
// Externally referenced function
BOOL SendPacket(INPUT BOOL Broadcast, INPUT WORD_VAL DestinationPANID, 
                INPUT BYTE *DestinationAddress, INPUT BOOL isCommand, 
                INPUT BOOL SecurityEnabled);

///////////////////////////////////////////////////////////////////
// Variable myChannel defines the default channel for the P2P connection
// if/when energy scan is turned on this will change to the least
// noisy channel
BYTE myChannel = 25;

/*******************************************************************/
// AdditionalNodeID variable array defines the additional 
// information to identify a device on a PAN. This array
// will be transmitted when initiate the connection between 
// the two devices. This  variable array will be stored in 
// the Connection Entry structure of the partner device. The 
// size of this array is ADDITIONAL_NODE_ID_SIZE, defined in 
// ConfigApp.h.
/******************************************************************/
#if ADDITIONAL_NODE_ID_SIZE > 0
    BYTE AdditionalNodeID[ADDITIONAL_NODE_ID_SIZE] = {'M', 'C', 'H', 'P', 'I', 'H', 'U'};
#endif

///////////////////////////////////////////////////////////////////
// Variables 
xQueueHandle hMIWITxQueue; // queue to send outgoing MIWI messages
xQueueHandle hMIWIRxQueue; // queue to receive incoming MIWI messages

// handle for the task
xTaskHandle hMIWITask;
// semaphore used to indicate received message in the MiWi stack
xSemaphoreHandle hMiWiSemaphore;

/*********************************************************************
 * Function:        xQueueHandle xStartMIWITask(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          Handle to the MIWI transmit queue
 *
 * Side Effects:    None
 *
 * Overview:        Setup the MIWI control task, and create an input
 *					and output queue for MIWI data
 *
 * Note:            
 ********************************************************************/
xQueueHandle xStartMIWITask(void)
{
	// create the MIWI transmit queue
	hMIWITxQueue = xQueueCreate(MIWI_QUEUE_SIZE, MIWI_ENTRY_SIZE);
	// create the queue to place received MIWI data
	hMIWIRxQueue = xQueueCreate(MIWI_QUEUE_SIZE, MIWI_ENTRY_SIZE);

	// create the MIWI task
	xTaskCreate(taskMIWI, (signed char*) "MIWI", STACK_SIZE_MIWI, 
		NULL, tskIDLE_PRIORITY + 1, &hMIWITask );
	
	return hMIWITxQueue;		
}

/*********************************************************************
 * Function:        void taskMIWI(void* pvParameter)
 *
 * PreCondition:    None
 *
 * Input:           void* pvParameter, any parameter from the creator
 *                  
 * Output:          Does not return
 *
 * Side Effects:    None
 *
 * Overview:        MIWI handling task
 *
 * Note:            
 ********************************************************************/
void taskMIWI(void* pvParameter)
{
	BYTE i;
	static char sBuff[20];
    METER_MSG msg;
    DWORD	dwUnits;
	
	// create the miwi semaphore
	vSemaphoreCreateBinary(hMiWiSemaphore);
	xSemaphoreTake(hMiWiSemaphore, portMAX_DELAY);
	
	// allow interrupts from the RF transceiver after the MiWiSemaphore
	// has been created (it is posted from the RF ISR so must exist first)
	RFIE = 1;
	
	// initialize the Microchip MiWi P2P stack
	MiApp_ProtocolInit();
	UARTprintf("MIWI: Task Started.\r\n");
	
	// select the specified channel
	MiApp_SetChannel(myChannel);
	// allow another P2P device to connect to us (the meter)
	MiApp_ConnectionMode(ENABLE_ALL_CONN);
	UARTprintf("MIWI: Waiting for connection from peer.\r\n");
	
	// create the new connection
    i = MiApp_EstablishConnection(0xFF, CONN_MODE_DIRECT);
	// connection complete
	UARTprintf("MIWI: Bound to peer.\r\n");
	
	while (1) {
    	// We use a sempahore to signal that this task should
    	// perform some processing. In the MiWi ISR handler
    	// if any packet is received hMiWiSemaphore is set indicating
    	// that processing needs to occur. This will unblock this task
    	// which will then complete the receive and process the packet
    	// To keep the network alive a timeout is used so that 
    	// stack processing can occur regardless but at a much slower
    	// rate. This technique allows taskMiWi to run at low
    	// priority without consuming too much processor time
	    xSemaphoreTake(hMiWiSemaphore, 1000 / portTICK_RATE_MS);
	        
		// check for new received data. This function call also keeps
		// the stack alive by allowing it to perform stack processing
		if (MiApp_MessageAvailable()) {
			LEDSetState(LED6, 1);
			
			// copy the received value into the buffer for queueing
			for(i = 0; (i < rxMessage.PayloadSize) && (i < MIWI_ENTRY_SIZE); i++) {
				sBuff[i] = rxMessage.Payload[i];
			}
			sBuff[i++] = '\0';
			
			// distinguish between the two types of update
			// electricity or gas
			if (sBuff[0] == 'E') {
				dwUnits = atoi(&sBuff[1]);
				msg.data.dVal = dwUnits;
				msg.cmd = MSG_METER_UPDATE_ELECTRIC;			
				xQueueSend(hMETERQueue, &msg, 20 / portTICK_RATE_MS);
			} else if (sBuff[0] == 'G') {
				dwUnits = atoi(&sBuff[1]);
				msg.data.dVal = dwUnits;
				msg.cmd = MSG_METER_UPDATE_GAS;
				xQueueSend(hMETERQueue, &msg, 20 / portTICK_RATE_MS);
			} else {
				// UARTprintf("MIWI: Unknown packet type\r\n");
			}
			
			MiApp_DiscardMessage();	
		} else {
			// reset the transmit buffer
			MiApp_FlushTx();
			// send out any pending data
		}
					
		LEDSetState(LED6, 0);
	}		
}



