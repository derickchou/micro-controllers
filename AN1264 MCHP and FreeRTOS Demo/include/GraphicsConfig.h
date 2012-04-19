/*********************************************************************
 * Module for Microchip Graphics Library
 * This file contains compile time options for the Graphics Library. 
 *********************************************************************
 * FileName:        GraphicsConfig.h
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC24/PIC30/PIC32
 * Compiler:        C30 V3.00/C32
 * Company:         Microchip Technology, Inc.
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
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Anton Alkhimenok		10/28/2007
 * Darren Wenn			09/22/2008	Configuration for the MCHP and
 *									FreeRTOS stacks demo
 * Darren Wenn			06/01/09    Upgraded to Graphics V1.65 
 ********************************************************************/

#ifndef _GRAPHICSCONFIG_H
#define _GRAPHICSCONFIG_H

//////////////////// COMPILE OPTIONS AND DEFAULTS ////////////////////

/*********************************************************************
* Overview:
*   Graphics PICtail Plus Board Version setting. The value assigned 
*	to this macro determines the version of the PICtail Plus Board. 
*	- 1 - Uses Graphics PICtail Plus Board Version 1 (not supported)   
*	- 2 - Uses Graphics PICtail Plus Board Version 2 
*   - 3 - Uses Graphics PICtail Plus Board Version 3   
*                                        
********************************************************************/
#define GRAPHICS_PICTAIL_VERSION   3

#if (GRAPHICS_PICTAIL_VERSION == 1)
#error Graphics Controller is not supported
#elif (GRAPHICS_PICTAIL_VERSION == 2)

#define DISPLAY_CONTROLLER 				LGDP4531
#define DISP_HOR_RESOLUTION				240
#define DISP_VER_RESOLUTION				320
#define COLOR_DEPTH						16
#define DISP_ORIENTATION				90

#elif (GRAPHICS_PICTAIL_VERSION == 3)
/*********************************************************************
* Overview: Defines color depth. 
********************************************************************/
#define COLOR_DEPTH						16

/*********************************************************************
* Overview: Display controller selection.
*                                          
********************************************************************/
#define DISPLAY_CONTROLLER				SSD1926

/*********************************************************************
* Overview: Display panel selection.
*                                          
********************************************************************/
#define DISPLAY_PANEL                   TFT_G240320LTSW_118W_E

#if (DISPLAY_PANEL==TFT_G240320LTSW_118W_E)
/*********************************************************************
* Overview: Horizontal and vertical display resolution
*                  (from the glass datasheet).
*********************************************************************/
#define DISP_HOR_RESOLUTION				240
#define DISP_VER_RESOLUTION				320
/*********************************************************************
* Overview: Image orientation (can be 0, 90, 180, 270 degrees).
*********************************************************************/
#define DISP_ORIENTATION				90
/*********************************************************************
* Overview: Panel Data Width (can be 18 or 24 bits).
*********************************************************************/
#define DISP_DATA_WIDTH                 18
/*********************************************************************
* Overview: LSHIFT Polarity Swap
* If defined LSHIFT is a falling trigger
*********************************************************************/
#define DISP_INV_LSHIFT
/*********************************************************************
* Overview: Horizontal synchronization timing in pixels
*                  (from the glass datasheet).
*********************************************************************/
#define DISP_HOR_PULSE_WIDTH		    25
#define DISP_HOR_BACK_PORCH				4
#define DISP_HOR_FRONT_PORCH			0
/*********************************************************************
* Overview: Vertical synchronization timing in lines
*                  (from the glass datasheet).
*********************************************************************/
#define DISP_VER_PULSE_WIDTH		    4
#define DISP_VER_BACK_PORCH				0
#define DISP_VER_FRONT_PORCH			0

#elif (DISPLAY_PANEL==TFT_G320240DTSW_69W_TP_E)

