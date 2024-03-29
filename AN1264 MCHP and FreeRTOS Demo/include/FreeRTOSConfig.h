/*
	FreeRTOS.org V5.0.3 - Copyright (C) 2003-2008 Richard Barry.

	This file is part of the FreeRTOS.org distribution.

	FreeRTOS.org is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	FreeRTOS.org is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FreeRTOS.org; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	A special exception to the GPL can be applied should you wish to distribute
	a combined work that includes FreeRTOS.org, without being obliged to provide
	the source code for any proprietary components.  See the licensing section 
	of http://www.FreeRTOS.org for full details of how and when the exception
	can be applied.

    ***************************************************************************
    ***************************************************************************
    *                                                                         *
    * SAVE TIME AND MONEY!  We can port FreeRTOS.org to your own hardware,    *
    * and even write all or part of your application on your behalf.          *
    * See http://www.OpenRTOS.com for details of the services we provide to   *
    * expedite your project.                                                  *
    *                                                                         *
    ***************************************************************************
    ***************************************************************************

	Please ensure to read the configuration and relevant port sections of the
	online documentation.

	http://www.FreeRTOS.org - Documentation, latest information, license and 
	contact details.

	http://www.SafeRTOS.com - A version that is certified for use in safety 
	critical systems.

	http://www.OpenRTOS.com - Commercial support, development, porting, 
	licensing and training services.
*/

///////////////////////////////////////////////////////////////////
//
// Modified for Microchip Stacks and FreeRTOS demo
//
///////////////////////////////////////////////////////////////////

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#if defined(__PIC24F__)
	#include <p24Fxxxx.h>
#else
	#include <p32xxxx.h>
#endif

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION			1
#define configUSE_IDLE_HOOK				0
#define configUSE_TICK_HOOK				1
#define configTICK_RATE_HZ				( ( portTickType ) 200 )

#if defined(__PIC24F__)
// PIC24F
#define configUSE_16_BIT_TICKS			1
#define configMINIMAL_STACK_SIZE		( 115 )
#define configCPU_CLOCK_HZ				(16000000UL)
#else 
// PIC32F
#define configPERIPHERAL_CLOCK_HZ		(40000000UL)
#define configCPU_CLOCK_HZ				(80000000UL)
#define configUSE_16_BIT_TICKS			0
#define configMINIMAL_STACK_SIZE		( 240 )
#define configISR_STACK_SIZE			( 400 )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY	0x04
#endif

#define configMAX_PRIORITIES			( ( unsigned portBASE_TYPE ) 6 )
#define configTOTAL_HEAP_SIZE			( ( size_t ) 0 )
#define configMAX_TASK_NAME_LEN			( 8 )
#define configUSE_TRACE_FACILITY		0
#define configIDLE_SHOULD_YIELD			1
#define configUSE_MUTEXES				1
#define configCHECK_FOR_STACK_OVERFLOW	2

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 		0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet				1
#define INCLUDE_uxTaskPriorityGet				0
#define INCLUDE_vTaskDelete						0
#define INCLUDE_vTaskCleanUpResources			0
#define INCLUDE_vTaskSuspend					1
#define INCLUDE_vTaskDelayUntil					0
#define INCLUDE_vTaskDelay						1
#define INCLUDE_uxTaskGetStackHighWaterMark		1

#define configKERNEL_INTERRUPT_PRIORITY			0x01

#endif /* FREERTOS_CONFIG_H */
