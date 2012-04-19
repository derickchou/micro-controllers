/*****************************************************************************
 * Microchip RTOS and Stacks Demo
 *****************************************************************************
 * FileName:        taskGraphics.c
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
 * 1.1      D.Wenn               09/07/09    Modified for new stacks
 *****************************************************************************/

#include "Graphics\Graphics.h"
#include "TCPIP Stack/TCPIP.h"
#include "FreeRTOS.h"
#include "Task.h"
#include "taskTouchScreen.h"
#include "string.h"
#include "taskGraphics.h"
#include "taskUART.h"
#include "taskMIWI.h"
#include "taskTCPIP.h"
#include "homeMeter.h"
#include <stdio.h>

///////////////////////////////////////////////////////////////////
// Flash images used by the application
extern const FONT_FLASH GOLFontDefault;
extern const FONT_FLASH GOLMediumFont;
extern const FONT_FLASH GOLSmallFont;

// for the PIC24F 128k part the bitmaps are not used so we #define
// the code out
#if !defined(__PIC24FJ128GA010__)
extern const BITMAP_FLASH mchpmeter;
extern const BITMAP_FLASH gasflamesmall;
extern const BITMAP_FLASH electricsmall;
extern const BITMAP_FLASH sunnyshowers;
extern const BITMAP_FLASH cloudy;
#endif

///////////////////////////////////////////////////////////////////
// external references
extern xSemaphoreHandle QVGASemaphore;
// task handle
xTaskHandle hGRAPHICSTask;
// TCPIP configuration block
extern APP_CONFIG AppConfig;

///////////////////////////////////////////////////////////////////
// Screens used in the demo
typedef enum {
	CREATE_SCREEN_MAIN = 0,
	DISPLAY_SCREEN_MAIN,
	CREATE_SCREEN_GAS,
	DISPLAY_SCREEN_GAS,
	CREATE_SCREEN_ELECTRICITY,
	DISPLAY_SCREEN_ELECTRICITY,
	CREATE_SCREEN_RTOS,
	DISPLAY_SCREEN_RTOS
} SCREEN_STATES;

// current screen state
SCREEN_STATES screenState = CREATE_SCREEN_MAIN;

///////////////////////////////////////////////////////////////////
// display color schemes
GOL_SCHEME* btnSchemeSmall;		// black buttons, small font
GOL_SCHEME* btnSchemeMedium;	// black buttons, medium font
GOL_SCHEME* blueScheme;			// Blue title pane
GOL_SCHEME* greenScheme;		// Green display button
GOL_SCHEME* whiteScheme;		// standard white button scheme
GOL_SCHEME* blackScheme;		// black scheme
GOL_SCHEME* redScheme;			// Red display buttons

///////////////////////////////////////////////////////////////////
// defines for the window and menu items
#define ID_WINDOW1					10
#define ID_WINDOW2					11
#define ID_WINDOW3					12
#define ID_WINDOW4					13
#define ID_BUTTON_GAS				20
#define ID_BUTTON_ELECTRIC			21
#define ID_BUTTON_TEMPERATURE		22
#define ID_ELECTRIC_TOTAL_COST		23
#define ID_GAS_TOTAL_COST			24
#define ID_PICTURE_WEATHER			25
#define ID_SETPOINT_DIAL			26
#define ID_SETPOINT_TEMPERATURE		27
#define ID_BUTTON_GAS_OFF			28
#define ID_BUTTON_GAS_ON			29
#define ID_GAS_SCREEN_TOTAL			30
#define ID_GAS_SCREEN_COST			31
#define ID_BUTTON_ELECTRIC_OFF		32
#define ID_BUTTON_ELECTRIC_ON		33
#define ID_ELECTRIC_SCREEN_TOTAL	34
#define ID_ELECTRIC_SCREEN_COST		35
#define ID_LABEL_COST				36
#define ID_LABEL_UNITS				37
#define ID_PICT_GAS					50
#define ID_PICT_ELECTRIC			51

///////////////////////////////////////////////////////////////////
// Colors
#define RED4					RGB565CONVERT(139,   0,   0)
#define FIREBRICK1				RGB565CONVERT(255,  48,  48)
#define DARKGREEN				RGB565CONVERT(  0, 100,   0)
#define PALEGREEN				RGB565CONVERT(152, 251, 152)

///////////////////////////////////////////////////////////////////
// local functions within this module
void CreateMain(void);
void CreateGasScreen(void);
void CreateElectricityScreen(void);
void CreateRTOSScreen(void);
void UpdateRTOSScreen(void);
void UpdateUsageGraph(void);
void DrawRTOSStack(XCHAR* sTitle, short ypos, short stack_used, short stack_size);
WORD msgMain(WORD objMsg, OBJ_HEADER* pObj);
WORD msgGas(WORD objMsg, OBJ_HEADER* pObj);
WORD msgElectric(WORD objMsg, OBJ_HEADER* pObj);
WORD msgRTOS(WORD objMsg, OBJ_HEADER* pObj);
void CreateSchemes(void);

///////////////////////////////////////////////////////////////////
// The graphics display uses several strings to display data
// to save RAM we re-use the strings between the various screens
static char qvgaBuff1[20];
static char qvgaBuff2[20];
static char qvgaBuff3[20];
static char qvgaBuff4[20];

///////////////////////////////////////////////////////////////////
// All display update messages are sent to the graphics task
// using this queue. Messages include touch screen updates
// as well as updates to the displayed parameters
xQueueHandle hQVGAQueue;