/*********************************************************************
* Overview: Horizontal and vertical display resolution
*                  (from the glass datasheet).
*********************************************************************/
#define DISP_HOR_RESOLUTION				320
#define DISP_VER_RESOLUTION				240
/*********************************************************************
* Overview: Image orientation (can be 0, 90, 180, 270 degrees).
*********************************************************************/
#define DISP_ORIENTATION				0
/*********************************************************************
* Overview: Panel Data Width (can be 18 or 24 bits).
*********************************************************************/
#define DISP_DATA_WIDTH                 18
/*********************************************************************
* Overview: LSHIFT Polarity Swap
* If defined LSHIFT is a falling trigger
*********************************************************************/
//#define DISP_INV_LSHIFT
/*********************************************************************
* Overview: Horizontal synchronization timing in pixels
*                  (from the glass datasheet).
*********************************************************************/
#define DISP_HOR_PULSE_WIDTH		    25
#define DISP_HOR_BACK_PORCH				8
#define DISP_HOR_FRONT_PORCH			8
/*********************************************************************
* Overview: Vertical synchronization timing in lines
*                  (from the glass datasheet).
*********************************************************************/
#define DISP_VER_PULSE_WIDTH		    8
#define DISP_VER_BACK_PORCH				7
#define DISP_VER_FRONT_PORCH			5

#endif // (DISPLAY_PANEL ==...

#else
#error Graphics controller is not defined
#endif

/*********************************************************************
* Overview: Blocking and Non-Blocking configuration selection. To
*			enable non-blocking configuration USE_NONBLOCKING_CONFIG
*			must be defined. If this is not defined, blocking 
*			configuration is assumed.
*
********************************************************************/
//#define USE_NONBLOCKING_CONFIG // Comment this line to use blocking configuration


/*********************************************************************
* Overview: Keyboard control on some objects can be used by enabling
*			the GOL Focus (USE_FOCUS)support. 
*
*********************************************************************/
//#define USE_FOCUS

/*********************************************************************
* Overview: Input devices used defines the messages that Objects will
*			process. The following definitions indicate the usage of 
*			the different input device:
*			- USE_TOUCHSCREEN - enables the touch screen support.
*			- USE_KEYBOARD - enables the key board support.
*
*********************************************************************/
#define USE_TOUCHSCREEN			// Enable touch screen support.
#define USE_KEYBOARD			// Enable key board support.

/*********************************************************************
* Overview: To save program memory, unused Widgets or Objects can be 
*			removed at compile time. 
*
*********************************************************************/
#define USE_GOL                 // Enable Graphics Object Layer.
#define USE_BUTTON				// Enable Button Object.
#define USE_WINDOW				// Enable Window Object.
#define USE_CHECKBOX			// Enable Checkbox Object.
#define USE_RADIOBUTTON			// Enable Radio Button Object.
//#define USE_EDITBOX				// Enable Edit Box Object.
//#define USE_LISTBOX				// Enable List Box Object.
//#define USE_SLIDER 				// Enable Slider or Scroll Bar Object.
//#define USE_PROGRESSBAR			// Enable Progress Bar Object.
#define USE_STATICTEXT			// Enable Static Text Object.
#define USE_PICTURE				// Enable Picture Object.
//#define USE_GROUPBOX			// Enable Group Box Object.
#define USE_ROUNDDIAL			// Enable Dial Object.
//#define USE_METER				// Enable Meter Object.
//#define USE_CUSTOM				// Enable Custom Control Object (an example to create customized Object).

/*********************************************************************
* Overview: To enable support for unicode fonts, USE_MULTIBYTECHAR  
*			must be defined. This changes XCHAR definition. See XCHAR 
*			for details.
*
*********************************************************************/
//#define USE_MULTIBYTECHAR

/*********************************************************************
* Overview: Font data can be placed in two locations. One is in 
*			FLASH memory and the other is from external memory.
*			Definining one or both enables the support for fonts located
*			in internal flash and external memory. 
*	- USE_FONT_FLASH - Font in internal flash memory support.	
*	- USE_FONT_EXTERNAL - Font in external memory support.	
*
*********************************************************************/
#define USE_FONT_FLASH 			// Support for fonts located in internal flash
//#define USE_FONT_EXTERNAL		// Support for fonts located in external memory

/*********************************************************************
* Overview: Similar to Font data bitmaps can also be placed in 
*			two locations. One is in FLASH memory and the other is 
*			from external memory.
*			Definining one or both enables the support for bitmaps located
*			in internal flash and external memory. 
*	- USE_BITMAP_FLASH - Font in internal flash memory support.	
*	- USE_BITMAP_EXTERNAL - Font in external memory support.	
*
*********************************************************************/
#define USE_BITMAP_FLASH		// Support for bitmaps located in internal flash
//#define USE_BITMAP_EXTERNAL		// Support for bitmaps located in external memory

#endif // _GRAPHICSCONFIG_H
