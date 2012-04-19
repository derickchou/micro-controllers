/*****************************************************************************
 * Microchip RTOS and Stacks Demo
 *****************************************************************************
 * FileName:        MeterHTTPApp.c
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
 * 1.0      D.Wenn               09/30/08    Initial version   
 *											 File provides handling for
 *											 all dynamic variables
 *											 and web related events
 *****************************************************************************/
 
#include "TCPIP Stack/TCPIP.h"
#include "FreeRTOS.h"
#include "Task.h"
#include "semphr.h"
#include "homeMeter.h"

///////////////////////////////////////////////////////////////////
// forward declarations and local functions 

#if defined(HTTP_USE_POST)
	#if defined(STACK_USE_HTTP_APP_RECONFIG)
		extern APP_CONFIG AppConfig;
		static HTTP_IO_RESULT HTTPPostConfig(void);
	#endif	
#endif

static HTTP_IO_RESULT HTTPPostBilling(void);

///////////////////////////////////////////////////////////////////
// Check requested files for source path, if in the protect
// directory then require password entry
#if defined(HTTP_USE_AUTHENTICATION)
BYTE HTTPNeedsAuth(BYTE* cFile)
{
	// if the filename begins with the folder protect the require auth
	if (memcmppgm2ram(cFile, (ROM void*) "protect", 7) == 0)
		return 0x00;
		
	#if defined(HTTP_MPFS_UPLOAD_REQUIRES_AUTH)
	if (memcmppgm2ram(cFile, (ROM void*) "mpfsupload", 10) == 0)
		return 0x00;
	#endif
	
	return 0x80; // no authentication required	
}
#endif

/*****************************************************************************
  Function:
	BYTE HTTPCheckAuth(BYTE* cUser, BYTE* cPass)
	
  Internal:
  	See documentation in the TCP/IP Stack API or HTTP2.h for details.
  ***************************************************************************/
#if defined(HTTP_USE_AUTHENTICATION)
BYTE HTTPCheckAuth(BYTE* cUser, BYTE* cPass)
{
	if ((strcmppgm2ram((char *)cUser,(ROM char *)"admin") == 0) && 
		(strcmppgm2ram((char *)cPass, (ROM char *)"microchip") == 0))
		return 0x80;		// We accept this combination
	
	return 0x00;			// Provided user/pass is invalid
}
#endif

// helper function
void HTTPPrintIP(IP_ADDR ip)
{
	BYTE digits[4];
	BYTE i;
	
	for(i =0; i < 4u; i++) {
		if (i)
			TCPPut(sktHTTP, '.');
		uitoa(ip.v[i], digits);
		TCPPutString(sktHTTP, digits);
	}	
}
 
/*********************************************************************
 * Function:        HTTP_IO_RESULT HTTPExecutGet(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          Success
 *
 * Side Effects:    None
 *
 * Overview:        Process GET Requests
 *
 * Note:            
 ********************************************************************/
HTTP_IO_RESULT HTTPExecuteGet(void)
{
	BYTE* ptr;
	BYTE filename[20];
	
	// load the specified filename
	MPFSGetFilename(curHTTP.file, filename, sizeof(filename));

	// remotely update electricity or gas
	if (!memcmppgm2ram(filename, "switches.cgi", 12)) {
		// check for electricity being toggled
		ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE*) "electric");
		if (ptr != NULL) {
			// obtain access to the meter object
			if (xSemaphoreTake(METERSemaphore, 50 / portTICK_RATE_MS) == pdTRUE) {
				gMeter.electric_on = !gMeter.electric_on;
				xSemaphoreGive(METERSemaphore);
			}
		}
		
		// do the same for gas
		ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE*) "gas");
		if (ptr != NULL) {
			// obtain access to the meter object
			if (xSemaphoreTake(METERSemaphore, 50 / portTICK_RATE_MS) == pdTRUE) {
				gMeter.gas_on = !gMeter.gas_on;
				xSemaphoreGive(METERSemaphore);
			}
		}		
	}

		
	return HTTP_IO_DONE;	
}

///////////////////////////////////////////////////////////////////
// POST form handlers
#if defined(HTTP_USE_POST)

/*********************************************************************
 * Function:        HTTP_IO_RESULT HTTPExecutPost(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          Success
 *
 * Side Effects:    None
 *
 * Overview:        Process POST Messages
 *
 * Note:            
 ********************************************************************/