// x position store when displaying graphs and last value
WORD usage_xpos;
WORD usage_last_value;

/*********************************************************************
 * Function:        void taskGraphics(void* pvParameter)
 *
 * PreCondition:    None
 *
 * Input:           Pointer to optional parameter
 *                  
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Graphics display task
 *
 * Note:            
 ********************************************************************/
 int pressx, pressy;
 
void taskGraphics(void* pvParameter)
{
	static GRAPHICS_MSG msg;
	GOL_MSG* pMsg;
	OBJ_HEADER* pObj;
	
	// notify task started
	UARTprintf("GRAPH: Task Started.\r\n");
	
	// this semaphore will initially be taken by the touchscreen task
	// in case it needs to perform calibration we use to pause
	// this task until the touchscreen has finished startup
	xSemaphoreTake(QVGASemaphore, portMAX_DELAY);
	
	// create the color and style schemes used by the various screens
	// and perform GOL startup
	CreateSchemes();
	xSemaphoreGive(QVGASemaphore);
	
	// graphics task main loop
	while (1) {
		// gain access to the QVGA display and PMP
		xSemaphoreTake(QVGASemaphore, portMAX_DELAY);
		// when drawing is complete GOLDraw will return TRUE
		while (GOLDraw() != TRUE);
		xSemaphoreGive(QVGASemaphore);
		
		// block until we receive a new message from the graphics
		// queue. This message could be from the touchscreen or any
		// other task that may cause an update
		if (xQueueReceive(hQVGAQueue, &msg, portMAX_DELAY) == pdTRUE) {
			// perform message specific processing
			switch (msg.cmd) {
				case MSG_UPDATE_DISPLAY:
					// force a redraw of the screen, this allows for
					// periodic updates of the screen
					if (screenState == DISPLAY_SCREEN_RTOS) {
						xSemaphoreTake(QVGASemaphore, portMAX_DELAY);
						UpdateRTOSScreen();
						xSemaphoreGive(QVGASemaphore);
					} else if ((screenState == DISPLAY_SCREEN_ELECTRICITY) ||
							 (screenState == DISPLAY_SCREEN_GAS)) {
						xSemaphoreTake(QVGASemaphore, portMAX_DELAY);
						UpdateUsageGraph();
						xSemaphoreGive(QVGASemaphore);
					} else
						GOLDrawCallback();
					break;
				case MSG_UPDATE_TEMPERATURE:
					// update the real temperature display
					pObj = GOLFindObject(ID_BUTTON_TEMPERATURE);
					if (pObj) {
						sprintf(qvgaBuff1, "%d.%01dC", msg.data.wVal[0] / 10,
							msg.data.wVal[0] % 10);
						SetState((BUTTON*) pObj, DRAW_UPDATE);
					}
					break;
				case MSG_UPDATE_ELECTRIC_TOTAL:
					// update electricity total cost
					pObj = GOLFindObject(ID_ELECTRIC_TOTAL_COST);
					if (pObj) {
						sprintf(qvgaBuff2, "$%d.%02d",
							msg.data.wVal[1], msg.data.wVal[0]);
						SetState((BUTTON*) pObj, DRAW_UPDATE);
					}
					break;
				case MSG_UPDATE_ELECTRIC_UNITS:
					// update electricity total units used
					pObj = GOLFindObject(ID_ELECTRIC_SCREEN_TOTAL);
					if (pObj) {
						sprintf(qvgaBuff1, "%ld", msg.data.dVal[0]);
						SetState((BUTTON*) pObj, DRAW_UPDATE);							
					}
					break;
				case MSG_UPDATE_GAS_TOTAL:
					// update gas total cost
					pObj = GOLFindObject(ID_GAS_TOTAL_COST);
					if (pObj) {
						sprintf(qvgaBuff3, "$%d.%02d",
							msg.data.wVal[1], msg.data.wVal[0]);
						SetState((BUTTON*) pObj, DRAW_UPDATE);
					}
					break;
				case MSG_UPDATE_GAS_UNITS:
					// update gas total units used
					pObj = GOLFindObject(ID_GAS_SCREEN_TOTAL);
					if (pObj) {
						sprintf(qvgaBuff1, "%ld", msg.data.dVal[0]);
						SetState((BUTTON*) pObj, DRAW_UPDATE);							
					}
					break;
				case MSG_TOUCH_EVENT:
					// process events from the touchscreen
					pMsg = &msg.data.golMsg;
					pressx = pMsg->param1;
					pressy = pMsg->param2;
					GOLMsg(pMsg);
					break;
				default:
					break;
			}
		}	
	}
}

/*********************************************************************
 * Function:       void CreateSchemes(void) 
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Create the styles used by the various screens
 *					Since the creation process involves calls
 *					to malloc/free vai the GOL we suspend all
 *					other tasks from executing because the standard
 *					malloc/free are not re-entrant
 *
 * Note:            
 ********************************************************************/
