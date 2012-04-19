/*****************************************************************************
 *
 *              HardwareProfile.h -- Hardware Configuration 
 *
 *****************************************************************************
 * FileName:        HardwareProfile.h
 * Dependencies:
 * Processor:       PIC18F
 * Compiler:        C18 02.20.00 or higher
 *                  C30 02.03.00 or higher
 *                  C32 01.00.02 or higher
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         Microchip Technology Incorporated
 *
 * Software License Agreement
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
 *****************************************************************************
 * File Description:
 *
 *   This file provides configuration and basic hardware functionality 
 *   based on chosen hardware demo boards.
 *
 * Change History:
 *  Rev   Date         Description
 *  0.1   2/17/2009    Initial revision
 *  2.0   04/24/2009   Modified for MiApp interface
 *  2.1   06/20/2009   Add LCD support
 *****************************************************************************/
#ifndef _HARDWARE_PROFILE_H
    #define _HARDWARE_PROFILE_H

    #include "GenericTypeDefs.h"
    #include "ConfigApp.h"
    #ifdef __PIC32MX__
        #define RCON_SLEEP_MASK (0x8)
        #define RCON_IDLE_MASK  (0x4)
        #define        Sleep()      PowerSaveSleep()   
        void Enable_PB_1_2_Interrupts(void);
    #endif


    BYTE ButtonPressed(void);
    void BoardInit(void);

    void LCDDisplay(char *, BYTE, BOOL);
    void LCDTRXCount(BYTE, BYTE);


    /*********************************************************************/
    // The stack supports three Microchip standard demo boards out of the
    // box. All hardware configurations have been defined for those three
    // demo boards here. If the user needs to develop on a custom board,
    // hardware configuration needds to be defined in this file.
    // The supported standard demo boards are:
    //      PICDEM Z:       Support MRF24J40 only
    //      PIC18 Explorer: Support both MRF24J40 and MRF49XA
    //      Explorer 16:    Support both MRF24J40 and MRF49XA for PIC24,
    //                      dsPIC33 and PIC32
    // Only one of the demo boards can be used at a time.
    /*********************************************************************/
    #define PICDEMZ
    //#define PIC18_EXPLORER
    //#define EXPLORER16
    

    #if defined(PIC18_EXPLORER) 
        #define CLOCK_FREQ 10000000

        // Transceiver Configuration
        #if defined(MRF49XA)
            #define RFIF INTCON3bits.INT3IF
            #define RFIE INTCON3bits.INT3IE
            #define PHY_CS LATCbits.LATC2
            #define PHY_CS_TRIS TRISCbits.TRISC2
            #define RF_INT_PIN  PORTBbits.RB3
            #define RF_INT_TRIS TRISBbits.TRISB3
            
            #define nFSEL       LATEbits.LATE3             
            #define nFSEL_TRIS  TRISEbits.TRISE3
            #define FINT		PORTBbits.RB4               
            #define FINT_TRIS   TRISBbits.TRISB4  
        #else
            #define RFIF INTCON3bits.INT1IF
            #define RFIE INTCON3bits.INT1IE
            #define PHY_CS LATBbits.LATB3
            #define PHY_CS_TRIS TRISBbits.TRISB3
            #define RF_INT_PIN  PORTBbits.RB1
            #define RF_INT_TRIS TRISBbits.TRISB1       
        
            #define PHY_WAKE LATBbits.LATB4
            #define PHY_WAKE_TRIS TRISBbits.TRISB4
        #endif
    
        #define SPI_SDI     PORTCbits.RC4               
        #define SDI_TRIS    TRISCbits.TRISC4
        #define SPI_SDO     LATCbits.LATC5               
        #define SDO_TRIS    TRISCbits.TRISC5
        #define SPI_SCK     LATCbits.LATC3               
        #define SCK_TRIS    TRISCbits.TRISC3
    
        #define PHY_RESETn LATBbits.LATB5
        #define PHY_RESETn_TRIS TRISBbits.TRISB5

        
        #define PUSH_BUTTON_1   PORTBbits.RB0
        #define BUTTON_1_TRIS   TRISBbits.TRISB0
        #define PUSH_BUTTON_2   PORTAbits.RA5
        #define BUTTON_2_TRIS   TRISAbits.TRISA5
        
        
        #define LED_1       LATDbits.LATD7
        #define LED_1_TRIS  TRISDbits.TRISD7
        #define LED_2       LATDbits.LATD6
        #define LED_2_TRIS  TRISDbits.TRISD6
        
        #define TMRL TMR0L
    #endif


    #if defined(PICDEMZ)
        #define CLOCK_FREQ 16000000

        // Transceiver Configuration
        #define RFIF INTCONbits.INT0IF
        #define RFIE INTCONbits.INT0IE
        #define PHY_CS LATCbits.LATC0
        #define PHY_CS_TRIS TRISCbits.TRISC0
        #define PHY_RESETn LATCbits.LATC2
        #define PHY_RESETn_TRIS TRISCbits.TRISC2
        #define PHY_WAKE LATCbits.LATC1
        #define PHY_WAKE_TRIS TRISCbits.TRISC1
        
        #define PUSH_BUTTON_1 PORTBbits.RB5
        #define PUSH_BUTTON_2 PORTBbits.RB4
        #define LED_1 LATAbits.LATA0
        #define LED_2 LATAbits.LATA1
        
        #define BUTTON_1_TRIS TRISBbits.TRISB5
        #define BUTTON_2_TRIS TRISBbits.TRISB4
        #define LED_1_TRIS TRISAbits.TRISA0
        #define LED_2_TRIS TRISAbits.TRISA1
        
        #define RF_INT_PIN PORTBbits.RB0
        #define RF_INT_TRIS TRISBbits.TRISB0
        
        #define SPI_SDI     PORTCbits.RC4               
        #define SDI_TRIS    TRISCbits.TRISC4
        #define SPI_SDO     LATCbits.LATC5               
        #define SDO_TRIS    TRISCbits.TRISC5
        #define SPI_SCK     LATCbits.LATC3               
        #define SCK_TRIS    TRISCbits.TRISC3
        
        #define TMRL TMR0L
    #endif
    
    
    #if defined(EXPLORER16) 
    
        #if defined(__PIC32MX__)
            #define CLOCK_FREQ 64000000    
            #define RFIF IFS0bits.INT1IF
            #define RFIE IEC0bits.INT1IE
        #else
            #define CLOCK_FREQ 8000000
            #define RFIF IFS1bits.INT1IF
            #define RFIE IEC1bits.INT1IE
        #endif
    
        #if defined(__PIC24F__) || defined(__PIC32MX__)   
            #define RF_INT_PIN PORTEbits.RE8   
            #define RF_INT_TRIS TRISEbits.TRISE8 
        #elif defined(__dsPIC33F__) || defined(__PIC24H__)
            #define RF_INT_PIN PORTAbits.RA12
            #define RF_INT_TRIS TRISAbits.TRISA12
        #endif
        
        #define TMRL TMR2
        
        // Transceiver Configuration
        #define PHY_CS LATBbits.LATB2
        #define PHY_CS_TRIS TRISBbits.TRISB2
        #define PHY_RESETn LATGbits.LATG2
        #define PHY_RESETn_TRIS TRISGbits.TRISG2
        
        #define SPI_SDI     PORTFbits.RF7               
        #define SDI_TRIS    TRISFbits.TRISF7
        #define SPI_SDO     LATFbits.LATF8              
        #define SDO_TRIS    TRISFbits.TRISF8
        #define SPI_SCK     LATFbits.LATF6              
        #define SCK_TRIS    TRISFbits.TRISF6
        
        #if defined(MRF49XA)
            #define nFSEL       LATBbits.LATB1              
            #define nFSEL_TRIS  TRISBbits.TRISB1
            #define FINT		PORTEbits.RE9              
            #define FINT_TRIS   TRISEbits.TRISE9
        #else
            #define PHY_WAKE LATGbits.LATG3
            #define PHY_WAKE_TRIS TRISGbits.TRISG3
        #endif
        
        #define PUSH_BUTTON_1 PORTDbits.RD6
        #define PUSH_BUTTON_2 PORTDbits.RD7
        #define LED_1 LATAbits.LATA7
        #define LED_2 LATAbits.LATA6
        
        #define BUTTON_1_TRIS TRISDbits.TRISD6
        #define BUTTON_2_TRIS TRISDbits.TRISD7
        #define LED_1_TRIS TRISAbits.TRISA7
        #define LED_2_TRIS TRISAbits.TRISA6
        
        
        #define EE_nCS_TRIS     TRISFbits.TRISF1
        #define EE_nCS          LATFbits.LATF1
    
    #endif



#endif