HTTP_IO_RESULT HTTPExecutePost(void)
{
	BYTE filename[20];
	
	MPFSGetFilename(curHTTP.file, filename, sizeof(filename));
	
	#if defined(STACK_USE_HTTP_APP_RECONFIG)
		if (!memcmppgm2ram(filename,"protect/config.htm", 18))
			return HTTPPostConfig();
	#endif	
	
	if (!memcmppgm2ram(filename, "protect/billing.htm", 19))
		return HTTPPostBilling();
		 
	return HTTP_IO_DONE;
}

#endif // HTTP_USE_POST

/*****************************************************************************
  Function:
	static HTTP_IO_RESULT HTTPPostBilling(void)

  Summary:
	Processes the configuration form on config/billing.htm

  Description:
	Accepts billing parameters from the form

  Precondition:
	None

  Parameters:
	None

  Return Values:
  	HTTP_IO_DONE - all parameters have been processed
  	HTTP_IO_WAITING - the function is pausing to continue later
  	HTTP_IO_NEED_DATA - data needed by this function has not yet arrived
  ***************************************************************************/
HTTP_IO_RESULT HTTPPostBilling(void)
{
	METER_MSG msg;
	
	#define SM_BILLING_START		(0u)
	#define SM_BILLING_READ_NAME	(1u)
	#define SM_BILLING_READ_VALUE	(2u)
	#define SM_BILLING_SUCCESS		(3u)

	switch (curHTTP.smPost) {
		case SM_BILLING_START:
			// perform any initialisation
			curHTTP.smPost = SM_BILLING_READ_NAME;
		
		case SM_BILLING_READ_NAME:	
			// if no more data finish
			if(curHTTP.byteCount == 0)
			{
				curHTTP.smPost = SM_BILLING_SUCCESS;
				break;
			}
		
			// Read a name
			if (HTTPReadPostName(curHTTP.data, 16) == HTTP_READ_INCOMPLETE)
				return HTTP_IO_NEED_DATA;
				
			// Move to reading a value, but no break
			curHTTP.smPost = SM_BILLING_READ_VALUE;
		
		case SM_BILLING_READ_VALUE:
			// read the value
			if (HTTPReadPostValue(curHTTP.data + 16, 8) == HTTP_READ_INCOMPLETE)
				return HTTP_IO_NEED_DATA;
				
			// default action is to read the next name
			curHTTP.smPost = SM_BILLING_READ_NAME;
			
			// parse the values, in practice this needs to be more robust
			if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "electric")) {
				// read the new electricity unit cost
				msg.data.wVal[0] = atoi((char*) (curHTTP.data + 16));
				msg.cmd = MSG_METER_UPDATE_ELECTRIC_COST;
				xQueueSendToBack(hMETERQueue, &msg, portMAX_DELAY);
			} else if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "gas")) {
				// read the new gas unit cost
				msg.data.wVal[0] = atoi((char*) (curHTTP.data + 16));
				msg.cmd = MSG_METER_UPDATE_GAS_COST;
				xQueueSendToBack(hMETERQueue, &msg, portMAX_DELAY);			
			}
			
			break;
			
		case SM_BILLING_SUCCESS:
			return HTTP_IO_DONE;
			break;
		default:
			break;
	}

	return HTTP_IO_WAITING;	
}

/*****************************************************************************
  Function:
	static HTTP_IO_RESULT HTTPPostConfig(void)

  Summary:
	Processes the configuration form on config/index.htm

  Description:
	Accepts configuration parameters from the form, saves them to a
	temporary location in RAM, then eventually saves the data to EEPROM or
	external Flash.
	
	When complete, this function redirects to config/reboot.htm, which will
	then issue an AJAX call to reboot the device.

	This function creates a shadow copy of the AppConfig structure in 
	the upper portion of curHTTP.data, and then overwrites incoming data 
	there as it arrives.  For each name/value pair, the name is first 
	read to curHTTP.data[0:7].  Next, the value is read to 
	curHTTP.data[8:31].  The name is matched against an expected list, 
	and then the value is parsed and stored in the appropriate location 
	of the shadow copy of AppConfig.  Once all data has been read, the new
	AppConfig is saved back to EEPROM and the browser is redirected to 
	reboot.htm.  That file includes an AJAX call to reboot.cgi, which 
	performs the actual reboot of the machine.
	
	If an IP address cannot be parsed, the browser redirects to
	config_error.htm and does not save anything so as to prevent errors.

  Precondition:
	None

  Parameters:
	None

  Return Values:
  	HTTP_IO_DONE - all parameters have been processed
  	HTTP_IO_WAITING - the function is pausing to continue later
  	HTTP_IO_NEED_DATA - data needed by this function has not yet arrived
  ***************************************************************************/