void CreateSchemes(void)
{
	vTaskSuspendAll();
	
	// Initialize the GOL
	GOLInit();
	
	// create the display schemes
	btnSchemeSmall = GOLCreateScheme();
	btnSchemeSmall->EmbossDkColor = RGB565CONVERT(0x1E, 0x00, 0xE5);
	btnSchemeSmall->EmbossLtColor = RGB565CONVERT(0xA9, 0xDB, 0xEF);
	btnSchemeSmall->TextColor0 = RGB565CONVERT(0x1E, 0x00, 0xE5);
	btnSchemeSmall->TextColor1 = RGB565CONVERT(0x1E, 0x00, 0xE5);
	btnSchemeSmall->TextColorDisabled = RGB565CONVERT(0xFF, 0xFF, 0xFF);
	btnSchemeSmall->Color0 = RGB565CONVERT(0x1E, 0x00, 0xE5);
	btnSchemeSmall->Color1 = RGB565CONVERT(0x1E, 0x00, 0xE5);
	btnSchemeSmall->ColorDisabled = RGB565CONVERT(0x1E, 0x00, 0xE5);
	btnSchemeSmall->CommonBkColor = RGB565CONVERT(0x1E, 0x00, 0xE5);
	btnSchemeSmall->pFont = (void*) &GOLSmallFont;
	
	btnSchemeMedium = GOLCreateScheme();
	memcpy(btnSchemeMedium, btnSchemeSmall, sizeof(GOL_SCHEME));
	btnSchemeMedium->pFont = (void*) &GOLMediumFont;

	blueScheme = GOLCreateScheme();
	blueScheme->TextColor0 = BLACK;
	blueScheme->TextColor1 = BLACK;
	blueScheme->TextColorDisabled = BLACK;
	
	greenScheme = GOLCreateScheme();
	greenScheme->Color0 = RGB565CONVERT(0x23, 0x9E, 0x0A);
	greenScheme->Color1 = BRIGHTGREEN;
	greenScheme->ColorDisabled = RGB565CONVERT(0x23, 0x9E, 0x0A);
	greenScheme->EmbossDkColor = DARKGREEN;
	greenScheme->EmbossLtColor = PALEGREEN;
	greenScheme->TextColor0 = RGB565CONVERT(0xFF, 0xFF, 0xFF);
	greenScheme->TextColor1 = BLACK;
	greenScheme->TextColorDisabled = RGB565CONVERT(0xDF, 0xAC, 0x83);

	redScheme = GOLCreateScheme();
	redScheme->Color0 = RGB565CONVERT(0xCC, 0x00, 0x00);
	redScheme->Color1 = BRIGHTRED;
	redScheme->EmbossDkColor = RED4;
	redScheme->EmbossLtColor = FIREBRICK1;
	redScheme->TextColor0 = RGB565CONVERT(0xC8, 0xD5, 0x85);
	redScheme->TextColor1 = BLACK;

	whiteScheme = GOLCreateScheme();
	whiteScheme->EmbossDkColor = RGB565CONVERT(0x40, 0x40, 0x40);
	whiteScheme->EmbossLtColor = RGB565CONVERT(0xE0, 0xE0, 0xE0);
	whiteScheme->TextColor0 = RGB565CONVERT(0x00, 0x00, 0x00);
	whiteScheme->TextColor1 = RGB565CONVERT(0x00, 0x00, 0x00);
	whiteScheme->TextColorDisabled = RGB565CONVERT(0x00, 0x00, 0x00);
	whiteScheme->Color0 = RGB565CONVERT(0xA0, 0xA0, 0xA0);
	whiteScheme->Color1 = RGB565CONVERT(0xA0, 0xA0, 0xA0);
	whiteScheme->ColorDisabled = RGB565CONVERT(0xFF, 0xFF, 0xFF);
	whiteScheme->CommonBkColor = RGB565CONVERT(0xFF, 0xFF, 0xFF);
	whiteScheme->pFont = (void*) &GOLFontDefault;
	
	blackScheme = GOLCreateScheme();
	memcpy(blackScheme, btnSchemeSmall, sizeof(GOL_SCHEME));
	blackScheme->Color0 = BLACK;
	blackScheme->CommonBkColor = BLACK;
	blackScheme->TextColor0 = RGB565CONVERT(0xFF, 0xFF, 0xFF);
	blackScheme->TextColor1 = BLACK;
	blackScheme->pFont = (void*) &GOLMediumFont;
	
	xTaskResumeAll();
}

/*********************************************************************
 * Function:        WORD GOLMsgCallback(WORD objMsg, OBJ_HEADER* pObj, 
 *										GOL_MSG* pMsg);
 *
 * PreCondition:    None
 *
 * Input:           objMsg full message, 
 *					pObj pointer to header,
 *					pMsg the message
 *                  
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Process a screen specific message or call
 *					the default handler
 *
 * Note:            
 ********************************************************************/
WORD GOLMsgCallback(WORD objMsg, OBJ_HEADER* pObj, GOL_MSG* pMsg)
{
	// process messages for the demo screens
	switch (screenState) {
		case DISPLAY_SCREEN_MAIN:
			return msgMain(objMsg, pObj);
		case DISPLAY_SCREEN_GAS:
			return msgGas(objMsg, pObj);
		case DISPLAY_SCREEN_ELECTRICITY:
			return msgElectric(objMsg, pObj);
		case DISPLAY_SCREEN_RTOS:
			return msgRTOS(objMsg, pObj);
		default:
			break;
	}
		
	return 1;	
}

/*********************************************************************
 * Function:        WORD GOLDrawCallback(void);
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handle application specific drawing, returning
 *					non-zero to allow the GOL to handle drawing
 *
 * Note:            
 ********************************************************************/
