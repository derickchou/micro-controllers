/*****************************************************************************
 * Microchip RTOS and Stacks Demo
 *****************************************************************************
 * FileName:        MainDemo.c
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
 * 1.0      D.Wenn               09/01/08    Initial Version  
 * 1.1		D.Wenn				 07/15/09	 Added GFX3 and ENC624J600 support
 * 1.2      D.Wenn               09/04/09    Upgrade to latest stacks  
 *****************************************************************************/
 
// Standard includes
#include "HardwareProfile.h"
#include "Graphics\Graphics.h"
#include <stdio.h>

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"

// local includes
#include "LEDUtils.h"
#include "taskUART.h"
#include "taskMIWI.h"
#include "taskTouchScreen.h"
#include "taskGraphics.h"
#include "taskTCPIP.h"
#include "homeMeter.h"

// set the configuration fuses for the processor
#if defined(__PIC24F__)
	_CONFIG2(FNOSC_PRIPLL & POSCMOD_XT)		// Primary XT OSC with 4x PLL
	_CONFIG1(ICS_PGx2 & JTAGEN_OFF & FWDTEN_OFF) // JTAG off, watchdog timer off
#elif defined(__PIC32MX__)
	#pragma config FPLLODIV = DIV_1, FPLLMUL = MUL_20, FPLLIDIV = DIV_2
	#pragma config FWDTEN = OFF, FPBDIV = DIV_2, POSCMOD = XT, FNOSC = PRIPLL, CP = OFF
#endif

///////////////////////////////////////////////////////////////////
// Semaphores used to guard access to the various hardware resources
// they are controlled because multiple tasks may access them
xSemaphoreHandle SPI2Semaphore;
xSemaphoreHandle FLASHSemaphore;
xSemaphoreHandle QVGASemaphore;
xSemaphoreHandle METERSemaphore;

// global meter object
structMeter gMeter;

///////////////////////////////////////////////////////////////////
// Forward references in this module
static void InitializeBoard(void);

///////////////////////////////////////////////////////////////////
// local variables and constants
const UARTMsg msgAppStart = {"\r\nApplication Started. Build: " __TIME__ "\r\n"};

/*********************************************************************
 * Function:        int main(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          Does not return
 *
 * Side Effects:    None
 *
 * Overview:        Application main entry point
 *
 * Note:            
 ********************************************************************/
int main(void)
{
	// create a semaphore to control access to SPI2 since it used
	// by both the TCPIP and FLASH devices
	SPI2Semaphore = xSemaphoreCreateMutex();
	// The FLASH semaphore guards access to the FLASH itself. Since
	// we use the FLASH to store web pages AND touchscreen configuration
	// and the FLASH code uses caching we must prevent conflicts. In
	// this case we will allow the touchscreen to access the FLASH
	// first and then allow TCPIP unrestricted access afterwards
	FLASHSemaphore = xSemaphoreCreateMutex();
	// the QVGA Semaphore is normally controlled by the graphics
	// task however if calibration is required then it must take
	// control until the user has completed the calibration steps
	// we also use this on GFX2 boards to control access to the 
	// parallel FLASH device which is shared by graphics and tcpip
	QVGASemaphore = xSemaphoreCreateMutex();
	
	// semaphore to control access to the meter object
	vSemaphoreCreateBinary(METERSemaphore);
	
	// messages to update the display are sent via the QVGAQueue
	hQVGAQueue = xQueueCreate(QVGA_QUEUE_SIZE, sizeof(GRAPHICS_MSG));
	
	// configure the hardware resources on the board
	InitializeBoard();	
	
	// create the UART task
	xStartUARTTask();
	// tell the world we have started
	xQueueSendToBack(hUARTTxQueue, &msgAppStart, 0);
	
	// create the meter task
	xTaskCreate(taskMeter, (signed char*) "METER", STACK_SIZE_METER,
		NULL, tskIDLE_PRIORITY + 1, &hMETERTask);
	hMETERQueue = xQueueCreate(METER_QUEUE_SIZE, sizeof(METER_MSG));
		
	// create the task that handles all MIWI functionality
	xStartMIWITask();
	
	// touchscreen task, this is given higher priority than the TCPIP task
	// so that it can complete its read of the calibration data before
	// the TCPIP task gains complete control of the EEPROM
	xTaskCreate(taskTouchScreen, (signed char*) "TOUCH", STACK_SIZE_TOUCHSCREEN, 
		NULL, tskIDLE_PRIORITY + 4, &hTOUCHTask);

	// QVGA Graphics display task
	xTaskCreate(taskGraphics, (signed char*) "GRAPH", STACK_SIZE_GRAPHICS,
		NULL, tskIDLE_PRIORITY + 3, &hGRAPHICSTask);
		
	// create the task to handle all TCPIP functions (namely HTTP Server)
	xTaskCreate(taskTCPIP, (signed char*) "TCPIP", STACK_SIZE_TCPIP,
		NULL, tskIDLE_PRIORITY + 2, &hTCPIPTask);
		
	// start the RTOS running, this function should never return
	vTaskStartScheduler();
		
	// but in case it does
	return 1;	
}

/*********************************************************************
 * Function:        void InitializeBoard(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Board configuration functions, some functions
 *					are performed in separate tasks (i.e. UART)
 *
 * Note:            
 ********************************************************************/
