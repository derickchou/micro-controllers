/*****************************************************************************
 * Microchip RTOS and Stacks Demo
 *****************************************************************************
 * FileName:        taskTouchScreen.c
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
 * 1.0      D.Wenn               09/22/08    Modified from QVGA library 1.65
 *											 to be RTOS compliant  
 * 1.1      D.Wenn               09/07/09    Upgraded for Graphics lib V1.75  
 *****************************************************************************/

#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "Graphics\Graphics.h"
#include "SST25VF016.h"
#include "LEDUtils.h"
#include "taskGraphics.h"
#include "taskTouchScreen.h"
#include "taskUART.h"
#include "TCPIP Stack/XEEPROM.h"
#include "homeMeter.h"

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// sampling interval for the Timer routine
#define SAMPLE_PERIOD 1000		// us
#define TIME_BASE (((GetPeripheralClock() / 1000) * SAMPLE_PERIOD) / 8000)
#define PRESS_THRESHOLD 256		// pressure threshold for the touchscreen

///////////////////////////////////////////////////////////////////
// Local Variables and prototypes
void TouchGetCalPoints(WORD* ax, WORD* ay);

// Max/Min Values
volatile WORD _calXMin = XMINCAL;
volatile WORD _calXMax = XMAXCAL;
volatile WORD _calYMin = YMINCAL;
volatile WORD _calYMax = YMAXCAL;

// current ADC values for X and Y channels and pot
// adcPot and adcTemp can only be accessed via atomic actions
// however adcX and adcY are updated as a pair so tasks accessing
// their values must be written accordingly
volatile SHORT adcX		= -1;	// current X or -1 for not pressed
volatile SHORT adcY		= -1;	// same for Y
volatile SHORT adcPot	= 0;	// Potentiometer reading
volatile SHORT adcTemp	= 0;	// Temperature reading
volatile SHORT prevX = -1;
volatile SHORT prevY = -1;
volatile SHORT x, y;

// stages for the ADC/ISR sampling state machine
typedef enum{
	SET_X = 0,
	RUN_X,
	GET_X,
	RUN_CHECK_X,
	CHECK_X,
	SET_Y,
	RUN_Y,
	GET_Y,
	CHECK_Y,
	SET_VALUES,
	GET_POT,
	RUN_POT,
	GET_TEMP,
	RUN_TEMP
} TOUCH_STATES;

volatile TOUCH_STATES touchState = SET_X;

// semaphore to regulate access to the FLASH to read calibration
// constants and one for accessing the QVGA display
extern xSemaphoreHandle FLASHSemaphore;
extern xSemaphoreHandle QVGASemaphore;
// handle to the task
xTaskHandle hTOUCHTask;

// this semaphore is used to indicate when there is new data to be 
// processed by the touch screen task
xSemaphoreHandle hADCSemaphore;

/*********************************************************************
 * Function:        Timer 4 ISR
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Touchscreen ISR. Routine reads the analog inputs
 *					in succession and if it detects a touch event it
 *					then signals via a queue a new event to the
 *					touchscreen handling task
 *
 * Note:            
 ********************************************************************/
