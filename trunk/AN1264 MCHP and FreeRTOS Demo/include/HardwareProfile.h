/*********************************************************************
 *
 *	Hardware specific definitions
 *
 *********************************************************************
 * FileName:        HardwareProfile.h
 * Dependencies:    None
 * Processor:       PIC24F, PIC32
 * Compiler:        Microchip C32 v1.00 or higher
 *					Microchip C30 v3.01 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2008 Microchip Technology Inc.  All rights 
 * reserved.
 *
 * You should refer to the license agreement accompanying this 
 * Software for additional information regarding your rights and 
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT 
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT 
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL 
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR 
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF 
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS 
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE 
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER 
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT 
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author               Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Darren Wenn  		09/10/08	Original
 * Darren Wenn			06/01/09    Upgraded to Graphics V1.65 and 
 *									TCPIP V5.00
 * Darren Wenn          09/03/09    Upgraded to Graphics V1.75, MiWi P2P 2.1
 *                                  and TCPIP V5.10
 ********************************************************************/
#ifndef __HARDWARE_PROFILE_H
#define __HARDWARE_PROFILE_H

// we must include this since we build dependent upon the graphics controller
#include "GraphicsConfig.h"

// this project assumes that the MiWi PICTail+ is in the upper slot
// and the ENC is in the lower slot
///////////////////////////////////////////////////////////////////
// Choose which hardware profile to compile for here. 
#if defined(__PIC24F__)
	// must use an Explorer 16 for PIC24F and dsPIC
	#define EXPLORER_16	// PIC24FJ128GA010
#else
	// if we are using PIC32 define what demo platform we are using
	#define EXPLORER_16	// PIC32MX360F512L
	//#define PIC32_SK_IO	// PIC32MX360F512L Starter Kit mounted on IO Expansion Board
#endif

// Choose which Ethernet adapter to use
//#define ENC624J600
#define ENC28J60
#if  !defined(ENC624J600) && !defined(ENC28J60)
#error "Please define ENC624J600 OR ENC28J60 to specify the ethernet adapter"
#endif

// This value is used to calculate Tick Counter value
#if defined(__PIC24F__)	
	// PIC24F processor
	#define GetSystemClock()		(32000000ul)      // Hz
	#define GetInstructionClock()	(GetSystemClock()/2)
	#define GetPeripheralClock()	(GetInstructionClock())
	#define CLOCK_FREQ              (32000000ul)
#elif defined(__PIC32MX__)
	// PIC32MX processor
	#define GetSystemClock()		(80000000ul)      // Hz
	#define GetInstructionClock()	(GetSystemClock()/1)
	#define GetPeripheralClock()	(GetInstructionClock()/2)	
	#define CLOCK_FREQ              (80000000ul)
#endif


/******************************************************************
 * IOs for the Display Controller
 *****************************************************************/
#if (GRAPHICS_PICTAIL_VERSION == 1)
	#error Unsupported Graphics Board