#if defined(STACK_USE_HTTP_APP_RECONFIG)
static HTTP_IO_RESULT HTTPPostConfig(void)
{
	APP_CONFIG *app;
	BYTE *ptr;
	WORD len;
	
	#define SM_CFG_START		(0u)
	#define SM_CFG_READ_NAME	(1u)
	#define SM_CFG_READ_VALUE	(2u)
	#define SM_CFG_SUCCESS		(3u)
	#define SM_CFG_FAILURE		(4u)
	
	// Set app config pointer to use data array
	app = (void*)(&curHTTP.data[32]);
	
	switch(curHTTP.smPost)
	{
		case SM_CFG_START:
			// Use current config as defaults
			memcpy((void*)app, (void*)&AppConfig, sizeof(AppConfig));
			app->Flags.bIsDHCPEnabled = 0;
			
			// Move to next state, but no break
			curHTTP.smPost = SM_CFG_READ_NAME;
			
		case SM_CFG_READ_NAME:
			// If all parameters have been read, end
			if(curHTTP.byteCount == 0)
			{
				curHTTP.smPost = SM_CFG_SUCCESS;
				break;
			}
		
			// Read a name
			if(HTTPReadPostName(curHTTP.data, 8) == HTTP_READ_INCOMPLETE)
				return HTTP_IO_NEED_DATA;
				
			// Move to reading a value, but no break
			curHTTP.smPost = SM_CFG_READ_VALUE;
			
		case SM_CFG_READ_VALUE:
			// Read a value
			if(HTTPReadPostValue(curHTTP.data + 8, 24) == HTTP_READ_INCOMPLETE)
				return HTTP_IO_NEED_DATA;
				
			// Default action after this is to read the name, unless there's an error
			curHTTP.smPost = SM_CFG_READ_NAME;
				
			// Parse the value that was read
			if(!strcmppgm2ram((char*)curHTTP.data, (ROM char*)"ip"))
			{// Read new static IP Address
				if(StringToIPAddress(curHTTP.data+8, &(app->MyIPAddr)))
					memcpy((void*)&(app->DefaultIPAddr), (void*)&(app->MyIPAddr), sizeof(IP_ADDR));
				else
					curHTTP.smPost = SM_CFG_FAILURE;
			}
			else if(!strcmppgm2ram((char*)curHTTP.data, (ROM char*)"gw"))
			{// Read new gateway address
				if(!StringToIPAddress(curHTTP.data+8, &(app->MyGateway)))
					curHTTP.smPost = SM_CFG_FAILURE;
			}
			else if(!strcmppgm2ram((char*)curHTTP.data, (ROM char*)"subnet"))
			{// Read new static subnet
				if(StringToIPAddress(curHTTP.data+8, &(app->MyMask)))
					memcpy((void*)&(app->DefaultMask), (void*)&(app->MyMask), sizeof(IP_ADDR));
				else
					curHTTP.smPost = SM_CFG_FAILURE;
			}
			else if(!strcmppgm2ram((char*)curHTTP.data, (ROM char*)"dns1"))
			{// Read new primary DNS server
				if(!StringToIPAddress(curHTTP.data+8, &(app->PrimaryDNSServer)))
					curHTTP.smPost = SM_CFG_FAILURE;
			}
			else if(!strcmppgm2ram((char*)curHTTP.data, (ROM char*)"dns2"))
			{// Read new secondary DNS server
				if(!StringToIPAddress(curHTTP.data+8, &(app->SecondaryDNSServer)))
					curHTTP.smPost = SM_CFG_FAILURE;
			}
			else if(!strcmppgm2ram((char*)curHTTP.data, (ROM char*)"mac"))
			{// Read new MAC address
				WORD_VAL w;
				BYTE i;
	
				ptr = curHTTP.data+8;
	
				for(i = 0; i < 12; i++)
				{// Read the MAC address
					
					// Skip non-hex bytes
					while( *ptr != 0x00 && !(*ptr >= '0' && *ptr <= '9') && !(*ptr >= 'A' && *ptr <= 'F') && !(*ptr >= 'a' && *ptr <= 'f') )
						ptr++;
	
					// MAC string is over, so zeroize the rest
					if(*ptr == 0x00)
					{
						for(; i < 12; i++)
							curHTTP.data[i] = '0';
						break;
					}
					
					// Save the MAC byte
					curHTTP.data[i] = *ptr++;
				}
				
				// Read MAC Address, one byte at a time
				for(i = 0; i < 6; i++)
				{				
					w.v[1] = curHTTP.data[i*2];
					w.v[0] = curHTTP.data[i*2+1];
					app->MyMACAddr.v[i] = hexatob(w);
				}
			}
			else if(!strcmppgm2ram((char*)curHTTP.data, (ROM char*)"host"))
			{// Read new hostname
				for(len = strlen((char*)&curHTTP.data[8]); len < 15; len++)
					curHTTP.data[8+len] = ' ';
				memcpy((void*)app->NetBIOSName, (void*)curHTTP.data+8, 15);
				strupr((char*)app->NetBIOSName);
			}
			else if(!strcmppgm2ram((char*)curHTTP.data, (ROM char*)"dhcpen"))
			{// Read new DHCP Enabled flag
				if(curHTTP.data[8] == '1')
					app->Flags.bIsDHCPEnabled = 1;
				else
					app->Flags.bIsDHCPEnabled = 0;
			}

			break;
			
		case SM_CFG_SUCCESS:
			// Save new settings and force a reboot
			
			// If DCHP, then disallow editing of DefaultIP and DefaultMask
			if(app->Flags.bIsDHCPEnabled)
			{
				// If DHCP is enabled, then reset the default IP and mask
				app->DefaultIPAddr.v[0] = MY_DEFAULT_IP_ADDR_BYTE1;
				app->DefaultIPAddr.v[1] = MY_DEFAULT_IP_ADDR_BYTE2;
				app->DefaultIPAddr.v[2] = MY_DEFAULT_IP_ADDR_BYTE3;
				app->DefaultIPAddr.v[3] = MY_DEFAULT_IP_ADDR_BYTE4;
				app->DefaultMask.v[0] = MY_DEFAULT_MASK_BYTE1;
				app->DefaultMask.v[1] = MY_DEFAULT_MASK_BYTE2;
				app->DefaultMask.v[2] = MY_DEFAULT_MASK_BYTE3;
				app->DefaultMask.v[3] = MY_DEFAULT_MASK_BYTE4;
			}
			ptr = (BYTE*)app;
			#if defined(MPFS_USE_EEPROM)
		    XEEBeginWrite(0x0000);
		    XEEWrite(0x60);
		    for (len = 0; len < sizeof(AppConfig); len++ )
		        XEEWrite(*ptr++);
		    XEEEndWrite();
	        while(XEEIsBusy());
	        #elif defined(MPFS_USE_SPI_FLASH)
	        // write the data to the FLASH
	        #if (GRAPHICS_PICTAIL_VERSION == 3)
		        SST25SectorErase(MPFS_RESERVE_BASE);
				SST25WriteByte(MPFS_RESERVE_BASE, 0x60);
				SST25WriteArray(MPFS_RESERVE_BASE + 1, ptr, sizeof(AppConfig));
			#else
		        SST39SectorErase(MPFS_RESERVE_BASE);
				SST39WriteByte(MPFS_RESERVE_BASE, 0x60);
				SST39WriteArray(MPFS_RESERVE_BASE + 1, ptr, sizeof(AppConfig));
			#endif
	        #endif
			
			// Set the board to reboot to the new address
			strcpypgm2ram((char*)curHTTP.data, (ROM char*)"/protect/reboot.htm?");
			memcpy((void*)(curHTTP.data+20), (void*)app->NetBIOSName, 16);
			ptr = curHTTP.data;
			while(*ptr != ' ' && *ptr != '\0')
				ptr++;
			*ptr = '\0';
			curHTTP.httpStatus = HTTP_REDIRECT;	
			
			return HTTP_IO_DONE;
			
		case SM_CFG_FAILURE:
			// An error occurred parsing the IP, so don't save 
			// anything to prevent network errors.
			
			strcpypgm2ram((char*)curHTTP.data, (ROM char*)"/protect/config_error.htm");
			curHTTP.httpStatus = HTTP_REDIRECT;		

			return HTTP_IO_DONE;			
	}

	return HTTP_IO_WAITING;		// Assume we're waiting to process more data
}
#endif	// #if defined(STACK_USE_HTTP_APP_RECONFIG)