WORD GOLDrawCallback(void)
{
	// process the draw messages
	switch (screenState) {
		case CREATE_SCREEN_MAIN:
			CreateMain();
			screenState = DISPLAY_SCREEN_MAIN;
			return 1;
		case DISPLAY_SCREEN_MAIN:
			return 1;
		case CREATE_SCREEN_GAS:
			CreateGasScreen();
			screenState = DISPLAY_SCREEN_GAS;
			return 1;
		case DISPLAY_SCREEN_GAS:
			return 1;
		case CREATE_SCREEN_ELECTRICITY:
			CreateElectricityScreen();
			screenState = DISPLAY_SCREEN_ELECTRICITY;
			return 1;
		case DISPLAY_SCREEN_ELECTRICITY:
			return 1;
		case CREATE_SCREEN_RTOS:
			CreateRTOSScreen();
			screenState = DISPLAY_SCREEN_RTOS;
			return 1;
		case DISPLAY_SCREEN_RTOS:
			return 1;
	}
	
	return 1;	
}

/*********************************************************************
 * Function:        void CreateMain(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Create the main menu screen
 *
 * Note:            
 ********************************************************************/
void CreateMain(void)
{
	GOLFree();
	SetColor(BLACK);
	ClearDevice();
	
	// to keep the time that the meter semaphore is held to a minimum
	// we define the strings early on and then release it
	xSemaphoreTake(METERSemaphore, portMAX_DELAY);
	sprintf(qvgaBuff1, "%d.%01dC", gMeter.temperature / 10,
			gMeter.temperature % 10);
	sprintf(qvgaBuff2, "$%ld.%02ld", gMeter.electric_total / 100, 
		gMeter.electric_total % 100);
	sprintf(qvgaBuff3, "$%ld.%02ld", gMeter.gas_total / 100,
		gMeter.gas_total % 100);	
	sprintf(qvgaBuff4, "%d.%01dC", gMeter.setpoint / 10,
			gMeter.setpoint % 10);
	xSemaphoreGive(METERSemaphore);
	
	// draw the title message
	BtnCreate(ID_WINDOW1, 0, 0, GetMaxX(), 39, 0,
		BTN_DRAW, NULL, "Microchip Metering Demo", blueScheme);

	// set point display creation
	StCreate(ID_SETPOINT_TEMPERATURE, 80, 40, GetMaxX() - 80, 59,
		ST_DRAW | ST_CENTER_ALIGN, qvgaBuff4, blackScheme);
	// set point temperature dial
	RdiaCreate(ID_SETPOINT_DIAL, 160, 120,
		60, RDIA_DRAW, 5, gMeter.setpoint, 350, blueScheme);

	// real temperature display
	BtnCreate(ID_BUTTON_TEMPERATURE, 80, GetMaxY() - 40,
		239, GetMaxY(), 10,
		BTN_DRAW | BTN_DISABLED, NULL, qvgaBuff1, greenScheme);

	// the electricity and gas buttons are created depending
	// on device type. Larger memory parts include a bitmap		
#if defined(__PIC24FJ128GA010__)
	BtnCreate(ID_BUTTON_ELECTRIC, 0, 40,
		79, GetMaxY() - 40, 10,
		BTN_DRAW, NULL, "Electric", blackScheme);
	BtnCreate(ID_BUTTON_GAS, 240, 40, 
		GetMaxX(), GetMaxY() - 40, 10,
		BTN_DRAW, NULL, "Gas", blackScheme);
#else
	BtnCreate(ID_BUTTON_ELECTRIC, 0, 40,
		79, GetMaxY() - 40, 10,
		BTN_DRAW, (void*) &electricsmall, "Electric", btnSchemeSmall);
	BtnCreate(ID_BUTTON_GAS, 240, 40, 
		GetMaxX(), GetMaxY() - 40, 10,
		BTN_DRAW, (void*) &gasflamesmall, "Gas", btnSchemeSmall);
#endif	
	
	// create the billing total displays at the bottom
	BtnCreate(ID_ELECTRIC_TOTAL_COST, 0, GetMaxY() - 40, 
		79, GetMaxY(), 10,
		BTN_DRAW | BTN_DISABLED, NULL, qvgaBuff2, btnSchemeMedium);

	BtnCreate(ID_GAS_TOTAL_COST, 240, GetMaxY() - 40, 
		GetMaxX(), GetMaxY(), 10,
		BTN_DRAW | BTN_DISABLED, NULL, qvgaBuff3, btnSchemeMedium);	

}

/*********************************************************************
 * Function:        void CreateGasScreen(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Create the gas menu screen
 *
 * Note:            
 ********************************************************************/