#ifdef __PIC32MX__
void __attribute__((interrupt(ipl4), vector(_TIMER_4_VECTOR))) T4InterruptHandler(void)
#else
void __attribute__((interrupt, no_auto_psv)) _T4Interrupt(void)
#endif
{
	static SHORT tempX, tempY, temp;
	portBASE_TYPE taskWoken = pdFALSE;

	// move through the various sampling modes
	switch (touchState) {
		case SET_VALUES:
			if (!AD1CON1bits.DONE)
				break;	
			if ((WORD) ADC1BUF0 > (WORD) PRESS_THRESHOLD) {
				adcX = -1; adcY = -1;
			} else {
				adcX = tempX; adcY = tempY;
			}
			touchState = RUN_POT;
			
		case RUN_POT:
			AD1CHS = ADC_POT;
			AD1CON1bits.SAMP = 1;
			touchState = GET_POT;
			break;
			
		case GET_POT:
			if (!AD1CON1bits.DONE)
				break;	
			#if defined(EXPLORER_16)
				adcPot = ADC1BUF0;
			#else
				adcPot = 511;
			#endif
			touchState = RUN_TEMP;
				
		case RUN_TEMP:
			AD1CHS = ADC_TEMP;
			AD1CON1bits.SAMP = 1;
			touchState = GET_TEMP;
			break;
			
		case GET_TEMP:
			if (!AD1CON1bits.DONE)
				break;	
			#if defined(EXPLORER_16)
				adcTemp = ADC1BUF0;
			#else
				adcTemp = 260;
			#endif
			// since the data is stored as a WORD we can access it
			// atomically without safeguards
			gMeter.temperature = adcTemp;
			touchState = SET_X;
			
		case SET_X:
			AD1CHS = ADC_XPOS;
			ADPCFG_XPOS = 0;
			ADPCFG_YPOS = 1;
			TRIS_XPOS = 1;
			TRIS_YPOS = 1;
			TRIS_XNEG = 1;
			LAT_YNEG = 0;
			TRIS_YNEG = 0;
			
			AD1CON1bits.SAMP = 1;
			touchState = CHECK_X;
			break;
			
		case CHECK_X:
			if (!AD1CON1bits.DONE)
				break;	
			
			if ((WORD) ADC1BUF0 < (WORD) PRESS_THRESHOLD) {
				LAT_YPOS = 1;
				TRIS_YPOS = 0;
				tempX = -1;
				touchState = RUN_X;
			} else {
				adcX = -1; adcY = -1;
				touchState = RUN_POT;
				break;
			}
				
		case RUN_X:
			AD1CON1bits.SAMP = 1;
			touchState = GET_X;
			break;
			
		case GET_X:
			if (!AD1CON1bits.DONE)
				break;	
				
			temp = ADC1BUF0;
			if (temp != tempX) {
				tempX = temp;
				touchState = RUN_X;
				break;
			}
			TRIS_YPOS = 1;
			AD1CON1bits.SAMP = 1;
			touchState = SET_Y;
			break;
				
		case SET_Y:
			if (!AD1CON1bits.DONE)
				break;	
			
			if ((WORD) ADC1BUF0 > (WORD) PRESS_THRESHOLD) {
				adcX = -1; adcY = -1;
				touchState = RUN_POT;
				break;
			}
			
			AD1CHS = ADC_YPOS;
			ADPCFG_XPOS = 1;
			ADPCFG_YPOS = 0;
			
			TRIS_XPOS = 1;
			TRIS_YPOS = 1;
			LAT_XNEG = 0;
			TRIS_XNEG = 0;
			TRIS_YNEG = 1;
			
			AD1CON1bits.SAMP = 1;
			touchState  = CHECK_Y;
			break;
			
		case CHECK_Y:
			if (!AD1CON1bits.DONE)
				break;
			
			if ((WORD) ADC1BUF0 < (WORD) PRESS_THRESHOLD) {
				LAT_XPOS = 1;
				TRIS_XPOS = 0;
				tempY = -1;
				touchState = RUN_Y;
			} else {
				adcX = -1; adcY = -1;
				touchState = RUN_POT;	
				break;
			}
				
		case RUN_Y:
			AD1CON1bits.SAMP = 1;
			touchState = GET_Y;
			break;
			
		case GET_Y:
			if (!AD1CON1bits.DONE)
				break;	
				
			temp = ADC1BUF0;
			if (temp != tempY) {
				tempY = temp;
				touchState = RUN_Y;
				break;
			}
			TRIS_XPOS = 1;
			AD1CON1bits.SAMP = 1;
			touchState = SET_VALUES;
			break;
			
		default:
			touchState = SET_X;
	}
		
	// clear the interrupt flag
	#ifdef __PIC32MX__
		mT4ClearIntFlag();
	#else
		IFS1bits.T4IF = 0;
	#endif
	
	// convert the reading and signal the semaphore if an action detected
	x = TouchGetX(); y = TouchGetY();
	if ((x == -1) || (y == -1)) {
		x = -1; y = -1;
	}
	
	if ((x != prevX) && (y != prevY)) {
		///////////////////////////////////////////////////////////
		// change since last cycle so send wake-up to touchscreen task
		// we use the hADCSemaphore to signal the touchscreen task
		// that it needs to do some processing
		xSemaphoreGiveFromISR(hADCSemaphore, &taskWoken);	
	}
	
	// this macro will allow a higher priority task than the one originally
	// interrupted to be woken after servicing the interrupt
	portEND_SWITCHING_ISR(taskWoken);		
}

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
void TouchLoadCalibration(void)
{
#if (GRAPHICS_PICTAIL_VERSION == 3)
    _calXMin = SST25ReadWord(ADDRESS_XMIN);
    _calXMax = SST25ReadWord(ADDRESS_XMAX);
    _calYMin = SST25ReadWord(ADDRESS_YMIN);
    _calYMax = SST25ReadWord(ADDRESS_YMAX);
#else
	SST39PMPInit();
    _calXMin = SST39ReadWord(ADDRESS_XMIN);
    _calXMax = SST39ReadWord(ADDRESS_XMAX);
    _calYMin = SST39ReadWord(ADDRESS_YMIN);
    _calYMax = SST39ReadWord(ADDRESS_YMAX);
    LCDPMPInit();
#endif
}