//////////////////////////////////////////////////////////////////
// These are callback handlers for all of the dynamic variables
// on the web pages, code will need adding and removing from
// here to reflect changes in the web pages
 
void HTTPPrint_config_hostname(void)
{
	TCPPutString(sktHTTP, AppConfig.NetBIOSName);
	return;
}

void HTTPPrint_config_dhcpchecked(void)
{
	if (AppConfig.Flags.bIsDHCPEnabled)
		TCPPutROMString(sktHTTP, (ROM BYTE*)"checked");
	return;	
}

void HTTPPrint_config_ip(void)
{
	HTTPPrintIP(AppConfig.MyIPAddr);	
	return;
}

void HTTPPrint_config_gw(void)
{
	HTTPPrintIP(AppConfig.MyGateway);	
	return;
}

void HTTPPrint_config_subnet(void)
{
	HTTPPrintIP(AppConfig.MyMask);	
	return;
}

void HTTPPrint_config_dns1(void)
{
	HTTPPrintIP(AppConfig.PrimaryDNSServer);	
	return;
}

void HTTPPrint_config_dns2(void)
{
	HTTPPrintIP(AppConfig.SecondaryDNSServer);	
	return;
}

void HTTPPrint_config_mac(void)
{
	BYTE i;
	
	// check for space
	if (TCPIsPutReady(sktHTTP) < 18u) {
		curHTTP.callbackPos = 0x01;
		return;
	}
	
	// write each byte
	for(i = 0; i < 6u; i++) {
		if (i)
			TCPPut(sktHTTP, ':');
		TCPPut(sktHTTP, btohexa_high(AppConfig.MyMACAddr.v[i]));
		TCPPut(sktHTTP, btohexa_low(AppConfig.MyMACAddr.v[i]));
	}
	
	// indicate completion
	curHTTP.callbackPos = 0x00;
	return;
}

