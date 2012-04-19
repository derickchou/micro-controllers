/********************************************************************
* FileName:		main.c
* Dependencies: none   
* Processor:	PIC18, PIC24F, PIC24H, dsPIC30, dsPIC33
*               tested with 18F4620, dsPIC33FJ256GP710	
* Hardware:		PICDEM Z, Explorer 16, PIC18 Explorer
* Complier:     Microchip C18 v3.30 or higher
*				Microchip C30 v2.03 or higher
*               Microchip C32 v1.02 or higher	
* Company:		Microchip Technology, Inc.
*
* Copyright and Disclaimer Notice for P2P Software:
*
* Copyright © 2007-2009 Microchip Technology Inc.  All rights reserved.
*
* Microchip licenses to you the right to use, modify, copy and distribute 
* Software only when embedded on a Microchip microcontroller or digital 
* signal controller and used with a Microchip radio frequency transceiver, 
* which are integrated into your product or third party product (pursuant 
* to the terms in the accompanying license agreement).  
*
* You should refer to the license agreement accompanying this Software for 
* additional information regarding your rights and obligations.
*
* SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT WARRANTY OF ANY 
* KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY 
* WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A 
* PARTICULAR PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE 
* LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, 
* CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY 
* DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO 
* ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, 
* LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, 
* TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT 
* NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*
*********************************************************************
* File Description:
*
*  This is the main example file.  
*
* Change History:
*  Rev   Date         Author    Description
*  0.1   1/03/2008    yfy       Initial revision
*  2.0   4/15/2009    yfy       MiMAC and MiApp revision
*  0.2   9/03/2008    DWenn     Modified for the FreeRTOS and Stacks
*                               demo. Made incrementing rate meter
*  1.1   06/20/2009   yfy       Add LCD support
*  0.3   10/20/2008	  DWenn     Generates data for Electricty and Gas
********************************************************************/

/************************ HEADERS **********************************/
#include "Common\Console.h"
#include "Transceivers\Transceivers.h"
#include "Common\SymbolTime.h"
#include "WirelessProtocols\MCHP_API.h"
#include "Common\LCDBlocking.h"

/************************ VARIABLES ********************************/

/*******************************************************************/
// AdditionalNodeID variable array defines the additional 
// information to identify a device on a PAN. This array
// will be transmitted when initiate the connection between 
// the two devices. This  variable array will be stored in 
// the Connection Entry structure of the partner device. The 
// size of this array is ADDITIONAL_NODE_ID_SIZE, defined in 
// ConfigApp.h.
// In this demo, this variable array is set to be empty.
/******************************************************************/
#if ADDITIONAL_NODE_ID_SIZE > 0
    BYTE AdditionalNodeID[ADDITIONAL_NODE_ID_SIZE] = {'M', 'E', 'T', 'E', 'R', '0', '1'};
#endif

/*******************************************************************/
// The variable myChannel defines the channel that the P2P connection
// is operate on. This variable will be only effective if energy scan
// (ENABLE_ED_SCAN) is not turned on. Once the energy scan is turned
// on, the operating channel will be one of the channels available with
// least amount of energy (or noise).
/*******************************************************************/
BYTE myChannel = 25;

///////////////////////////////////////////////////////////////////
// Added for FreeRTOS and Stacks Demo
// dwMeterReading is the current meter reading
DWORD dwMeterCounter;
// update rate is the number of passes around the main loop before
// a new message is sent out. We send out dwMeterCounter as
// a value however the actual value may be ignored as just
// the sending of a messages will increment the meter
DWORD gasUpdateCounter;
int gasAcceleration;
DWORD electricUpdateCounter;
int electricAcceleration;
DWORD buttonCheckCounter;

// base rates for gas and electric
#define BASE_RATE_GAS		39000
#define BASE_RATE_ELECTRIC	38000
#define BASE_RATE_BUTTON	5000