/*********************************************************************
* Function: void TouchStoreCalibration(void)
*
* PreCondition: EEPROMInit() must be called before, the EEPROM
*				semaphore must be held
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
void TouchStoreCalibration(void)
{
#if (GRAPHICS_PICTAIL_VERSION == 3)
    SST25SectorErase(ADDRESS_VERSION); // erase 4K sector
    SST25WriteWord(ADDRESS_XMIN, _calXMin);
    SST25WriteWord(ADDRESS_XMAX, _calXMax);
    SST25WriteWord(ADDRESS_YMIN, _calYMin);
    SST25WriteWord(ADDRESS_YMAX, _calYMax);
    SST25WriteWord(ADDRESS_VERSION, GRAPHICS_LIBRARY_VERSION);
#else
	SST39PMPInit();
    SST39SectorErase(ADDRESS_VERSION); // erase 4K sector
    SST39WriteWord(ADDRESS_XMIN, _calXMin);
    SST39WriteWord(ADDRESS_XMAX, _calXMax);
    SST39WriteWord(ADDRESS_YMIN, _calYMin);
    SST39WriteWord(ADDRESS_YMAX, _calYMax);
    SST39WriteWord(ADDRESS_VERSION, GRAPHICS_LIBRARY_VERSION);
    LCDPMPInit();
#endif
}

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
void TouchCalibration(void)
{
	static const XCHAR scr1StrLn1[] = {'I','M','P','O','R','T','A','N','T','.',0};
	static const XCHAR scr1StrLn2[] = {'N','o','w',' ','t','o','u','c','h',' ','s','c','r','e','e','n',' ','c','a','l','i','b','r','a','t','i','o','n',0};
	static const XCHAR scr1StrLn3[] = {'w','i','l','l',' ','b','e',' ','p','e','r','f','o','m','e','d','.','T','o','u','c','h',' ','p','o','i','n','t','s',0};
	static const XCHAR scr1StrLn4[] = {'E','X','A','C','T','L','Y',' ','a','t',' ','t','h','e',' ','p','o','s','i','t','i','o','n','s',0};
	static const XCHAR scr1StrLn5[] = {'s','h','o','w','n',' ','b','y',' ','a','r','r','o','w','s','.',0};
	static const XCHAR scr1StrLn6[] = {'T','o','u','c','h',' ','s','c','r','e','e','n',' ','t','o',' ','c','o','n','t','i','n','u','e','.',0};
	
	static const XCHAR scr2StrLn1[] = {'H','o','l','d',' ','S','3',' ','b','u','t','t','o','n',' ','a','n','d',0};
	static const XCHAR scr2StrLn2[] = {'p','r','e','s','s',' ','M','C','L','R',' ','r','e','s','e','t','(','S','1',')',0};
	static const XCHAR scr2StrLn3[] = {'t','o',' ','R','E','P','E','A','T',' ','t','h','e',' ','c','a','l','i','b','r','a','t','i','o','n',0};
	static const XCHAR scr2StrLn4[] = {'p','r','o','c','e','d','u','r','e','.',0};
	
	SHORT counter;
	SHORT x, y;
	WORD ax[3], ay[3];
	SHORT textHeight;
	
	// Initialize display
    InitGraph();

	SetFont((void*) &GOLFontDefault);
	textHeight = GetTextHeight((void*) &GOLFontDefault);
	
	SetColor(WHITE);
	ClearDevice();
	
    SetColor(BRIGHTRED);
    OutTextXY(0,0*textHeight, (XCHAR*)scr1StrLn1);
    SetColor(BLACK);
    OutTextXY(0,1*textHeight, (XCHAR*)scr1StrLn2);
    OutTextXY(0,2*textHeight, (XCHAR*)scr1StrLn3);
    OutTextXY(0,3*textHeight, (XCHAR*)scr1StrLn4);
    OutTextXY(0,4*textHeight, (XCHAR*)scr1StrLn5);
    SetColor(BRIGHTRED);
    OutTextXY(0,6*textHeight, (XCHAR*)scr1StrLn6);
	
	// wait for touch
	do {
		vTaskDelay(20 / portTICK_RATE_MS);
		x = ADCGetX(); y = ADCGetY();
	} while ((y == -1) || (x == -1));
	
	vTaskDelay(500 / portTICK_RATE_MS);
	
	SetColor(WHITE);
	ClearDevice();
	SetColor(BRIGHTRED);
	
    Line(5,5,5,15);
    Line(4,5,4,15);
    Line(6,5,6,15);

    Line(5,5,15,5);
    Line(5,4,15,4);
    Line(5,6,15,6);

    Line(5,6,15,16);
    Line(5,4,15,14);
    Line(5,5,15,15);

    TouchGetCalPoints(ax, ay);
	
    // Get min X and max Y
    _calYMax = 0;
    _calXMin = 0xFFFF;
    for(counter=0; counter<3; counter++){
        if(_calYMax < ay[counter])
            _calYMax = ay[counter];

        if(_calXMin > ax[counter])
            _calXMin = ax[counter];
    }
	
    SetColor(WHITE);
    ClearDevice();
    SetColor(BRIGHTRED);
    
    Line(5,GetMaxY()-5,5,GetMaxY()-15);
    Line(4,GetMaxY()-5,4,GetMaxY()-15);
    Line(6,GetMaxY()-5,6,GetMaxY()-15);

    Line(5,GetMaxY()-5,15,GetMaxY()-5);
    Line(5,GetMaxY()-4,15,GetMaxY()-4);
    Line(5,GetMaxY()-6,15,GetMaxY()-6);

    Line(5,GetMaxY()-6,15,GetMaxY()-16);
    Line(5,GetMaxY()-4,15,GetMaxY()-14);
    Line(5,GetMaxY()-5,15,GetMaxY()-15);
	
    TouchGetCalPoints(ax, ay);
	
    // Get min Y
    _calYMin = 0xffff;
    for(counter=0; counter<3; counter++){
        if(_calYMin > ay[counter])
            _calYMin = ay[counter];
    }

    SetColor(WHITE);
    ClearDevice();
    SetColor(BRIGHTRED);    

    Line(GetMaxX()-5,GetMaxY()/2-5,GetMaxX()-5,GetMaxY()/2-15);
    Line(GetMaxX()-4,GetMaxY()/2-5,GetMaxX()-4,GetMaxY()/2-15);
    Line(GetMaxX()-6,GetMaxY()/2-5,GetMaxX()-6,GetMaxY()/2-15);

    Line(GetMaxX()-5,GetMaxY()/2-5,GetMaxX()-15,GetMaxY()/2-5);
    Line(GetMaxX()-5,GetMaxY()/2-4,GetMaxX()-15,GetMaxY()/2-4);
    Line(GetMaxX()-5,GetMaxY()/2-6,GetMaxX()-15,GetMaxY()/2-6);

    Line(GetMaxX()-5,GetMaxY()/2-6,GetMaxX()-15,GetMaxY()/2-16);
    Line(GetMaxX()-5,GetMaxY()/2-4,GetMaxX()-15,GetMaxY()/2-14);
    Line(GetMaxX()-5,GetMaxY()/2-5,GetMaxX()-15,GetMaxY()/2-15);

    TouchGetCalPoints(ax, ay);

    // Get max X
    _calXMax = 0;
    for(counter=0; counter<3; counter++){
        if(_calXMax < ax[counter])
            _calXMax = ax[counter];
    }

    SetColor(WHITE);
    ClearDevice();
    SetColor(BLACK);
    
    OutTextXY(10,1*textHeight,(XCHAR*)scr2StrLn1);
    OutTextXY(10,2*textHeight,(XCHAR*)scr2StrLn2);
    OutTextXY(10,3*textHeight,(XCHAR*)scr2StrLn3);
    OutTextXY(10,4*textHeight,(XCHAR*)scr2StrLn4);
    SetColor(BRIGHTRED);
    OutTextXY(10,6*textHeight,(XCHAR*)scr1StrLn6);

    // Wait for touch
    do{
        x=ADCGetX(); y=ADCGetY();
    } while ((y == -1) || (x == -1));

    vTaskDelay(500 / portTICK_RATE_MS);

    SetColor(BLACK);
    ClearDevice();
}

/*********************************************************************
* Function: void TouchGetCalPoints(WORD* ax, WORD* ay)
*
* PreCondition: InitGraph() must be called before
*
* Input: ax - pointer to array receiving 3 X touch positions
*        ay - pointer to array receiving 3 Y touch positions
*
* Output: none
*
* Side Effects: none
*
* Overview: gets values for 3 touches
*
* Note: none
*
********************************************************************/
void TouchGetCalPoints(WORD* ax, WORD* ay)
{
	static const XCHAR calStr[] = {'C','A','L','I','B','R','A','T','I','O','N',0};
	XCHAR calTouchLeft[] = {'3',' ','t','o','u','c','h','e','s',' ','l','e','f','t',0};
	SHORT counter;
	SHORT x,y;

    SetFont((void*)&GOLFontDefault);
    SetColor(BRIGHTRED);
    OutTextXY((GetMaxX() - GetTextWidth((XCHAR*) calStr,(void*) &GOLFontDefault)) >> 1,
              (GetMaxY() - GetTextHeight((void*) &GOLFontDefault)) >> 1,
              (XCHAR*) calStr);

    for(counter = 0; counter < 3; counter++) {
        SetColor(BRIGHTRED);
        calTouchLeft[0] = '3' - counter;

        OutTextXY((GetMaxX() - GetTextWidth(calTouchLeft, (void*)&GOLFontDefault)) >> 1,
                  (GetMaxY() + GetTextHeight((void*) &GOLFontDefault)) >> 1,
                   calTouchLeft);

        // Wait for press
        do{
            x = ADCGetX(); y = ADCGetY();
        }while((y == -1) || (x == -1));

         *(ax + counter) = x; *(ay + counter) = y;
     
        // Wait for release
        do{
            x = ADCGetX(); y = ADCGetY();
        } while ((y != -1) && (x != -1));

        SetColor(WHITE);
        OutTextXY((GetMaxX() - GetTextWidth(calTouchLeft, (void*) &GOLFontDefault)) >> 1,
                  (GetMaxY() + GetTextHeight((void*) &GOLFontDefault)) >> 1,
                   calTouchLeft);

        vTaskDelay(500 / portTICK_RATE_MS);
    }
}

