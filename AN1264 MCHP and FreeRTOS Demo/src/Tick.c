/*****************************************************************************
 * Microchip RTOS and Stacks Demo
 *****************************************************************************
 * FileName:        Tick.c
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
 *                                           This is a local version of the
 *                                           Tick.c and SymbolTime files
 *                                           used in MiWi and TCPIP
 *                                           it provides compatibility.   
 *****************************************************************************/

#include "HardwareProfile.h"

// include device specific peripheral libraries
#if defined(__PIC24F__)
	#define USE_AND_OR
	#include <p24fxxxx.h>
	#include <timer.h>
#else
	#include <plib.h>
	#include <GenericTypeDefs.h>
#endif

#include "FreeRTOS.h"
#include "Common\SymbolTime.h"

// internal counter to store the upper 16 bits of the tick value
static WORD wInternalTicks;

/*********************************************************************
 * Function:        void TickInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          None
 *
 * Side Effects:    RTOS is not running at this point
 *
 * Overview:        Performs initialization of the TICK counter used
 *                  by MiWi and TCPIP. MiWi uses 48bit ticks where as
 *                  TCPIP can use 48 bits. This code cascades T2 and T3
 *                  to form a 32bit hardware counter and then uses
 *                  a timer interrupt routine to implement the upper
 *                  16 bits. 
 *
 * Note:            
 ********************************************************************/
void TickInit(void)
{
	wInternalTicks = 0;
	
#if defined(__PIC24F__)
    // timer.h in Microchip/include conflicts with the plib version
    // so we must explicitly set up the timer for PIC24F
    TMR2 = 0;
    TMR3 = 0;
    PR2 = 0xFFFF;
    PR3 = 0xFFFF;
    T2CONbits.T32 = 1;
    T2CONbits.TCKPS = 3;   // 1:256
    T2CONbits.TON = 1;
    IFS0bits.T3IF = 0;    
    IPC2bits.T3IP = 2;
    IEC0bits.T3IE = 1;
#else
	WriteTimer23(0x00000000);
	OpenTimer23(T2_ON | T2_32BIT_MODE_ON | T2_PS_1_256 , 0xFFFFFFFF);
	ConfigIntTimer23(T23_INT_PRIOR_2 | T23_INT_ON);
#endif
}

/*********************************************************************
 * Function:        Timer 23 ISR
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Timer23 ISR increments the high resolution tick
 *					counter used by TCPIP and MiWi, this interrupt
 *					is triggered approx every 7 hours on a PIC32
 *					(40MHz * 256 prescalar * 2^32) and every ~19
 *					hours on a PIC24
 *
 * Note:            
 ********************************************************************/
#ifdef __PIC32MX__
void __attribute__((interrupt(ipl2), vector(_TIMER_3_VECTOR))) T3InterruptHandler(void)
#else
void __attribute__((interrupt, auto_psv)) _T3Interrupt(void)
#endif
{
	// increment the upper 16bits counter
	wInternalTicks++;

	// clear the interrupt flag
#ifdef __PIC32MX__
	mT3ClearIntFlag();
#else
	IFS0bits.T3IF = 0;
#endif
}

/*********************************************************************
 * Function:        static void GetTickCopy(BYTE* pData)
 *
 * PreCondition:    None
 *
 * Input:           Pointer to byte counter store
 *                  
 * Output:          32bit TICK value
 *
 * Side Effects:    RTOS assumed to be running
 *
 * Overview:        Return the 48bit count value in a thread safe
 *					manner
 *
 * Note:            
 ********************************************************************/
static void GetTickCopy(WORD* pData)
{
#if defined(__PIC32MX__)
	unsigned int tVal;
#endif

	// create a critical section
	portENTER_CRITICAL();
#if defined(__PIC24F__)
	*pData++ = TMR2;
	*pData++ = TMR3HLD;
	*pData = wInternalTicks;
#else
	// pic32
	tVal = ReadTimer23();
	*((DWORD*) pData) = tVal;
	pData += 2;
	*pData = wInternalTicks;
#endif
	portEXIT_CRITICAL();
}

/*********************************************************************
 * Function:        DWORD TickGet(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          32bit TICK value
 *
 * Side Effects:    RTOS assumed to be running
 *
 * Overview:        Return a 32 bit TICK value from TMR2:3
 *
 * Note:            
 ********************************************************************/
DWORD TickGet(void)
{
	DWORD res;
	
	portENTER_CRITICAL();
#if defined(__PIC24F__)
	res = TMR2;
	res |= (DWORD) TMR3HLD << 16;
#else
	// pic32
	res = ReadTimer23();
#endif
	portEXIT_CRITICAL();
	
	return res;
}

/*********************************************************************
 * Function:        DWORD TickGetDiv256(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          32bit TICK value
 *
 * Side Effects:    RTOS assumed to be running
 *
 * Overview:        Return a 32 bit TICK value from TMR2:3 and the
 *					upper 16 bit counter
 *
 * Note:            
 ********************************************************************/
DWORD TickGetDiv256(void)
{
	BYTE bVal[6];
	DWORD_VAL ret;
	
	GetTickCopy((WORD*) bVal);
	ret.v[0] = bVal[1];
	ret.v[1] = bVal[2];
	ret.v[2] = bVal[3];
	ret.v[3] = bVal[4];	
	
	return ret.Val;
}

/*********************************************************************
 * Function:        DWORD TickGetDiv64k(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          32bit TICK value
 *
 * Side Effects:    RTOS assumed to be running
 *
 * Overview:        Return a 32 bit TICK value from TMR2:3 and the
 *					upper 16 bit counter
 *
 * Note:            
 ********************************************************************/
DWORD TickGetDiv64k(void)
{
	BYTE bVal[6];
	DWORD_VAL ret;
	
	GetTickCopy((WORD*) bVal);
	ret.v[0] = bVal[2];
	ret.v[1] = bVal[3];
	ret.v[2] = bVal[4];
	ret.v[3] = bVal[5];	
	
	return ret.Val;
}
/*********************************************************************
 * Function:        MIWITICK MiWiTickGet(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          24bit MIWITICK value
 *
 * Side Effects:    RTOS assumed to be running
 *
 * Overview:        Return a 24 bit MIWITICK value from TMR2:3
 *
 * Note:            
 ********************************************************************/
MIWITICK MiWiTickGet(void)
{
	MIWITICK res;
	
	portENTER_CRITICAL();
#if defined(__PIC24F__)
	res.word.w0 = TMR2;
	res.word.w1 = TMR3HLD;
#else
	// PIC32
	res.Val = ReadTimer23();
#endif
	portEXIT_CRITICAL();
	
	return res;
}