void HTTPPrint_builddate(void)
{
	curHTTP.callbackPos = 0x01;
	if (TCPIsPutReady(sktHTTP) < strlenpgm((ROM char*)__DATE__" "__TIME__))
		return;
		
	curHTTP.callbackPos = 0x00;
	TCPPutROMString(sktHTTP, (ROM void*)__DATE__" "__TIME__);	
}

void HTTPPrint_version(void)
{
	TCPPutROMString(sktHTTP, (ROM void*)" "VERSION" ");	
}

void HTTPPrint_reboot(void)
{
	// This is not so much a print function, but causes the board to reboot
	// when the configuration is changed.  If called via an AJAX call, this
	// will gracefully reset the board and bring it back online immediately
	Reset();
}

void HTTPPrint_electric_units(void)
{
	BYTE sBuff[20];
	
	if (xSemaphoreTake(METERSemaphore, 50 / portTICK_RATE_MS) == pdTRUE) {
		sprintf((char*) sBuff, "%ld", gMeter.electric_units);
		TCPPutString(sktHTTP, sBuff);
		xSemaphoreGive(METERSemaphore);	
	} else {
		TCPPutROMString(sktHTTP, (ROM BYTE*) "?");
	}
	return;		
}

void HTTPPrint_electric_total(void)
{
	BYTE sBuff[20];
	
	if (xSemaphoreTake(METERSemaphore, 50 / portTICK_RATE_MS) == pdTRUE) {
		sprintf((char*) sBuff, "$%ld.%02ld", gMeter.electric_total / 100,
			gMeter.electric_total % 100);
		TCPPutString(sktHTTP, sBuff);	
		xSemaphoreGive(METERSemaphore);	
	} else {
		TCPPutROMString(sktHTTP, (ROM BYTE*) "?");
	}
	return;		
}

void HTTPPrint_electric_cost(void)
{
	BYTE sBuff[20];
	
	if (xSemaphoreTake(METERSemaphore, 50 / portTICK_RATE_MS) == pdTRUE) {
		sprintf((char*) sBuff, "%ld", gMeter.electric_cost);
		TCPPutString(sktHTTP, sBuff);	
		xSemaphoreGive(METERSemaphore);	
	} else {
		TCPPutROMString(sktHTTP, (ROM BYTE*) "?");
	}
	return;		
}