#elif (GRAPHICS_PICTAIL_VERSION == 2)

	#if (DISPLAY_CONTROLLER == LGDP4531)
	
	// definitions for reset pin
	#define RST_TRIS_BIT		TRISCbits.TRISC1
	#define RST_LAT_BIT			LATCbits.LATC1
	
	// definitions for RS pin
	#define RS_TRIS_BIT			TRISCbits.TRISC2
	#define RS_LAT_BIT			LATCbits.LATC2
	
	//definitions for CS pin
	#define CS_TRIS_BIT			TRISDbits.TRISD10
	#define CS_LAT_BIT			LATDbits.LATD10
	
	// defintions for POWER ON pin
	#define POWERON_TRIS_BIT	TRISCbits.TRISC3
	#define POWERON_LAT_BIT		LATCbits.LATC3
	
	#elif (DISPLAY_CONTROLLER == SSD1906)
	
	// definitions for reset line
	#define RST_TRIS_BIT		TRISCbits.TRISC1
	#define RST_LAT_BIT			LATCbits.LATC1
	
	// defintions for RS line
	#define RS_TRIS_BIT			TRISCbits.TRISC2
	#define RS_LAT_BIT			LATCbits.LATC2
	
	// defintions for CS line
	#define CS_TRIS_BIT			TRISDbits.TRISD10
	#define CS_LAT_BIT			LATDbits.LATD10
	
	// definitions for A0 line
	#define A0_TRIS_BIT			TRISDbits.TRISD3
	#define A0_LAT_BIT			LATDbits.LATD3
	
	// definitions for A17 line
	#define A17_TRIS_BIT		TRISGbits.TRISG14
	#define A17_LAT_BIT			LATGbits.LATG14
	
	#else
	#error Graphics controller is not supported
	#endif // display controller ==
	
	#define SST39_CS_TRIS    TRISDbits.TRISD1
	#define SST39_CS_LAT     LATDbits.LATD1
	#define SST39_A18_TRIS   TRISCbits.TRISC2
	#define SST39_A18_LAT    LATCbits.LATC2
	#define SST39_A17_TRIS   TRISGbits.TRISG15
	#define SST39_A17_LAT    LATGbits.LATG15
	#define SST39_A16_TRIS   TRISGbits.TRISG14
	#define SST39_A16_LAT    LATGbits.LATG14
	
	// we need a quick method of changing between QVGA use of the PMP and
	// FLASH use so we have these macros to help this
	// was PMMODE = 0x0A4A
	#define SST39PMPInit()	{ \
								while( PMMODEbits.BUSY); \
								PMMODE = 0x0A49; \
								PMAEN = 0x0003; \
								PMCON = 0x9320;\
							}
							
	#define LCDPMPInit()	{ \
								while (PMMODEbits.BUSY); \
								PMMODE = 0x0204; \
								PMAEN = 0x0000; \
								PMCON = 0x8300; \
							}
							
#elif (GRAPHICS_PICTAIL_VERSION == 3)

	// Definitions for reset line
	#define RST_TRIS_BIT		TRISCbits.TRISC1
	#define RST_LAT_BIT			LATCbits.LATC1
	
	// Definitions for RS line
	#define RS_TRIS_BIT			TRISCbits.TRISC2
	#define RS_LAT_BIT			LATCbits.LATC2
	
	// definitions for CS line
	#define CS_TRIS_BIT			TRISDbits.TRISD10
	#define CS_LAT_BIT			LATDbits.LATD10
	
	#define SST25_CS_TRIS       TRISDbits.TRISD1
	#define SST25_CS_LAT        LATDbits.LATD1
	#define SST25_SCK_TRIS      TRISGbits.TRISG6
	#define SST25_SDO_TRIS      TRISGbits.TRISG8
	#define SST25_SDI_TRIS      TRISGbits.TRISG7

#else
	#error Graphics PICTail not defined
#endif