void CreateGasScreen(void)
{
	WORD gas_on;
	
	GOLFree();	
	SetColor(BLACK);
	ClearDevice();
	
	// display the title message
	BtnCreate(ID_WINDOW2,
		0, 0, GetMaxX(), 39, 0,
		BTN_DRAW, NULL, "Gas", blueScheme);
	
	// title logo
#if !defined(__PIC24FJ128GA010__)
	PictCreate(ID_PICT_GAS, 80, 40, 
		239, 199, PICT_DRAW | PICT_DISABLED, 2,
		(void*) &gasflamesmall, blackScheme);
#endif
		
	// get the data from the meter object
	xSemaphoreTake(METERSemaphore, portMAX_DELAY);
	gas_on = gMeter.gas_on;
	sprintf(qvgaBuff1, "%ld", gMeter.gas_units);
	// we use qvgaBuff3 to match the buffer used on the main display screen
	// this means that one update routine (in the main draw code) can be
	// used to update controls in both the main and gas code with no extra
	// overhead
	sprintf(qvgaBuff3, "$%ld.%02ld", gMeter.gas_total / 100, 
		gMeter.gas_total % 100);
	xSemaphoreGive(METERSemaphore);
		
	// button in enabled or disabled state
	if (gas_on == 1) {
		BtnCreate(ID_BUTTON_GAS_ON,
			0, 40, 79, 139, 20,
			BTN_DRAW | BTN_PRESSED, NULL, "ON", greenScheme);
		BtnCreate(ID_BUTTON_GAS_OFF,
			0, 140, 79, 239, 20,
			BTN_DRAW, NULL, "OFF", redScheme);
	} else {
		BtnCreate(ID_BUTTON_GAS_ON,
			0, 40, 79, 139, 20,
			BTN_DRAW, NULL, "ON", greenScheme);
		BtnCreate(ID_BUTTON_GAS_OFF,
			0, 140, 79, 239, 20,
			BTN_DRAW | BTN_PRESSED, NULL, "OFF", redScheme);
	}	
	
	// buttons with total units and cost
	StCreate(ID_LABEL_UNITS, 240, 40, GetMaxX(), 59,
		ST_DRAW | ST_CENTER_ALIGN, "Units", blackScheme);
	BtnCreate(ID_GAS_SCREEN_TOTAL,
		240, 60, GetMaxX(), 139, 10,
		BTN_DRAW | BTN_DISABLED, NULL, qvgaBuff1, btnSchemeMedium);

	StCreate(ID_LABEL_COST, 240, 140, GetMaxX(), 159,
		ST_DRAW | ST_CENTER_ALIGN, "Cost", blackScheme);
	BtnCreate(ID_GAS_TOTAL_COST,
		240, 160, GetMaxX(), GetMaxY(), 10,
		BTN_DRAW | BTN_DISABLED, NULL, qvgaBuff3, btnSchemeMedium);
	
	// zero the graph index
	usage_xpos = 0xFFFF;
	usage_last_value = 0xFFFF;
	
}

/*********************************************************************
 * Function:        void CreateElectricityScreen(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Create the Electric menu screen
 *
 * Note:            
 ********************************************************************/
void CreateElectricityScreen(void)
{
	WORD electric_on;
	
	GOLFree();	
	SetColor(BLACK);
	ClearDevice();
	
	// display the title message
	BtnCreate(ID_WINDOW3,
		0, 0, GetMaxX(), 39, 0,
		BTN_DRAW, NULL, "Electric", blueScheme);
	
	// title logo
#if !defined(__PIC24FJ128GA010__)
	PictCreate(ID_PICT_ELECTRIC, 80, 40, 
		239, 199, PICT_DRAW | PICT_DISABLED, 2,
		(void*) &electricsmall, blackScheme);
#endif
		
		
	// button in enabled or disabled state
	// get the data from the meter object
	xSemaphoreTake(METERSemaphore, portMAX_DELAY);
	electric_on = gMeter.electric_on;
	sprintf(qvgaBuff1, "%ld", gMeter.electric_units);
	sprintf(qvgaBuff2, "$%ld.%02ld", gMeter.electric_total / 100, 
		gMeter.electric_total % 100);
	xSemaphoreGive(METERSemaphore);

	if (electric_on == 1) {
		BtnCreate(ID_BUTTON_ELECTRIC_ON,
			0, 40, 79, 139, 20,
			BTN_DRAW | BTN_PRESSED, NULL, "ON", greenScheme);
		BtnCreate(ID_BUTTON_ELECTRIC_OFF,
			0, 140, 79, 239, 20,
			BTN_DRAW, NULL, "OFF", redScheme);
	} else {
		BtnCreate(ID_BUTTON_ELECTRIC_ON,
			0, 40, 79, 139, 20,
			BTN_DRAW, NULL, "ON", greenScheme);
		BtnCreate(ID_BUTTON_ELECTRIC_OFF,
			0, 140, 79, 239, 20,
			BTN_DRAW | BTN_PRESSED, NULL, "OFF", redScheme);
	}
	
	// buttons with total units and cost
	StCreate(ID_LABEL_UNITS, 240, 40, GetMaxX(), 59,
		ST_DRAW | ST_CENTER_ALIGN, "Units", blackScheme);
	BtnCreate(ID_ELECTRIC_SCREEN_TOTAL,
		240, 60, GetMaxX(), 139, 10,
		BTN_DRAW | BTN_DISABLED, NULL, qvgaBuff1, btnSchemeMedium);

	StCreate(ID_LABEL_COST, 240, 140, GetMaxX(), 159,
		ST_DRAW | ST_CENTER_ALIGN, "Cost", blackScheme);
	BtnCreate(ID_ELECTRIC_TOTAL_COST,
		240, 160, GetMaxX(), GetMaxY(), 10,
		BTN_DRAW | BTN_DISABLED, NULL, qvgaBuff2, btnSchemeMedium);
	
	// zero the graph index
	usage_xpos = 0xFFFF;
	usage_last_value = 0xFFFF;
	
}

/*********************************************************************
 * Function:        void DrawRTOSStack(XCHAR* sTitle, short ypos, 
 *                            short stack_used, short stack_size)
 *
 * PreCondition:    None
 *
 * Input:           sTitle, task name to be printed
 *					ypos, position on the display
 *					stack_used, current high water mark (bytes)
 *					stack_size, size of this stack (bytes)
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Draw an rtos bar
 *
 * Note:            
 ********************************************************************/