/*********************************************************************
* Function:         void main(void)
*
* PreCondition:     none
*
* Input:		    none
*
* Output:		    none
*
* Side Effects:	    none
*
* Overview:		    This is the main function that runs the simple 
*                   example demo. The purpose of this example is to
*                   demonstrate the simple application programming
*                   interface for the MiWi(TM) P2P stack. By virtually
*                   total of less than 30 lines of code, we can develop
*                   a complete application using MiWi(TM) P2P APIs
*                   provided the stack. The application will first 
*                   try to establish a P2P link with another device
*                   and then process the received information as well
*                   as transmit its own information.
*                   MiWi(TM) P2P stack also support a set of rich 
*                   features. Example code RichFeatureExampleP2P will
*                   demonstrate how to implement the rich features with
*                   MiWi(TM) P2P stack application programming interfaces.
*
* Note:			    
********************************************************************/
#if defined(__18CXX)
void main(void)
#else
int main(void)
#endif
{   
    BYTE i, j;
    BYTE PressedButton; 
    char outputBuff[20];
    
    /*******************************************************************/
    // Initialize the system
    /*******************************************************************/
    BoardInit();         
    ConsoleInit();  
    LED_1 = 0;
    LED_2 = 0;
    /*******************************************************************/
    // Following block display demo information on LCD of Explore 16 or 
    // PIC18 Explorer demo board.
    /*******************************************************************/
    #if defined(MRF49XA)
        LCDDisplay((char *)"Simple P2P Demo  MRF49XA Node 2", 0, TRUE); 
    #elif defined(MRF24J40)
        LCDDisplay((char *)"Simple P2P Demo MRF24J40 Node 2", 0, TRUE);
    #endif
    MiApp_ProtocolInit();   
    #if defined(PICDEMZ) || defined(PIC18_EXPLORER)
        INTCONbits.GIEH = 1;
    #elif defined(EXPLORER16)
    #else
        #error "Unknown board.  Please initialize board as required."
    #endif

    ConsolePutROMString((ROM char*)"\r\nStarting P2P Demo...");
    
    // Set default channel
    if( MiApp_SetChannel(myChannel) == FALSE )
    {
        Printf("\r\nSelection of channel ");
        PrintDec(myChannel);
        Printf(" is not supported in current condition.\r\n");
        #if defined(__18CXX)
            return;
        #else
            return 0;
        #endif
    }
    
    /*******************************************************************/
    // Function MiApp_ConnectionMode defines the connection mode. The
    // possible connection modes are:
    //  ENABLE_ALL_CONN:    Enable all kinds of connection
    //  ENABLE_PREV_CONN:   Only allow connection already exists in 
    //                      connection table
    //  ENABL_ACTIVE_SCAN_RSP:  Allow response to Active scan
    //  DISABLE_ALL_CONN:   Disable all connections. 
    /*******************************************************************/
    MiApp_ConnectionMode(ENABLE_ALL_CONN);
    
    /*******************************************************************/
    // Display current opertion on LCD of demo board, if applicable
    /*******************************************************************/
    LCDDisplay((char *)"Connecting Peer  on Channel %d", myChannel, FALSE);
    
    /*******************************************************************/
    // Function MiApp_EstablishConnection try to establish a new connection
    // with peer device. 
    // The first parameter is the index to the active scan result, which is
    //      acquired by discovery process (active scan). If the value of the 
    //      index is 0xFF, try to establish a connection with any peer.
    // The second parameter is the mode to establish connection, either direct
    //      or indirect. Direct mode means connection within the radio range;
    //      Indirect mode means connection may or may not in the radio range. 
    /*******************************************************************/
    while( (i = MiApp_EstablishConnection(0xFF, CONN_MODE_DIRECT)) == 0xFF );

    /*******************************************************************/
    // Display current opertion on LCD of demo board, if applicable
    /*******************************************************************/
    LCDDisplay((char *)" Connected Peer  on Channel %d", myChannel, TRUE);
    
    /*******************************************************************/
    // Function DumpConnection is used to print out the content of the
    // P2P Connection Entry on the hyperterminal. It may be useful in 
    // the debugging phase.
    // The only parameter of this function is the index of the P2P 
    // Connection Entry. The value of 0xFF means to print out all
    // valid P2P Connection Entry; otherwise, the P2P Connection Entry
    // of the input index will be printed out.
    /*******************************************************************/
    #ifdef ENABLE_DUMP
        DumpConnection(i);
    #endif

    // Turn on LED 1 to indicate P2P connection established
    LED_1 = 1;

	// set up the initial meter readings
	gasUpdateCounter = BASE_RATE_GAS;
	gasAcceleration = 0;
	electricUpdateCounter = BASE_RATE_ELECTRIC;
	electricAcceleration = 0;
	buttonCheckCounter = BASE_RATE_BUTTON;
	dwMeterCounter = 0;
	
    while(1)
    {
        /*******************************************************************/
        // Function ReceivedPacket will return a boolean to indicate if a 
        // packet has been received by the transceiver. If a packet has been
        // received, all information will be stored in the rxFrame, structure  
        // of RECEIVED_FRAME.
        /*******************************************************************/
        if( MiApp_MessageAvailable() )
        {
            /*******************************************************************/
            // If a packet has been received, following code prints out some of
            // the information available in rxMessage.
            /*******************************************************************/
            ConsolePutROMString((ROM char *)"Received Packet ");
            #ifdef ENABLE_SECURITY
                if( rxMessage.flags.bits.secEn )
                {
                    ConsolePutROMString((ROM char *)"Secured ");
                }
            #endif
            #ifndef TARGET_SMALL
                if( rxMessage.flags.bits.broadcast )
                {
                    ConsolePutROMString((ROM char *)"Broadcast with RSSI ");
                }
                else
                {
                    ConsolePutROMString((ROM char *)"Unicast with RSSI ");
                }
                PrintChar(rxMessage.PacketRSSI);
                if( rxMessage.flags.bits.srcPrsnt )
                {
                    ConsolePutROMString((ROM char *)" from ");
                    for(i = 0; i < (BYTE) MY_ADDRESS_LENGTH; i++)
                    {
                        PrintChar(rxMessage.SourceAddress[MY_ADDRESS_LENGTH-1-i]);
                    }
                }
            #endif
            ConsolePutROMString((ROM char *)": ");
            for(i = 0; i < rxMessage.PayloadSize; i++)
            {
                ConsolePut(rxMessage.Payload[i]);
            }
            
            // Toggle LED2 to indicate receiving a packet.
            LED_2 ^= 1;

            /*******************************************************************/
            // Function MiApp_DiscardMessage is used to release the current 
            //  received packet.
            // After calling this function, the stack can start to process the
            //  next received frame 
            /*******************************************************************/  
            MiApp_DiscardMessage();
            
        }
        else
        {
            /*******************************************************************/
            // If no packet received, now we can check if we want to send out
            // any information.
            // If the update counter is zero send an update to the main unit
            
            gasUpdateCounter--;
            
            if (gasUpdateCounter == 0ul) {
	            
	        	// send out an update    
	            sprintf(outputBuff, "G%ld", dwMeterCounter);	            
                MiApp_FlushTx();
	            for(i = 0; i < strlen(outputBuff); i++)
	            	MiApp_WriteData(outputBuff[i]);
	            // send out an update
				MiApp_BroadcastPacket(FALSE);
				
				// update the counter based upon the button state
				gasUpdateCounter = BASE_RATE_GAS -  300 * gasAcceleration;
	        }
	        
	        electricUpdateCounter--;
	        if (electricUpdateCounter == 0ul) {
		        
		        // send out an update
	            sprintf(outputBuff, "E%ld", dwMeterCounter);
                MiApp_FlushTx();
	            for(i = 0; i < strlen(outputBuff); i++)
	            	MiApp_WriteData(outputBuff[i]);

	            // send out an update
				MiApp_BroadcastPacket(FALSE);
		        
		        electricUpdateCounter = BASE_RATE_ELECTRIC - 300 * electricAcceleration;		        		        
		   	}
            
            
            // periodically check the state of the buttons
            // and effect the acceleration for gas and electric changes
            buttonCheckCounter--;
            if (buttonCheckCounter == 0u) {
		        if (PUSH_BUTTON_1 == 0u) {
					// accelerate the electricty usage
					if (electricAcceleration < 100)
						electricAcceleration++;  
		        } else {
	            	// decelerate the usage
	            	if (electricAcceleration > -10)
	            		electricAcceleration--;
	         	}
	         	   
		        if (PUSH_BUTTON_2 == 0u) {
					// accelerate the gas usage	
					if (gasAcceleration < 100)
						gasAcceleration++;  
		        } else {
	            	// decelerate the usage
	            	if (gasAcceleration > -10)
	            		gasAcceleration--;
	         	}

	         	buttonCheckCounter = BASE_RATE_BUTTON;   
	        }
        }
    }
}