// Hardware mappings
#if defined(EXPLORER_16)
	// Explorer 16 + PIC24FJ128GA010 + Ethernet PICtail Plus + MiWi + QVGA 2
	// Explorer 16 + PIC32MX360F512L PIM + Ethernet PICtail Plus + MiWi + QVGA 2

	// The LEDs
	#define LED0_TRIS			(TRISAbits.TRISA0)	// Ref D3
	#define LED0_IO				(LATAbits.LATA0)	
	#define LED1_TRIS			(TRISAbits.TRISA1)	// Ref D4
	#define LED1_IO				(LATAbits.LATA1)
	#define LED4_TRIS			(TRISAbits.TRISA4)	// Ref D7
	#define LED4_IO				(LATAbits.LATA4)
	#define LED5_TRIS			(TRISAbits.TRISA5)	// Ref D8
	#define LED5_IO				(LATAbits.LATA5)
	#define LED6_TRIS			(TRISAbits.TRISA6)	// Ref D9
	#define LED6_IO				(LATAbits.LATA6)
	#define LED0				0x0001
	#define LED1				0x0002
	#define LED4				0x0010
	#define LED5				0x0020
	#define LED6				0x0040

	// The Buttons
	#define BUTTON0_TRIS		(TRISDbits.TRISD13)	// Ref S4
	#define	BUTTON0_IO			(PORTDbits.RD13)
	#define BUTTON1_TRIS		(TRISAbits.TRISA7)	// Ref S5	// Note: This is multiplexed with LED7
	#define	BUTTON1_IO			(PORTAbits.RA7)
	#define BUTTON2_TRIS		(TRISDbits.TRISD7)	// Ref S6
	#define	BUTTON2_IO			(PORTDbits.RD7)
	#define BUTTON3_TRIS		(TRISDbits.TRISD6)	// Ref S3
	#define	BUTTON3_IO			(PORTDbits.RD6)

	// The UART
	#define UART_BAUD_RATE		19200
	#define UARTTX_TRIS			(TRISFbits.TRISF5)
	#define UARTTX_IO			(PORTFbits.RF5)
	#define UARTRX_TRIS			(TRISFbits.TRISF4)
	#define UARTRX_IO			(PORTFbits.RF4)
	#define UBRG				U2BRG
	#define UMODE				U2MODE
	#define USTA				U2STA
	#define USTAbits			U2STAbits
	#define UTXREG				U2TXREG
	#define URXREG				U2RXREG
	
	#if defined(__C30__) 	// PIC24F
		#define UART_CONFIG1 (UART_EN)
		#define UART_CONFIG2 (UART_TX_ENABLE | UART_INT_RX_CHAR)
		#define CLOSEST_BRG ((GetPeripheralClock()+8ul*UART_BAUD_RATE)/16/UART_BAUD_RATE-1)
		// because of bit alignment we have hardcoded the priority to be
		// configKERNEL_INTERRUPT_PRIORITY + 1 = 2
		#define UART_INT_CONFIG (UART_RX_INT_EN | UART_RX_INT_PR2)
	#else					// PIC32
		#define UART_CONFIG1 (UART_EN)
		#define UART_CONFIG2 (UART_RX_ENABLE | UART_TX_ENABLE | UART_INT_RX_CHAR)
		#define CLOSEST_BRG ((GetPeripheralClock()+8ul*UART_BAUD_RATE)/16/UART_BAUD_RATE-1)
		#define UART_INT_CONFIG ((configKERNEL_INTERRUPT_PRIORITY + 1) | UART_RX_INT_EN)
	#endif

	#define OpenUART(a,b,c)			OpenUART2(a,b,c)
	#define ConfigIntUART(a)		ConfigIntUART2(a)

	// enc in lower slot
	#if defined(ENC28J60)
		// ENC28J60 I/O pins, comment this line to enable ENC624J600 support
		#define ENC_CS_TRIS			(TRISFbits.TRISF12)
	#endif
	#define ENC_CS_IO			(LATFbits.LATF12)

	// SPI SCK, SDI, SDO pins are automatically controlled by the 
	// PIC24/PIC32 SPI module 
	#if defined(__C30__)	// PIC24F
		#define ENC_SPI_IF			(IFS2bits.SPI2IF)
		#define ENC_SSPBUF			(SPI2BUF)
		#define ENC_SPISTAT			(SPI2STAT)
		#define ENC_SPISTATbits		(SPI2STATbits)
		#define ENC_SPICON1			(SPI2CON1)
		#define ENC_SPICON1bits		(SPI2CON1bits)
		#define ENC_SPICON2			(SPI2CON2)
	#else					// PIC32
		#define ENC_SPI_IF			(IFS1bits.SPI2RXIF)
		#define ENC_SSPBUF			(SPI2BUF)
		#define ENC_SPISTATbits		(SPI2STATbits)
		#define ENC_SPICON1			(SPI2CON)
		#define ENC_SPICON1bits		(SPI2CONbits)	
		#define ENC_SPIBRG			(SPI2BRG)
	#endif

	// ENC624J600 Interface Configuration
	// Comment out ENC100_INTERFACE_MODE if you don't have an ENC624J600 or 
	// ENC424J600.  Otherwise, choose the correct setting for the interface you 
	// are using.  Legal values are:
	//  - Commented out: No ENC424J600/624J600 present or used.  All other 
	//                   ENC100_* macros are ignored.
	//	- 0: SPI mode using CS, SCK, SI, and SO pins
	#if defined(ENC624J600)
		#define ENC100_INTERFACE_MODE			0
	#endif
	
	// Auto-crossover pins on Fast 100Mbps Ethernet PICtail/PICtail Plus.  If 
	// your circuit doesn't have such a feature, delete these two defines.
	#define ENC100_MDIX_TRIS				(TRISAbits.TRISA9)
	#define ENC100_MDIX_IO					(LATAbits.LATA9)

	// ENC624J600 I/O control and status pins
	// If a pin is not required for your selected ENC100_INTERFACE_MODE 
	// interface selection (ex: WRH/B1SEL for PSP modes 1, 2, 5, and 6), then 
	// you can ignore, delete, or put anything for the pin definition.  Also, 
	// the INT and POR pins are entirely optional.  If not connected, comment 
	// them out.
	#if defined(__dsPIC33FJ256GP710__)
		#warning "Verify ENC624J600 interrupt connection on this processor"
		#define ENC100_INT_TRIS				(TRISAbits.TRISA15)		// INT signal is optional and currently unused in the Microchip TCP/IP Stack.  Leave this pin disconnected and comment out this pin definition if you don't want it.
		#define ENC100_INT_IO				(PORTAbits.RA15)
	#else
		#define ENC100_INT_TRIS				(TRISAbits.TRISA15)		// INT signal is optional and currently unused in the Microchip TCP/IP Stack.  Leave this pin disconnected and comment out this pin definition if you don't want it.
		#define ENC100_INT_IO				(PORTAbits.RA159)
	#endif
	
	#if (ENC100_INTERFACE_MODE >= 1)	// Parallel mode
		#error "ENC624J600 mode not supported"
	#else	
		// SPI pinout
		#define ENC100_CS_TRIS					(TRISFbits.TRISF12)	// CS is mandatory when using the SPI interface
		#define ENC100_CS_IO					(LATFbits.LATF12)
		#define ENC100_POR_TRIS					(TRISFbits.TRISF13)	// POR signal is optional.  If your application doesn't have a power disconnect feature, comment out this pin definition.
		#define ENC100_POR_IO					(LATFbits.LATF13)
		#define ENC100_SO_WR_B0SEL_EN_TRIS		(TRISGbits.TRISG7)	// SO is ENCX24J600 Serial Out, which needs to connect to the PIC SDI pin for SPI mode
		#define ENC100_SO_WR_B0SEL_EN_IO		(PORTGbits.RG7)
		#define ENC100_SI_RD_RW_TRIS			(TRISGbits.TRISG8)	// SI is ENCX24J600 Serial In, which needs to connect to the PIC SDO pin for SPI mode
		#define ENC100_SI_RD_RW_IO				(LATGbits.LATG8)
		#define ENC100_SCK_AL_TRIS				(TRISGbits.TRISG6)
	#endif

	// ENC624J600 SPI SFR register selection (controls which SPI peripheral to 
	// use on PICs with multiple SPI peripherals).  
	#if defined(__C30__)	// PIC24F, PIC24H, dsPIC30, dsPIC33
		#define ENC100_ISR_ENABLE		(IEC3bits.INT4IE)
		#define ENC100_ISR_FLAG			(IFS3bits.INT4IF)	
		#define ENC100_ISR_POLARITY		(INTCON2bits.INT4EP)
		#define ENC100_ISR_PRIORITY		(IPC13bits.INT4IP)
		#define ENC100_SPI_ENABLE		(ENC100_SPISTATbits.SPIEN)
		#define ENC100_SPI_IF			(IFS0bits.SPI2IF)
		#define ENC100_SSPBUF			(SPI2BUF)
		#define ENC100_SPISTAT			(SPI2STAT)
		#define ENC100_SPISTATbits		(SPI2STATbits)
		#define ENC100_SPICON1			(SPI2CON1)
		#define ENC100_SPICON1bits		(SPI2CON1bits)
		#define ENC100_SPICON2			(SPI2CON2)
	#else					// PIC32MX
		#define ENC100_ISR_ENABLE		(IEC0bits.INT4IE)
		#define ENC100_ISR_FLAG			(IFS0bits.INT4IF)
		#define ENC100_ISR_POLARITY		(INTCONbits.INT4EP)	
		#define ENC100_ISR_PRIORITY		(IPC2bits.INT4IP)	
		#define ENC100_SPI_ENABLE		(ENC100_SPICON1bits.ON)
		#define ENC100_SPI_IF			(IFS0bits.SPI2RXIF)
		#define ENC100_SSPBUF			(SPI2BUF)
		#define ENC100_SPICON1			(SPI2CON)
		#define ENC100_SPISTATbits		(SPI2STATbits)
		#define ENC100_SPICON1bits		(SPI2CONbits)
		#define ENC100_SPIBRG			(SPI2BRG)
	#endif

	// MRF24J40 I/O pins
	// we assume that the PICTail+ is in the upper slot
	#if defined(__PIC32MX__)
		#define RFIF	IFS0bits.INT1IF	
		#define RFIE	IEC0bits.INT1IE
		#define RFIP	IPC1bits.INT1IP
	#else
		#define RFIF	IFS1bits.INT1IF
		#define RFIE	IEC1bits.INT1IE
		#define RFIP	IPC5bits.INT1IP
	#endif

    #define RF_INT_PIN PORTEbits.RE8
    #define TMRL TMR2
    #define PHY_CS LATBbits.LATB2
    #define PHY_CS_TRIS TRISBbits.TRISB2
    #define PHY_RESETn LATGbits.LATG2
    #define PHY_RESETn_TRIS TRISGbits.TRISG2
    #define PHY_WAKE LATGbits.LATG3
    #define PHY_WAKE_TRIS TRISGbits.TRISG3    

	// Analog sensors on the Explorer16 board
	#ifdef __PIC32MX__
    	#define ADC_TEMP        ADC_CH0_POS_SAMPLEA_AN4
    	#define ADC_POT         ADC_CH0_POS_SAMPLEA_AN5
	#else
    	#define ADC_TEMP        4
    	#define ADC_POT         5
	#endif
   	#define ADPCFG_TEMP			AD1PCFGbits.PCFG4
   	#define ADPCFG_POT			AD1PCFGbits.PCFG5

	#if (GRAPHICS_PICTAIL_VERSION == 2)
	   	#ifdef __PIC32MX__
	       	#define ADC_XPOS        ADC_CH0_POS_SAMPLEA_AN11
	       	#define ADC_YPOS        ADC_CH0_POS_SAMPLEA_AN10       	
	   	#else
	       	#define ADC_XPOS        11
	       	#define ADC_YPOS        10
	   	#endif
	   	
	    // Y port definitions
	    #define ADPCFG_XPOS     AD1PCFGbits.PCFG11
	    #define LAT_XPOS        LATBbits.LATB11
	    #define LAT_XNEG        LATGbits.LATG13
	    #define TRIS_XPOS       TRISBbits.TRISB11
	    #define TRIS_XNEG       TRISGbits.TRISG13
	   
	    // X port definitions
	    #define ADPCFG_YPOS     AD1PCFGbits.PCFG10
	    #define LAT_YPOS        LATBbits.LATB10  
	    #define LAT_YNEG        LATGbits.LATG12 
	    #define TRIS_YPOS       TRISBbits.TRISB10
	    #define TRIS_YNEG       TRISGbits.TRISG12 
	#elif (GRAPHICS_PICTAIL_VERSION == 3)
	   #ifdef __PIC32MX__
	        #define ADC_XPOS        ADC_CH0_POS_SAMPLEA_AN11
	        #define ADC_YPOS        ADC_CH0_POS_SAMPLEA_AN10
	    #else
	        #define ADC_XPOS        11
	        #define ADC_YPOS        10
	    #endif
	    
	    // Y port definitions
	    #define ADPCFG_XPOS     AD1PCFGbits.PCFG11
	    #define LAT_XPOS        LATBbits.LATB11
	    #define LAT_XNEG        LATDbits.LATD9
	    #define TRIS_XPOS       TRISBbits.TRISB11
	    #define TRIS_XNEG       TRISDbits.TRISD9
	    
	    // X port definitions
	    #define ADPCFG_YPOS     AD1PCFGbits.PCFG10
	    #define LAT_YPOS        LATBbits.LATB10  
	    #define LAT_YNEG        LATDbits.LATD8 
	    #define TRIS_YPOS       TRISBbits.TRISB10
	    #define TRIS_YNEG       TRISDbits.TRISD8 
    #else
    	#error "Unsupported graphics pictail"
    #endif
    