void DrawRTOSStack(XCHAR* sTitle, short ypos, short stack_used, short stack_size)
{
	short textHeight;
	short bar_length;
	char sBuff[30];

	// select the small font and calculate its height	
	SetFont((void*) &GOLSmallFont);
	textHeight = GetTextHeight((void*) &GOLSmallFont);
	
	// draw the bar title
	SetColor(WHITE);
	OutTextXY(58 - GetTextWidth(sTitle, (void*) &GOLSmallFont), ypos, sTitle);

	// calculate the length of the bar, for convenience we assume that
	// no task will have a stack bigger than 1200 bytes (on PIC24), the values
	// are scaled into the range 0-1200 and made to fit into a 200 pixel
	// wide display area. (Hence 2 /  in the calculations), for PIC32 the task
	// length is ~3000 bytes max
	bar_length = (2 * stack_size) / MAX_BAR_LENGTH;
	SetColor(GREEN);
	Bar(60, ypos + 1, 60 + bar_length, ypos + textHeight - 2);				
	
	bar_length = (2 * stack_used) / MAX_BAR_LENGTH;
	SetColor(RED);
	Bar(61, ypos + 2, 60 + bar_length, ypos + textHeight - 3);	
	
	// draw the usage summary at the end
	SetColor(WHITE);
	sprintf(sBuff, "%d/%d", stack_used, stack_size);
	OutTextXY(264, ypos, sBuff);
	
}


/*********************************************************************
 * Function:        void CreateRTOSScreen(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Create the RTOS summary screen
 *
 * Note:            
 ********************************************************************/
static const XCHAR ipStr[] = {'I','P',':',0};
static const XCHAR netmaskStr[] = {'M','A','S','K',':',0};
static const XCHAR gwStr[] = {'G','W',':',0};
void CreateRTOSScreen(void)
{
	short textHeight;
	short ypos;
	char sBuff[30];
	
	GOLFree();	
	SetColor(BLACK);
	ClearDevice();
	
	// display the title message
	BtnCreate(ID_WINDOW4,
		0, 0, GetMaxX(), 39, 0,
		BTN_DRAW, NULL, "RTOS Summary", blueScheme);
	
	// we will call the update function to perform drawing (saving code)
	UpdateRTOSScreen();

	// fill in the IP address information
	
	SetColor(WHITE);
	SetFont((void*) &GOLSmallFont);
	textHeight = GetTextHeight((void*) &GOLSmallFont);

	Line(0, 148, GetMaxX(), 148);
	ypos = 150;
	OutTextXY(58 - GetTextWidth((XCHAR*) ipStr, (void*) &GOLSmallFont), ypos, (XCHAR*) ipStr);
	sprintf(sBuff, "%u.%u.%u.%u",
			AppConfig.MyIPAddr.v[0], AppConfig.MyIPAddr.v[1],
			AppConfig.MyIPAddr.v[2], AppConfig.MyIPAddr.v[3]); 
	OutTextXY(60, ypos, sBuff);

	ypos += textHeight;
	OutTextXY(58 - GetTextWidth((XCHAR*) netmaskStr, (void*) &GOLSmallFont), ypos, (XCHAR*) netmaskStr);
	sprintf(sBuff, "%u.%u.%u.%u",
			AppConfig.MyMask.v[0], AppConfig.MyMask.v[1],
			AppConfig.MyMask.v[2], AppConfig.MyMask.v[3]); 
	OutTextXY(60, ypos, sBuff);
		
	ypos += textHeight;
	OutTextXY(58 - GetTextWidth((XCHAR*) gwStr, (void*) &GOLSmallFont), ypos, (XCHAR*) gwStr);
	sprintf(sBuff, "%u.%u.%u.%u",
			AppConfig.MyGateway.v[0], AppConfig.MyGateway.v[1],
			AppConfig.MyGateway.v[2], AppConfig.MyGateway.v[3]); 
	OutTextXY(60, ypos, sBuff);
}

/*********************************************************************
 * Function:        WORD msgMain(WORD objMsg, OBJ_HEADER* pObj)
 *
 * PreCondition:    None
 *
 * Input:           objMsg, message 
 *					pObj, object
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handle messages from the main menu
 *
 * Note:            
 ********************************************************************/
WORD msgMain(WORD objMsg, OBJ_HEADER* pObj)
{
	OBJ_HEADER* pSetPoint;
	SHORT dialVal;
	
	switch (GetObjID(pObj)) {
		case ID_SETPOINT_DIAL:
			dialVal = RdiaGetVal((ROUNDDIAL*) pObj);
			
			// update the setpoint display text box
			xSemaphoreTake(QVGASemaphore, portMAX_DELAY);
			pSetPoint = GOLFindObject(ID_SETPOINT_TEMPERATURE);
			sprintf(qvgaBuff4, "%d.%01dC", dialVal / 10, dialVal % 10);
			StDraw((STATICTEXT*) pSetPoint);
			xSemaphoreGive(QVGASemaphore);
			
			// update the meter object
			xSemaphoreTake(METERSemaphore, portMAX_DELAY);
			gMeter.setpoint = dialVal;
			xSemaphoreGive(METERSemaphore);
			
			// put limits on the rotation
			if ((objMsg == RD_MSG_CLOCKWISE) && (dialVal == 350))
				return 0;
			if ((objMsg == RD_MSG_CTR_CLOCKWISE) && (dialVal == 0))
				return 0;		
			break;
		case ID_BUTTON_GAS:
			if (objMsg == BTN_MSG_RELEASED) {
				screenState = CREATE_SCREEN_GAS;
			}
			return 1;
		case ID_BUTTON_ELECTRIC:
			if (objMsg == BTN_MSG_RELEASED) {
				screenState = CREATE_SCREEN_ELECTRICITY;
			}
			return 1;
		case ID_WINDOW1:
			if (objMsg == BTN_MSG_RELEASED) {
				screenState = CREATE_SCREEN_RTOS;
			}
			return 1;
		default:
			break;
	}	

	return 1;	
}