void InitializeBoard(void)
{
	// set the board for optimal performance
#if defined(__PIC32MX__)
	SYSTEMConfigPerformance(GetSystemClock());
	mOSCSetPBDIV(OSC_PB_DIV_2);	// match the PBus to the fuse setting
	// disable JTAG
#if defined(EXPLORER_16)
	mJTAGPortEnable(0);
#endif
	
	// enable global interrupt handling
	INTEnableSystemMultiVectoredInt();
#endif	

 	LEDInit();		
	TickInit();
	
	// MIWI PICTail+ initialization 
	PHY_RESETn = 0;
    PHY_RESETn_TRIS = 0;
    PHY_CS = 1;
    PHY_CS_TRIS = 0;
    PHY_WAKE = 1;
    PHY_WAKE_TRIS = 0;
    RFIP = 2;
    RFIF = 0;
    RFIE = 0;       // we enable RF interrupts in taskMiWi when ready

	// ethernet IO init
	#if defined(ENC_CS_TRIS)
		ENC_CS_IO = 1;
		ENC_CS_TRIS = 0;
	#elif defined(ENC100_INTERFACE_MODE)
		ENC100_CS_IO = 1;
		ENC100_CS_TRIS = 0;
	#else
		#error "Define either ENC_CS_TRIS or ENC100_INTERFACE_MODE in HardwareProfile.h"
	#endif
	
	/************************************************************************
	* For Explorer 16 RD12 is connected to EEPROM chip select.
	* To prevent a conflict between this EEPROM and SST25 flash
	* RD12 should be pulled up.
	************************************************************************/
	LATDbits.LATD12 = 1;
    TRISDbits.TRISD12 = 0;	

#if defined(__PIC32MX__)
	// Enable SPI1, Set to Master Mode & Set CKE bit 
	// Serial output data changes on transition 
    // from active clock state to Idle clock state
   	SPI1BRG = (GetPeripheralClock() -1ul)/2ul/10000000ul;
    SPI1CONbits.CKE = 1;
    SPI1CONbits.MSTEN = 1;
    SPI1CONbits.ON = 1;
	
	// Enable SPI2, Set to Master Mode & Set CKE bit 
	// Serial output data changes on transition 
    // from active clock state to Idle clock state
    // SPI2 is used by the ENC and the FLASH on GFX3
    SPI2STAT = 0;
    SPI2CON = 0;
    SPI2BRG = 0;
    SPI2CONbits.MSTEN = 1;
    SPI2CONbits.CKP = 0;
    SPI2CONbits.CKE = 0;
    SPI2CONbits.SMP = 0;
    SPI2BRG = (GetPeripheralClock()-1ul)/2ul/10000000ul;
    SPI2CONbits.ON = 1;
#else
	// configure the SPI1 for master mode 8MHz operation
	SPI1CON1bits.PPRE = 0b11;	// primary prescalar 1:1
	SPI1CON1bits.SPRE = 0b110;	// secondary prescalar 2:1 
	SPI1CON1bits.MSTEN = 1;		// master mode enable
	SPI1CON1bits.CKE = 1;		// clock edge select bit
    SPI1STAT = 0x8000;			// clear status and enable SPI

	// configure the SPI2 for master mode 8MHz operation
    // SPI2 is used by the ENC and the FLASH
    SPI2CON1bits.PPRE = 0b11;
    SPI2CON1bits.SPRE = 0b110;
    SPI2CON1bits.MSTEN = 1;
    SPI2CON1bits.CKE = 1;
    SPI2STAT = 0x8000;
#endif
    
    // the touchscreen routine needs access to the FLASH memory
    // so call the initialization routine here
    #if (GRAPHICS_PICTAIL_VERSION == 3)
		// set IO directions not previously set
	    SST25_CS_LAT = 1;
	    SST25_CS_TRIS = 0;
	    SST25_SCK_TRIS = 0;
	    SST25_SDO_TRIS = 0;
	    SST25_SDI_TRIS = 1;
    	SST25Init();
    #else
    	SST39Init();
    #endif	
}

/*********************************************************************
 * Function:        void vApplicationStackOverflowHook(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called if a task overflows its
 *					stack, it can be used for diagnostics
 *
 * Note:            
 ********************************************************************/
void vApplicationStackOverflowHook( void )
{
	/* Look at pxCurrentTCB to see which task overflowed its stack. */
	while (1) {
		portNOP();
	}
}

/*********************************************************************
 * Function:        void vApplicationTickHook(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called at each tick interval
 *
 * Note:            
 ********************************************************************/
void vApplicationTickHook(void)
{
	static unsigned int tickCount = 0;
	static METER_MSG mMsg;
	static GRAPHICS_MSG gMsg;
	portBASE_TYPE xTaskWoken;
	
	// send a temperature update to the graphics task every 2 seconds
	tickCount++;
	if (tickCount >= (2 * configTICK_RATE_HZ)) {
		tickCount = 0;
		
		// update the meters temperature reading
		mMsg.cmd = MSG_METER_UPDATE_TEMPERATURE;
		mMsg.data.wVal[0] = adcTemp;
		xQueueSendToBackFromISR(hMETERQueue, &mMsg, &xTaskWoken);
		
		// also use this as an opportunity to update the display
		// with periodic data updates if required
		gMsg.cmd = MSG_UPDATE_DISPLAY;
		xQueueSendToBackFromISR(hQVGAQueue, &gMsg, &xTaskWoken);
	}	
	
}