#elif defined(PIC32_SK_IO)
	// PIC32 Starter kit running on an IO expansion board. To make
	// the coding easier rather than commenting out IO pins and SFRs
	// here (which will then break the UART and EEPROM files) we will
	// just ifdef out sections of code that actually use that
	// functionality.
	#define LED0_TRIS			(TRISDbits.TRISD2)	
	#define LED0_IO				(LATDbits.LATD2)	
	#define LED1_TRIS			(TRISDbits.TRISD2)	
	#define LED1_IO				(LATDbits.LATD2)
	#define LED4_TRIS			(TRISDbits.TRISD2)
	#define LED4_IO				(LATDbits.LATD2)
	#define LED5_TRIS			(TRISDbits.TRISD2)
	#define LED5_IO				(LATDbits.LATD2)
	#define LED6_TRIS			(TRISDbits.TRISD2)	
	#define LED6_IO				(LATDbits.LATD2)
	#define LED0				0x0004
	#define LED1				0x0004
	#define LED4				0x0004
	#define LED5				0x0004
	#define LED6				0x0004

	// The Buttons
	#define BUTTON1_TRIS		(TRISDbits.TRISD13)	// Ref SW1
	#define	BUTTON1_IO			(PORTDbits.RD13)
	#define BUTTON2_TRIS		(TRISDbits.TRISD13)	// Ref SW2
	#define	BUTTON2_IO			(PORTDbits.RD13)
	#define BUTTON3_TRIS		(TRISDbits.TRISD13)	// Ref SW3
	#define	BUTTON3_IO			(PORTDbits.RD13)

	// The UART
	#define UART_BAUD_RATE		19200
	#define UARTTX_TRIS			(TRISFbits.TRISF5)
	#define UARTTX_IO			(PORTFbits.RF5)
	#define UARTRX_TRIS			(TRISFbits.TRISF4)
	#define UARTRX_IO			(PORTFbits.RF4)
	#define UBRG				U2BRG
	#define UMODE				U2MODE
	#define USTA				U2STA
	#define USTAbits			U2STAbits
	#define UTXREG				U2TXREG
	#define URXREG				U2RXREG
	
	#define UART_CONFIG1 (UART_EN)
	#define UART_CONFIG2 (UART_RX_ENABLE | UART_TX_ENABLE | UART_INT_RX_CHAR)
	#define CLOSEST_BRG ((GetPeripheralClock()+8ul*UART_BAUD_RATE)/16/UART_BAUD_RATE-1)
	#define UART_INT_CONFIG ((configKERNEL_INTERRUPT_PRIORITY + 1) | UART_RX_INT_EN)

	#define OpenUART(a,b,c)			OpenUART2(a,b,c)
	#define ConfigIntUART(a)		ConfigIntUART2(a)

	// enc in lower slot
	#define ENC_RST_TRIS		(TRISFbits.TRISF13)	// Not connected by default
	#define ENC_RST_IO			(LATFbits.LATF13)
	#if defined(ENC28J60)
    	// ENC28J60 I/O pins, comment this line to enable ENC624J600 support
	    #define ENC_CS_TRIS			(TRISFbits.TRISF12)
	#endif
	#define ENC_CS_IO			(LATFbits.LATF12)

	#define ENC_SPI_IF			(IFS1bits.SPI2RXIF)
	#define ENC_SSPBUF			(SPI2BUF)
	#define ENC_SPISTATbits		(SPI2STATbits)
	#define ENC_SPICON1			(SPI2CON)
	#define ENC_SPICON1bits		(SPI2CONbits)	
	#define ENC_SPIBRG			(SPI2BRG)

	// ENC624J600 Interface Configuration
	// Comment out ENC100_INTERFACE_MODE if you don't have an ENC624J600 or 
	// ENC424J600.  Otherwise, choose the correct setting for the interface you 
	// are using.  Legal values are:
	//  - Commented out: No ENC424J600/624J600 present or used.  All other 
	//                   ENC100_* macros are ignored.
	//	- 0: SPI mode using CS, SCK, SI, and SO pins
	#if defined(ENC624J600)
        #define ENC100_INTERFACE_MODE			0
    #endif

	// Auto-crossover pins on Fast 100Mbps Ethernet PICtail/PICtail Plus.  If 
	// your circuit doesn't have such a feature, delete these two defines.
	#define ENC100_MDIX_TRIS				(TRISAbits.TRISA9)
	#define ENC100_MDIX_IO					(LATAbits.LATA9)

	// ENC624J600 I/O control and status pins
	// If a pin is not required for your selected ENC100_INTERFACE_MODE 
	// interface selection (ex: WRH/B1SEL for PSP modes 1, 2, 5, and 6), then 
	// you can ignore, delete, or put anything for the pin definition.  Also, 
	// the INT and POR pins are entirely optional.  If not connected, comment 
	// them out.
	#define ENC100_INT_TRIS				(TRISAbits.TRISA15)		// INT signal is optional and currently unused in the Microchip TCP/IP Stack.  Leave this pin disconnected and comment out this pin definition if you don't want it.
	#define ENC100_INT_IO				(PORTAbits.RA159)
	
	#if (ENC100_INTERFACE_MODE >= 1)	// Parallel mode
		#error "ENC624J600 mode not supported"
	#else	
		// SPI pinout
		#define ENC100_CS_TRIS					(TRISFbits.TRISF12)	// CS is mandatory when using the SPI interface
		#define ENC100_CS_IO					(LATFbits.LATF12)
		#define ENC100_POR_TRIS					(TRISFbits.TRISF13)	// POR signal is optional.  If your application doesn't have a power disconnect feature, comment out this pin definition.
		#define ENC100_POR_IO					(LATFbits.LATF13)
		#define ENC100_SO_WR_B0SEL_EN_TRIS		(TRISGbits.TRISG7)	// SO is ENCX24J600 Serial Out, which needs to connect to the PIC SDI pin for SPI mode
		#define ENC100_SO_WR_B0SEL_EN_IO		(PORTGbits.RG7)
		#define ENC100_SI_RD_RW_TRIS			(TRISGbits.TRISG8)	// SI is ENCX24J600 Serial In, which needs to connect to the PIC SDO pin for SPI mode
		#define ENC100_SI_RD_RW_IO				(LATGbits.LATG8)
		#define ENC100_SCK_AL_TRIS				(TRISGbits.TRISG6)
	#endif

	// ENC624J600 SPI SFR register selection (controls which SPI peripheral to 
	// use on PICs with multiple SPI peripherals).  
	#define ENC100_ISR_ENABLE		(IEC0bits.INT4IE)
	#define ENC100_ISR_FLAG			(IFS0bits.INT4IF)
	#define ENC100_ISR_POLARITY		(INTCONbits.INT4EP)	
	#define ENC100_ISR_PRIORITY		(IPC2bits.INT4IP)	
	#define ENC100_SPI_ENABLE		(ENC100_SPICON1bits.ON)
	#define ENC100_SPI_IF			(IFS0bits.SPI2RXIF)
	#define ENC100_SSPBUF			(SPI2BUF)
	#define ENC100_SPICON1			(SPI2CON)
	#define ENC100_SPISTATbits		(SPI2STATbits)
	#define ENC100_SPICON1bits		(SPI2CONbits)
	#define ENC100_SPIBRG			(SPI2BRG)

	#define RFIF	IFS0bits.INT1IF	
	#define RFIE	IEC0bits.INT1IE
	#define RFIP	IPC1bits.INT1IP

    #define RF_INT_PIN PORTEbits.RE8
    #define TMRL TMR2
    #define PHY_CS LATBbits.LATB2
    #define PHY_CS_TRIS TRISBbits.TRISB2
    #define PHY_RESETn LATGbits.LATG2
    #define PHY_RESETn_TRIS TRISGbits.TRISG2
    #define PHY_WAKE LATGbits.LATG3
    #define PHY_WAKE_TRIS TRISGbits.TRISG3    

  	#define ADC_TEMP        ADC_CH0_POS_SAMPLEA_AN4
   	#define ADC_POT         ADC_CH0_POS_SAMPLEA_AN5
   	#define ADPCFG_TEMP			AD1PCFGbits.PCFG4
   	#define ADPCFG_POT			AD1PCFGbits.PCFG5

  	#if (GRAPHICS_PICTAIL_VERSION == 2)
       	#define ADC_XPOS        ADC_CH0_POS_SAMPLEA_AN11
       	#define ADC_YPOS        ADC_CH0_POS_SAMPLEA_AN10       	
	   	
	    // Y port definitions
	    #define ADPCFG_XPOS     AD1PCFGbits.PCFG11
	    #define LAT_XPOS        LATBbits.LATB11
	    #define LAT_XNEG        LATGbits.LATG13
	    #define TRIS_XPOS       TRISBbits.TRISB11
	    #define TRIS_XNEG       TRISGbits.TRISG13
	   
	    // X port definitions
	    #define ADPCFG_YPOS     AD1PCFGbits.PCFG10
	    #define LAT_YPOS        LATBbits.LATB10  
	    #define LAT_YNEG        LATGbits.LATG12 
	    #define TRIS_YPOS       TRISBbits.TRISB10
	    #define TRIS_YNEG       TRISGbits.TRISG12 
	#elif (GRAPHICS_PICTAIL_VERSION == 3)
        #define ADC_XPOS        ADC_CH0_POS_SAMPLEA_AN11
        #define ADC_YPOS        ADC_CH0_POS_SAMPLEA_AN10
	    
	    // Y port definitions
	    #define ADPCFG_XPOS     AD1PCFGbits.PCFG11
	    #define LAT_XPOS        LATBbits.LATB11
	    #define LAT_XNEG        LATDbits.LATD9
	    #define TRIS_XPOS       TRISBbits.TRISB11
	    #define TRIS_XNEG       TRISDbits.TRISD9
	    
	    // X port definitions
	    #define ADPCFG_YPOS     AD1PCFGbits.PCFG10
	    #define LAT_YPOS        LATBbits.LATB10  
	    #define LAT_YNEG        LATDbits.LATD8 
	    #define TRIS_YPOS       TRISBbits.TRISB10
	    #define TRIS_YNEG       TRISDbits.TRISD8 
    #else
    	#error "Unsupported graphics pictail"
    #endif    
#else
	#error "Hardware profile not defined." 
#endif

// Select which UART the STACK_USE_UART and STACK_USE_UART2TCP_BRIDGE 
// options will use.  You can change these to U1BRG, U1MODE, etc. if you 
// want to use the UART1 module instead of UART2.
#define BusyUART()				BusyUART2()
#define CloseUART()				CloseUART2()
#define DataRdyUART()			DataRdyUART2()
#define ReadUART()				ReadUART2()
#define WriteUART(a)			WriteUART2(a)
#define getsUART(a,b,c)			getsUART2(a,b,c)
#if defined(__C32__)
	#define putsUART(a)			putsUART2(a)
#else
	#define putsUART(a)			putsUART2((unsigned int*)a)
#endif
#define getcUART()				getcUART2()
#define putcUART(a)				WriteUART(a)
#define putrsUART(a)			putsUART(a)

#endif