void HTTPPrint_electric_onoff(void)
{
	if (xSemaphoreTake(METERSemaphore, 50 / portTICK_RATE_MS) == pdTRUE) {
		if (gMeter.electric_on != 0) 
			TCPPutROMString(sktHTTP, (ROM BYTE*)"true");
		else
			TCPPutROMString(sktHTTP, (ROM BYTE*)"false");
		xSemaphoreGive(METERSemaphore);	
	} else {
		TCPPutROMString(sktHTTP, (ROM BYTE*) "?");
	}
	return;		
}

void HTTPPrint_gas_units(void)
{
	BYTE sBuff[20];
	
	if (xSemaphoreTake(METERSemaphore, 50 / portTICK_RATE_MS) == pdTRUE) {
		sprintf((char*) sBuff, "%ld", gMeter.gas_units);
		TCPPutString(sktHTTP, sBuff);	
		xSemaphoreGive(METERSemaphore);	
	} else {
		TCPPutROMString(sktHTTP, (ROM BYTE*) "?");
	}
	return;		
}

void HTTPPrint_gas_total(void)
{
	BYTE sBuff[20];
	
	if (xSemaphoreTake(METERSemaphore, 50 / portTICK_RATE_MS) == pdTRUE) {
		sprintf((char*) sBuff, "$%ld.%02ld", gMeter.gas_total / 100,
			gMeter.gas_total % 100);
		TCPPutString(sktHTTP, sBuff);	
		xSemaphoreGive(METERSemaphore);	
	} else {
		TCPPutROMString(sktHTTP, (ROM BYTE*) "?");
	}
	return;		
}

void HTTPPrint_gas_cost(void)
{
	BYTE sBuff[20];
	
	if (xSemaphoreTake(METERSemaphore, 50 / portTICK_RATE_MS) == pdTRUE) {
		sprintf((char*) sBuff, "%ld", gMeter.gas_cost);
		TCPPutString(sktHTTP, sBuff);	
		xSemaphoreGive(METERSemaphore);	
	} else {
		TCPPutROMString(sktHTTP, (ROM BYTE*) "?");
	}
	return;		
}

void HTTPPrint_gas_onoff(void)
{
	if (xSemaphoreTake(METERSemaphore, 50 / portTICK_RATE_MS) == pdTRUE) {
		if (gMeter.gas_on != 0) 
			TCPPutROMString(sktHTTP, (ROM BYTE*)"true");
		else
			TCPPutROMString(sktHTTP, (ROM BYTE*)"false");
		xSemaphoreGive(METERSemaphore);	
	} else {
		TCPPutROMString(sktHTTP, (ROM BYTE*) "?");
	}
	return;		
}

void HTTPPrint_temperature(void)
{
	BYTE sBuff[20];
	
	if (xSemaphoreTake(METERSemaphore, 50 / portTICK_RATE_MS) == pdTRUE) {
		sprintf((char*) sBuff, "%d.%01dC", gMeter.temperature / 10,
			gMeter.temperature % 10);
		TCPPutString(sktHTTP, sBuff);	
		xSemaphoreGive(METERSemaphore);	
	} else {
		TCPPutROMString(sktHTTP, (ROM BYTE*) "?");
	}
	return;		
}

void HTTPPrint_setpoint(void)
{
	BYTE sBuff[20];
	
	if (xSemaphoreTake(METERSemaphore, 50 / portTICK_RATE_MS) == pdTRUE) {
		sprintf((char*) sBuff, "%d.%01dC", gMeter.setpoint / 10,
			gMeter.setpoint % 10);
		TCPPutString(sktHTTP, sBuff);	
		xSemaphoreGive(METERSemaphore);	
	} else {
		TCPPutROMString(sktHTTP, (ROM BYTE*) "?");
	}
	return;		
}

void HTTPPrint_pot(void)
{
	BYTE tempString[8];	
	sprintf((char*) tempString, "?");
	TCPPutString(sktHTTP, tempString);
	return;		
}

void HTTPPrint_electric(void)
{
	TCPPutROMString(sktHTTP, (ROM BYTE*)"Unused");
	return;		
}
