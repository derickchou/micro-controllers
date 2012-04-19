/*****************************************************************************
 * Microchip RTOS and Stacks Demo
 *****************************************************************************
 * FileName:        taskUART.c
 * Dependencies:    UART.h, FreeRTOS
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
 * 1.0      D.Wenn               09/01/08    Initial Version    
 *****************************************************************************/

// include device specific peripheral libraries
#if defined(__PIC24F__)
	#define USE_AND_OR
	#include <uart.h>
#else
	#include <plib.h>
	#include <GenericTypeDefs.h>
#endif

#include <string.h>

#include "HardwareProfile.h"
#include "taskUART.h"

///////////////////////////////////////////////////////////////////
// forward declarations
void taskUART(void* pvParameter);

///////////////////////////////////////////////////////////////////
// Variables 
xQueueHandle hUARTTxQueue;
xQueueHandle hUARTRxQueue;
xTaskHandle hUARTTask;

/*********************************************************************
 * Function:        xQueueHandle xStartUARTTask(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          Handle to the UART TX queue
 *
 * Side Effects:    None
 *
 * Overview:        Configure the UART and channel
 *
 * Note:            
 ********************************************************************/
xQueueHandle xStartUARTTask(void)
{
	// create the UART queue for transmitting data
	hUARTTxQueue = xQueueCreate(UART_QUEUE_SIZE, sizeof(UARTMsg));
	// and the queue for receiving characters
	hUARTRxQueue = xQueueCreate(UART_QUEUE_SIZE, sizeof(char));
	
	// set up the UART
	UARTTX_TRIS = 0;
	UARTRX_TRIS = 1;
	OpenUART(UART_CONFIG1, UART_CONFIG2, CLOSEST_BRG);
	ConfigIntUART(UART_INT_CONFIG);	
	
	// create the UART transmit task
	xTaskCreate(taskUART, (signed char*) "UART", STACK_SIZE_UART, 
		NULL, tskIDLE_PRIORITY + 1, &hUARTTask );
		
	return hUARTTxQueue;
}

/*********************************************************************
 * Function:        void taskUART(void* pvParameter)
 *
 * PreCondition:    None
 *
 * Input:           void* pvParameter, any parameter from the creator
 *                  
 * Output:          Does not return
 *
 * Side Effects:    None
 *
 * Overview:        UART handling task
 *
 * Note:            
 ********************************************************************/
void taskUART(void* pvParameter)
{
	UARTMsg msg;
	char* pChar;
		
	// in this task we wait for a string to be received and then
	// print it out.
	while (1) {
		// wait until a message is received
		xQueueReceive(hUARTTxQueue, &msg, portMAX_DELAY);
		
		// print out the string
		pChar = msg.buff;
		while (*pChar != '\0') {
			// while there is space in the tx buffer
			while ((!USTAbits.UTXBF) && (*pChar != '\0')) {
				UTXREG = *pChar++;	
			}			
		}
	}	
}

/*********************************************************************
 * Function:        void UART ReceiveInterrupt(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        UART handling task, for PIC24F we install a vector
 *					at the correct location. For PIC32 the interrupt
 *					handler is called from an assembly wrapper
 *
 * Note:            
 ********************************************************************/
#if defined(__C30__)
void __attribute__((__interrupt__, auto_psv)) _U2RXInterrupt(void)
{
	char cChar;
	portBASE_TYPE xTaskWoken = pdFALSE;
	
	// get the character and post it on the queue possibly starting a context
	// switch if a higher priority task was woken
	IFS1bits.U2RXIF = 0;
	while (USTAbits.URXDA) {
		cChar = URXREG;
		// send the data to the queue, if the queue is full then we
		// lose the data rather than blocking the ISR
		xQueueSendFromISR(hUARTRxQueue, &cChar, &xTaskWoken);
	}	
	
	if (xTaskWoken != pdFALSE)
		taskYIELD();	
}
#else // PIC32
// the actual PIC32 handling routine 
void __attribute__( (interrupt(ipl2), vector(_UART2_VECTOR))) vU2InterruptHandler(void)
{
	char cChar;
	portBASE_TYPE xTaskWoken = pdFALSE;
	
	// check for RX interrupts
	if (mU2RXGetIntFlag()) {
		while (USTAbits.URXDA) {
			/* Retrieve the received character and place it in the queue of
			received characters. */
			cChar = U2RXREG;
			xQueueSendFromISR(hUARTRxQueue, &cChar, &xTaskWoken);
		}
		mU2RXClearIntFlag();
	}

	// if there are TX interrupts then just clear them as we don't handle them
	if (mU2TXGetIntFlag())
		mU2TXClearIntFlag();

	// If sending or receiving necessitates a context switch, then switch now.
	if (xTaskWoken != pdFALSE)
		taskYIELD();	
}
#endif

/*********************************************************************
 * Function:        void UARTprintf(char* msg)
 *
 * PreCondition:    None
 *
 * Input:           Character array to be printed
 *                  
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Print the string to the UART
 *
 * Note:            
 ********************************************************************/
void UARTprintf(char* msg)
{
	UARTMsg sMsg;
	
	// copy the message and ensure it is NULL terminated
	strncpy(sMsg.buff, msg, UART_ENTRY_SIZE);
	sMsg.buff[UART_ENTRY_SIZE - 1] = '\0';
	
	// send the message to the UART task
	// we use a zero wait time so as not to block any high priority tasks
	// if the low priority UART task is currently printing and the queue
	// is full then we just fail to print the message.
	// This is okay since it is only used for diagnostic messages
	xQueueSendToBack(hUARTTxQueue, &sMsg, 0);	
}

/*********************************************************************
 * Function:        void UARTPrintChar(BYTE c)
 *
 * PreCondition:    None
 *
 * Input:           Character to be printed
 *                  
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Print the character to the UART in hex format
 *
 * Note:            
 ********************************************************************/
const unsigned char HexCharArray[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void UARTPrintChar(BYTE c)
{
	UARTMsg sMsg;
	BYTE toPrint;
	
	// convert the character and print it
	toPrint = (c >> 4) & 0x0F;
	sMsg.buff[0] = HexCharArray[toPrint];
	toPrint = c & 0x0F;
	sMsg.buff[1] = HexCharArray[toPrint];
	sMsg.buff[2] = '\0';
	
	// send the message to the UART task
	xQueueSendToBack(hUARTTxQueue, &sMsg, portMAX_DELAY);	
}