/*********************************************************************
 * Function:        WORD msgGas(WORD objMsg, OBJ_HEADER* pObj)
 *
 * PreCondition:    None
 *
 * Input:           objMsg, message 
 *					pObj, object
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handle messages from the gas screen
 *
 * Note:            
 ********************************************************************/
WORD msgGas(WORD objMsg, OBJ_HEADER* pObj)
{
	OBJ_HEADER* pOtherBtn;
	
	switch(GetObjID(pObj)) {
		case ID_BUTTON_GAS_ON:
			if (objMsg == BTN_MSG_PRESSED) {
				if (!GetState(pObj, BTN_PRESSED)) {
					xSemaphoreTake(QVGASemaphore, portMAX_DELAY);
					pOtherBtn = GOLFindObject(ID_BUTTON_GAS_OFF);
					ClrState(pOtherBtn, BTN_PRESSED);
					SetState(pOtherBtn, BTN_DRAW);
					SetState(pObj, BTN_PRESSED | BTN_DRAW);
					xSemaphoreGive(QVGASemaphore);
					// update the meter
					xSemaphoreTake(METERSemaphore, portMAX_DELAY);
					gMeter.gas_on = 1;
					xSemaphoreGive(METERSemaphore);
				}
			}
			return 0;
		case ID_BUTTON_GAS_OFF:
			if (objMsg == BTN_MSG_PRESSED) {
				if (!GetState(pObj, BTN_PRESSED)) {
					xSemaphoreTake(QVGASemaphore, portMAX_DELAY);
					pOtherBtn = GOLFindObject(ID_BUTTON_GAS_ON);
					ClrState(pOtherBtn, BTN_PRESSED);
					SetState(pOtherBtn, BTN_DRAW);
					SetState(pObj, BTN_PRESSED | BTN_DRAW);
					xSemaphoreGive(QVGASemaphore);
					// update the meter
					xSemaphoreTake(METERSemaphore, portMAX_DELAY);
					gMeter.gas_on = 0;
					xSemaphoreGive(METERSemaphore);
				}
			}
			return 0;
		case ID_WINDOW2:
			if (objMsg == BTN_MSG_RELEASED) {
				screenState = CREATE_SCREEN_MAIN;	
			}
			return 1;
		default:
			return 1;
	}	
}

/*********************************************************************
 * Function:        WORD msgElectric(WORD objMsg, OBJ_HEADER* pObj)
 *
 * PreCondition:    None
 *
 * Input:           objMsg, message 
 *					pObj, object
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handle messages from the Electric screen
 *
 * Note:            
 ********************************************************************/
WORD msgElectric(WORD objMsg, OBJ_HEADER* pObj)
{
	OBJ_HEADER* pOtherBtn;
	
	switch(GetObjID(pObj)) {
		case ID_BUTTON_ELECTRIC_ON:
			if (objMsg == BTN_MSG_PRESSED) {
				if (!GetState(pObj, BTN_PRESSED)) {
					xSemaphoreTake(QVGASemaphore, portMAX_DELAY);
					pOtherBtn = GOLFindObject(ID_BUTTON_ELECTRIC_OFF);
					ClrState(pOtherBtn, BTN_PRESSED);
					SetState(pOtherBtn, BTN_DRAW);
					SetState(pObj, BTN_PRESSED | BTN_DRAW);
					xSemaphoreGive(QVGASemaphore);
					// update the meter
					xSemaphoreTake(METERSemaphore, portMAX_DELAY);
					gMeter.electric_on = 1;
					xSemaphoreGive(METERSemaphore);
				}
			}
			return 0;
		case ID_BUTTON_ELECTRIC_OFF:
			if (objMsg == BTN_MSG_PRESSED) {
				if (!GetState(pObj, BTN_PRESSED)) {
					xSemaphoreTake(QVGASemaphore, portMAX_DELAY);
					pOtherBtn = GOLFindObject(ID_BUTTON_ELECTRIC_ON);
					ClrState(pOtherBtn, BTN_PRESSED);
					SetState(pOtherBtn, BTN_DRAW);
					SetState(pObj, BTN_PRESSED | BTN_DRAW);
					xSemaphoreGive(QVGASemaphore);
					// update the meter
					xSemaphoreTake(METERSemaphore, portMAX_DELAY);
					gMeter.electric_on = 0;
					xSemaphoreGive(METERSemaphore);
				}
			}
			return 0;
		case ID_WINDOW3:
			if (objMsg == BTN_MSG_RELEASED) {
				screenState = CREATE_SCREEN_MAIN;	
			}
			return 1;
		default:
			return 1;
	}	
}

/*********************************************************************
 * Function:        WORD msgRTOS(WORD objMsg, OBJ_HEADER* pObj)
 *
 * PreCondition:    None
 *
 * Input:           objMsg, message 
 *					pObj, object
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handle messages from the RTOS screen
 *
 * Note:            
 ********************************************************************/