/*********************************************************************
* Function: SHORT TouchGetX()
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
SHORT TouchGetX(void)
{
	long result;

    result = ADCGetX();

	if (result >= 0) {
    	result -= _calXMin;
    	result *= GetMaxX();
    	result = result / ((long) _calXMax - (long) _calXMin);
 
#ifdef FLIP_X
        result = GetMaxX()- result;
#endif
    }
    
    return result;
}

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
SHORT TouchGetY(void)
{
	long result;

    result = ADCGetY();

    if (result >= 0) {
        result -= _calYMin;
        result *= GetMaxY();
        result = result / ((long) _calYMax - (long) _calYMin);

#ifdef FLIP_Y
        result = GetMaxY()- result;
#endif
    }
    
    return result;
}

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
 *					events from the graphics drawing improving
 *					overall responsiveness
 *
 * Note:            
 ********************************************************************/
void taskTouchScreen(void* pvParameter)
{
	WORD version;
	static GRAPHICS_MSG msg;
	
	// notify task started
	UARTprintf("TOUCH: Task Started.\r\n");

	// configure the ADC
	AD1CON1 = 0x80E0; 
	AD1CON2 = 0x0000;
	AD1CON3 = 0x1F80;
	AD1CHS = ADC_TEMP;
	AD1CSSL = 0;
	
	ADPCFG_TEMP = 0;
	ADPCFG_POT = 0;
	ADPCFG_XPOS = 0;
	ADPCFG_YPOS = 0;
	
	// create the semaphore used to signal touch activity from the ADC ISR
	vSemaphoreCreateBinary(hADCSemaphore);
	xSemaphoreTake(hADCSemaphore, portMAX_DELAY);

	// we may need to access the QVGA display to perform calibration
	// To prevent the Graphics task running we immediately take
	// the QVGA semaphore and only return it when startup is complete
	xSemaphoreTake(QVGASemaphore, portMAX_DELAY);
	
	// we initialize TMR4 here to control the sampling of the ADC
	// channels. 
	#ifdef __PIC32MX__
		OpenTimer4(T4_ON | T4_PS_1_8, TIME_BASE - 1);
		ConfigIntTimer4(T4_INT_ON | T4_INT_PRIOR_4);
	#else
		TMR4 = 0;
		PR4 = TIME_BASE - 1;
		T4CONbits.TCKPS = 1;	// prescale 1:8
		IFS1bits.T4IF = 0;
		IEC1bits.T4IE = 1;
		IPC6bits.T4IP = 4;
		T4CONbits.TON = 1;
	#endif

	// read the configuration information from the FLASH 
	if (xSemaphoreTake(FLASHSemaphore, 0) != pdTRUE) {
		UARTprintf("TOUCH: Unable to access FLASH\r\n");
		vTaskSuspend(NULL);
	}
	
	#if (GRAPHICS_PICTAIL_VERSION == 3)
		version = SST25ReadWord(ADDRESS_VERSION);
	#else
		SST39PMPInit();
		version = SST39ReadWord(ADDRESS_VERSION);
		LCDPMPInit();
	#endif
	
	// check the version, if it is not correct then we take control
	// of the QVGA display and perform touchscreen calibration
	if ((version != GRAPHICS_LIBRARY_VERSION) || (BUTTON3_IO == 0)) {
		UARTprintf("TOUCH: Performing calibration\r\n");
		
		// call the calibration routine
		TouchCalibration();
		TouchStoreCalibration();
	}
	
	TouchLoadCalibration();	
	
	// we are done with the FLASH now so release the semaphore
	xSemaphoreGive(FLASHSemaphore);
	
	// release control of the QVGA display
	xSemaphoreGive(QVGASemaphore);
	
	// any messages that we send will always be of this type
	msg.cmd = MSG_TOUCH_EVENT;
	msg.data.golMsg.type = TYPE_TOUCHSCREEN;
	
	// read the touchscreen. If the screen has been
	// touched then send a message to the graphics display
	// task. 
	while (1) {
		///////////////////////////////////////////////////////////
		// use a semaphore to wait for updates from the ISR
		// block forever waiting for hADCSemaphore to arrive
		xSemaphoreTake(hADCSemaphore, portMAX_DELAY);
		
		// analyze the current screen state and send an update to the
		// graphics task
		// something has changed (press, release or move)
		if ((prevX == -1) && (prevY == -1)) {
			// screen pressed
			msg.data.golMsg.uiEvent = EVENT_PRESS;
			msg.data.golMsg.param1 = x;
			msg.data.golMsg.param2 = y;
		} else if ((x == -1) && (y== -1)) {
			// screen released
			msg.data.golMsg.uiEvent = EVENT_RELEASE;
			msg.data.golMsg.param1 = prevX;
			msg.data.golMsg.param2 = prevY;
		} else {
			// screen moved
			msg.data.golMsg.uiEvent = EVENT_MOVE;
			msg.data.golMsg.param1 = x;
			msg.data.golMsg.param2 = y;				
		}
		
		// send a message to update the graphics task
		xQueueSend(hQVGAQueue, &msg, portMAX_DELAY);
		prevX = x; prevY = y;		
	}	
}