WORD msgRTOS(WORD objMsg, OBJ_HEADER* pObj)
{
	switch(GetObjID(pObj)) {
		case ID_WINDOW4:
			if (objMsg == BTN_MSG_RELEASED) {
				screenState = CREATE_SCREEN_MAIN;	
			}
			return 1;
		default:
			return 1;
	}	
}

/*********************************************************************
 * Function:        void UpdateRTOSScreen(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Repaint the stack usage info
 *
 * Note:            
 ********************************************************************/
static const XCHAR uartStr[] = {'U','A','R','T',':',0};
static const XCHAR meterStr[] = {'M','E','T','E','R',':',0};
static const XCHAR miwiStr[] = {'M','I','W','I',':',0};
static const XCHAR touchStr[] = {'T','O','U','C','H',':',0};
static const XCHAR graphicsStr[] = {'G','R','A','P','H',':',0};
static const XCHAR tcpipStr[] = {'T','C','P','I','P',':',0};
void UpdateRTOSScreen(void)
{
	unsigned portBASE_TYPE hw;
	short textHeight;
	short ypos;
	
	// display the RTOS task usage bargraph
	textHeight = GetTextHeight((void*) &GOLSmallFont);
	// just erase the area of the bars and summary text
	SetColor(BLACK);
	Bar(60, 48, GetMaxX(), (6 * textHeight) + 48);
	
	// for each task display its stack usage as a bar graph and summary text
	ypos = 48;	
	hw = (STACK_SIZE_UART - uxTaskGetStackHighWaterMark(hUARTTask)) * sizeof(portSTACK_TYPE);
	DrawRTOSStack((XCHAR*) uartStr, ypos, hw, STACK_SIZE_UART * sizeof(portSTACK_TYPE));
	
	ypos += textHeight;
	hw = (STACK_SIZE_METER - uxTaskGetStackHighWaterMark(hMETERTask)) * sizeof(portSTACK_TYPE);
	DrawRTOSStack((XCHAR*) meterStr, ypos, hw, STACK_SIZE_METER * sizeof(portSTACK_TYPE));
				
	ypos += textHeight;
	hw = (STACK_SIZE_MIWI - uxTaskGetStackHighWaterMark(hMIWITask)) * sizeof(portSTACK_TYPE);
	DrawRTOSStack((XCHAR*) miwiStr, ypos, hw, STACK_SIZE_MIWI * sizeof(portSTACK_TYPE));

	ypos += textHeight;
	hw = (STACK_SIZE_TOUCHSCREEN - uxTaskGetStackHighWaterMark(hTOUCHTask)) * sizeof(portSTACK_TYPE);
	DrawRTOSStack((XCHAR*) touchStr, ypos, hw, STACK_SIZE_TOUCHSCREEN * sizeof(portSTACK_TYPE));

	ypos += textHeight;
	hw = (STACK_SIZE_GRAPHICS - uxTaskGetStackHighWaterMark(hGRAPHICSTask)) * sizeof(portSTACK_TYPE);
	DrawRTOSStack((XCHAR*) graphicsStr, ypos, hw, STACK_SIZE_GRAPHICS * sizeof(portSTACK_TYPE));

	ypos += textHeight;
	hw = (STACK_SIZE_TCPIP - uxTaskGetStackHighWaterMark(hTCPIPTask)) * sizeof(portSTACK_TYPE);
	DrawRTOSStack((XCHAR*) tcpipStr, ypos, hw, STACK_SIZE_TCPIP * sizeof(portSTACK_TYPE));	
}

/*********************************************************************
 * Function:        void UpdateUsageGraph(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Draw the usage graph at the bottom of the screen
 *
 * Note:            
 ********************************************************************/
void UpdateUsageGraph(void)
{
	WORD currentVal, ypos;
	static WORD old_x, old_y;
	
	// check if this is a valid display option
	if ((screenState != DISPLAY_SCREEN_ELECTRICITY) && 
		(screenState != DISPLAY_SCREEN_GAS)) {
		return;
	}
	
	// calculate how many units used in the last time interval (2 seconds)
	xSemaphoreTake(METERSemaphore, portMAX_DELAY);
	if (screenState == DISPLAY_SCREEN_ELECTRICITY)
		currentVal = gMeter.electric_units;
	else
		currentVal = gMeter.gas_units;
	xSemaphoreGive(METERSemaphore);
		
	// subtract the previous reading (if valid)
	if (usage_last_value != 0xFFFF) {
		ypos = currentVal - usage_last_value;
	} else {
		ypos = 0;
	}
	usage_last_value = currentVal;

	// update the graph
	if (usage_xpos == 0xFFFF) {
		// redraw the graph
		SetColor(BLACK);	
		Bar(80, 200, 239, GetMaxY());
		SetColor(WHITE);
		Line(92, 200, 92, GetMaxY() - 2);
		Line(90, GetMaxY() - 4, 229, GetMaxY() - 4);
		
		usage_xpos = 0;
	}
	
	SetColor(GREEN);
	// scale the ypos result into the displayed pixel range
	ypos *= 3;
	ypos = GetMaxY() - 4 - ypos;
	
	// only draw if not the very first point of the line
	if (usage_xpos != 0)
		Line(old_x + 93, old_y, usage_xpos + 93, ypos);
	
	// record the previous points
	old_x = usage_xpos;
	old_y = ypos;
	
	usage_xpos += 4;
	
	// redraw graph next time if required
	if ((usage_xpos + 83) >= 220) {
		usage_xpos = 0xFFFF;
	}
}

