/*****************************************************************************
 *
 *              HardwareProfile.c -- Hardware Configuration
 *
 *****************************************************************************
 * FileName:        HardwareProfile.c
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
 * Copyright � 2007-2009 Microchip Technology Inc.  All rights reserved.
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
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED �AS IS� WITHOUT WARRANTY OF ANY 
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
#include "SystemProfile.h"
#include "Compiler.h"
#include "Common\SymbolTime.h"
#include "Common\LCDBlocking.h"
#include "Common\timer.h"

#if defined(__18F4620) 
        #pragma romdata CONFIG1H = 0x300001
        const rom unsigned char config1H = 0b00000110;      // HSPLL oscillator
    
        #pragma romdata CONFIG2L = 0x300002
        const rom unsigned char config2L = 0b00011111;      // Brown-out Reset Enabled in hardware @ 2.0V, PWRTEN disabled
    
        #pragma romdata CONFIG2H = 0x300003
        const rom unsigned char config2H = 0b00010110; //0b00001010;      // HW WD disabled, 1:32 prescaler
    
        #pragma romdata CONFIG3H = 0x300005
        const rom unsigned char config3H = 0b10000000;      // PORTB digital on RESET
    
        #pragma romdata CONFIG4L = 0x300006
        const rom unsigned char config4L = 0b11000001;      // DEBUG disabled,
                                                            // XINST enabled
                                                            // LVP disabled
                                                            // STVREN enabled
#elif defined(__18F2520)
        #pragma romdata CONFIG1H = 0x300001
        const rom unsigned char config1H = 0b00001000;      // use internal oscillator
        
        #pragma romdata CONFIG2L = 0x300002
        const rom unsigned char config2L = 0b00011111;      // Brown-out Reset Enabled in hardware @ 2.0V, PWRTEN disabled
        
        #pragma romdata CONFIG2H = 0x300003
        const rom unsigned char config2H = 0b00010110; //0b00001010;      // HW WD disabled, 1:32 prescaler
        
        #pragma romdata CONFIG3H = 0x300005
        const rom unsigned char config3H = 0b00000011;      // PORTB digital as PORT
        
        #pragma romdata CONFIG4L = 0x300006
        const rom unsigned char config4L = 0b10000001;      // DEBUG disabled,
                                                            // XINST disabled
                                                            // LVP disabled
                                                            // STVREN enabled
#elif defined(__18F87J11)

    #pragma config FOSC = HSPLL, WDTEN = OFF, WDTPS = 2048, XINST = ON                                                       
                                                             
#elif defined(__PIC24F__)
	// Explorer 16 board
	_CONFIG2(FNOSC_PRI & POSCMOD_XT)		// Primary XT OSC with 4x PLL
	_CONFIG1(JTAGEN_OFF & FWDTEN_OFF & WDTPS_PS2048)		// JTAG off, watchdog timer off
#elif defined(__dsPIC33F__) || defined(__PIC24H__)
	// Explorer 16 board
	_FOSCSEL(FNOSC_PRI);    //primary osc
	_FOSC(OSCIOFNC_OFF & POSCMD_XT)	// XT Osc
	_FWDT(FWDTEN_OFF & WDTPOST_PS2048)				// Disable Watchdog timer
	// JTAG should be disabled as well
#elif defined(__PIC32MX__)
    #define PIC32MX_SPI1_SDO_SCK_MASK_VALUE      (0x00000140)
    #define PIC32MX_SPI1_SDI_MASK_VALUE          (0x00000080)
    #define PIC32MX_INT1_MASK_VALUE              (0x00000008)
    /* MAX SPI CLOCK FREQ SUPPORTED FOR MIWI TRANSCIEVER */
    #define MAX_SPI_CLK_FREQ_FOR_P2P             (2000000)
    
    #define        Sleep()      PowerSaveSleep()
    
    #pragma config FPLLMUL = MUL_16, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FWDTEN = OFF
    #pragma config POSCMOD = XT, FNOSC = PRIPLL, FPBDIV = DIV_4, WDTPS = PS8192

#endif

#define DEBOUNCE_TIME 0x00008FFF

BOOL PUSH_BUTTON_1_pressed = FALSE;
BOOL PUSH_BUTTON_2_pressed = FALSE;

BOOL PUSH_BUTTON_pressed;
MIWITICK PUSH_BUTTON_press_time;

/*********************************************************************
 * Function:        void BoardInit( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    Board is initialized for P2P usage
 *
 * Overview:        This function configures the board 
 *
 * Note:            This routine needs to be called before the function 
 *                  to initialize P2P stack or any other function that
 *                  operates on the stack
 ********************************************************************/
void BoardInit(void)
{
    #if defined(PICDEMZ)
        WDTCONbits.SWDTEN = 0; //disable WDT
        
        // Switches S2 and S3 are on RB5 and RB4 respectively. We want interrupt-on-change
        INTCON = 0x00;
        
        // There is no external pull-up resistors on S2 and S3. We will use internal pull-ups.
        // The MRF24J40 is using INT0 for interrupts
        // Enable PORTB internal pullups
        INTCON2 = 0x00;
        INTCON3 = 0x00;
    
        // Make PORTB as input - this is the RESET default
        TRISB = 0xff;
    
        // Set PORTC control signal direction and initial states
        // disable chip select
        LATC = 0xfd;
    
        // Set the SPI module for use by Stack
        TRISC = 0xD0;
    
        // Set the SPI module
        #if defined(HARDWARE_SPI)
            SSPSTAT = 0xC0;
            SSPCON1 = 0x20;
        #endif
        
        // D1 and D2 are on RA0 and RA1 respectively, and CS of TC77 is on RA2.
        // Make PORTA as digital I/O.
        // The TC77 temp sensor CS is on RA2.
        ADCON1 = 0x0F;
    
        // Deselect TC77 (RA2)
        LATA = 0x04;
    
        // Make RA0, RA1, RA2 and RA4 as outputs.
        TRISA = 0xF8;
        
        PHY_CS = 1;             //deselect the MRF24J40
        PHY_CS_TRIS = 0;        //make chip select an output   
        
        RFIF = 0;               //clear the interrupt flag
    
        RCONbits.IPEN = 1;
        
        INTCON2bits.INTEDG0 = 0;
        
    #elif defined(EXPLORER16)
        
        // set I/O ports
        BUTTON_1_TRIS = 1;
        BUTTON_2_TRIS = 1;
        LED_1_TRIS = 0;
        LED_2_TRIS = 0;
        
        PHY_CS_TRIS = 0;
        PHY_CS = 1;
        PHY_RESETn_TRIS = 0;
        PHY_RESETn = 1;
        RF_INT_TRIS = 1;
        
        SDI_TRIS = 1;
        SDO_TRIS = 0;
        SCK_TRIS = 0;
        SPI_SDO = 0;        
        SPI_SCK = 0;             
        
        #if defined(MRF49XA)
            nFSEL_TRIS = 0;
            FINT_TRIS = 1;
            
            nFSEL = 1;          // nFSEL inactive
        #else
            PHY_WAKE_TRIS = 0;
            PHY_WAKE = 1;
        #endif
        

        #ifdef __PIC32MX__
           
           #if defined(HARDWARE_SPI)
           unsigned int pbFreq;
    
           /* Clear SPI1CON register */
           SPI1CONCLR = 0xFFFFFFFF;
    
           /* Enable SPI1, Set to Master Mode & Set CKE bit : Serial output data changes on transition 
              from active clock state to Idle clock state */
           SPI1CON = 0x00008120;
           
           /* Peripheral Bus Frequency = System Clock / PB Divider */
           pbFreq = (DWORD)CLOCK_FREQ / (1 << mOSCGetPBDIV() );
    
           /* PB Frequency can be maximum 40 MHz */
           if( pbFreq > ( 2 * MAX_SPI_CLK_FREQ_FOR_P2P) )
           {
               {
                   unsigned int SPI_Clk_Freq;
                   
                   unsigned char SPI_Brg = 1;
                   
                   /* Continue the loop till you find SPI Baud Rate Register Value */
                   while(1)
                   {
                      /* SPI Clock Calculation as per PIC32 Manual */
                      SPI_Clk_Freq = pbFreq / ( 2 * ( SPI_Brg + 1) );
                      
                      if( SPI_Clk_Freq <= MAX_SPI_CLK_FREQ_FOR_P2P )
                      {
                          break;
                      }
                   
                      SPI_Brg++;
                   }
           
                  mSpiChnSetBrg(1,SPI_Brg);
               
               }
           }
           else
           {
               /* Set SPI1 Baud Rate */
               mSpiChnSetBrg(1,0);
           }
    
           /* Set the Port Directions of SDO, SDI, Clock & Slave Select Signal */
           mPORTFSetPinsDigitalOut(PIC32MX_SPI1_SDO_SCK_MASK_VALUE);
           mPORTFSetPinsDigitalIn(PIC32MX_SPI1_SDI_MASK_VALUE);
           #endif
           
           /* Set the INT1 port pin to input */
           mPORTESetPinsDigitalIn(PIC32MX_INT1_MASK_VALUE);
    
           /* Set the Interrupt Priority */
           mINT1SetIntPriority(4);
    
           /* Set Interrupt Subpriority Bits for INT2 */
           mINT1SetIntSubPriority(2);
    
           /* Set INT1 to falling edge */
           mINT1SetEdgeMode(0);

           /* Set the Interrupt Priority for Push Button 2 & 3 as '5' */
           mCNSetIntPriority(5);
       
           /* Set the Interrupt Priority for Push Button 2 & 3 as '1' */
           mCNSetIntSubPriority(1);
           
           /* Enable INT1 */
           mINT1IntEnable(1);
           
           /* Enable Multi Vectored Interrupts */
           INTEnableSystemMultiVectoredInt();    
           
        #else

            #if defined(HARDWARE_SPI)
                SPI1CON1 = 0b0000000100111110;
                SPI1STAT = 0x8000;
            #endif
            
            CNEN1bits.CN15IE = 1;
            CNEN2bits.CN16IE = 1;
            INTCON2bits.INT1EP = 1;
        #endif
        
        // Make RB0 as Digital input
        AD1PCFGbits.PCFG2 = 1;
        
        #if defined(ENABLE_EEPROM)
        
            EE_nCS_TRIS = 0;
            EE_nCS = 1;
        
        #endif
        
        RFIF = 0;
        
        if( RF_INT_PIN == 0 )
        {
            RFIF = 1;
        }
        
        LCDInit();
        
    #elif defined(PIC18_EXPLORER)
        // primary external oscillator
        OSCCON = 0x70; 
        //OSCTUNEbits.PLLEN = 1;      
        
        // set up the analogue port
        WDTCONbits.ADSHR = 1;
        ANCON0 = 0b11011111;        // all digital pin
        ANCON1 = 0xFF;
        WDTCONbits.ADSHR = 0;
        
        TRISA = 0xFF;
        TRISB = 0xFF;
        TRISC = 0xFF;
        TRISD = 0x00;
        
        LATA = 0;
        LATB = 0;
        LATC = 0;
        LATD = 0;
        
        // set I/O ports
        BUTTON_1_TRIS = 1;
        BUTTON_2_TRIS = 1;
        LED_1_TRIS = 0;
        LED_2_TRIS = 0;
        
        PHY_CS_TRIS = 0;
        PHY_CS = 1;
        PHY_RESETn_TRIS = 0;
        PHY_RESETn = 1;
        RF_INT_TRIS = 1;
        
        SDI_TRIS = 1;
        SDO_TRIS = 0;
        SCK_TRIS = 0;
        
        
        #if defined(HARDWARE_SPI)
            SSP1STAT = 0xC0;
            SSP1CON1 = 0x21;
            
        #else
            SPI_SDO = 0;        
            SPI_SCK = 0;    
        #endif   
        
        #if defined(MRF49XA)
            nFSEL_TRIS = 0;
            FINT_TRIS = 1;
            nFSEL = 1;          // nFSEL inactive
        #else
            PHY_WAKE_TRIS = 0;
            PHY_WAKE = 1;
        #endif
        
        INTCONbits.GIEH = 1;
        
        // enable RF INT
        INTCON2bits.INTEDG3 = 0;
        RFIF = 0;
        RFIE = 1;
        
        LCDInit();
        
    #else
        #error "Unknown demo board.  Please properly initialize the part for the board."
    #endif
}


/*********************************************************************
 * Function:        void LCDTRXCount(BYTE txCount, BYTE rxCount)
 *
 * PreCondition:    LCD has been initialized
 *
 * Input:           txCount - the total number of transmitted messages
 *                  rxCount - the total number of received messages
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function display the total numbers of TX and
 *                  RX messages on the LCD, if applicable.
 *
 * Note:            This routine is only effective if Explorer16 or
 *                  PIC18 Explorer demo boards are used
 ********************************************************************/
void LCDTRXCount(BYTE txCount, BYTE rxCount)
{
    #if defined(EXPLORER16) || defined(PIC18_EXPLORER)
        LCDErase();
        #if defined(PIC18_EXPLORER)
            sprintf((char *)LCDText, (far rom char*)"TX Messages: %d", txCount);
            sprintf((char *)&(LCDText[16]), (far rom char*)"RX Messages: %d", rxCount);
        #else
            sprintf((char *)LCDText, (const char*)"TX Messages: %d", txCount);
            sprintf((char *)&(LCDText[16]), (const char*)"RX Messages: %d", rxCount);
        #endif
        LCDUpdate();    
    #endif
}


/*********************************************************************
 * Function:        void LCDDisplay(char *text, BYTE value, BOOL delay)
 *
 * PreCondition:    LCD has been initialized
 *
 * Input:           text - text message to be displayed on LCD
 *                  value - the text message allows up to one byte 
 *                          of variable in the message
 *                  delay - whether need to display the message for
 *                          2 second without change
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function display the text message on the LCD, 
 *                  including up to one BYTE variable, if applicable.
 *
 * Note:            This routine is only effective if Explorer16 or
 *                  PIC18 Explorer demo boards are used
 ********************************************************************/
void LCDDisplay(char *text, BYTE value, BOOL delay)
{
    #if defined(EXPLORER16) || defined(PIC18_EXPLORER)
        LCDErase();
        #if defined(PIC18_EXPLORER)
            sprintf((char *)LCDText, (far rom char*)text, value); 
        #elif defined(EXPLORER16)
            sprintf((char *)LCDText, (const char*)text, value); 
        #endif
        LCDUpdate();
        
        // display the message for 2 seconds
        if( delay )
        {
            BYTE i;
            for(i = 0; i < 8; i++)
            {
                delay_ms(250);
            }
        }
    #endif
}


/*********************************************************************
 * Function:        BYTE ButtonPressed(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Byte to indicate which button has been pressed. 
 *                  Return 0 if no button pressed.
 *
 * Side Effects:    
 *
 * Overview:        This function check if any button has been pressed
 *
 * Note:            
 ********************************************************************/
BYTE ButtonPressed(void)
{
    #if (defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24H__) || defined(__PIC32MX__)) && defined(ENABLE_SLEEP)
        while(PUSH_BUTTON_1 == 0);
        while(PUSH_BUTTON_2 == 0);
        if( PUSH_BUTTON_1_pressed )
        {
            PUSH_BUTTON_1_pressed = FALSE;
            PUSH_BUTTON_2_pressed = FALSE;
            return 1;
        }
        else if( PUSH_BUTTON_2_pressed )
        {
            PUSH_BUTTON_2_pressed = FALSE;
            return 2;
        }
    #else
        MIWITICK tickDifference;    
    
        if(PUSH_BUTTON_1 == 0u)
        {
            #ifdef ENABLE_SLEEP
                while(PUSH_BUTTON_1 == 0u );
                return 1;
            #else
                //if the button was previously not pressed
                if(PUSH_BUTTON_pressed == FALSE)
                {
                    PUSH_BUTTON_pressed = TRUE;
                    PUSH_BUTTON_press_time = MiWiTickGet();
                    return 1;
                }
            #endif
        }
        else if(PUSH_BUTTON_2 == 0u)
        {
            #ifdef ENABLE_SLEEP
                while(PUSH_BUTTON_2 == 0u);
                return 2;
            #else
                //if the button was previously not pressed
                if(PUSH_BUTTON_pressed == FALSE)
                {
                    PUSH_BUTTON_pressed = TRUE;
                    PUSH_BUTTON_press_time = MiWiTickGet();
                    return 2;
                }
            #endif
        } 
        #ifndef ENABLE_SLEEP
            else
            {
                //get the current time
                MIWITICK t = MiWiTickGet();
                
                //if the button has been released long enough
                tickDifference.Val = TickGetDiff(t,PUSH_BUTTON_press_time);
                
                //then we can mark it as not pressed
                if(tickDifference.Val > DEBOUNCE_TIME)
                {
                    PUSH_BUTTON_pressed = FALSE;
                }
            }
        #endif
    #endif
    
    return 0;
}

/*********************************************************************
 * Function:        void Enable_PB_1_2_Interrupts()
 *
 * PreCondition:    None
 * 
 * Input:           None
 * 
 * Output:          None
 * 
 * Side Effects:    None
 * 
 * Overview:        Configure/Enable PB 1 & 2 interrupts.
 *  
 * Note:            None
 ********************************************************************/

#ifdef __PIC32MX__

void Enable_PB_1_2_Interrupts(void)
{
    unsigned int value;

    CNCON = 0x8000;  // Enable Change Notice module

    /* Configure Change Notice Registers for Push Button 1 & 2. These buttons can be 
       used to wake the controller from sleep state. */
    CNEN  = 0x00018000; // Enable CN15 and CN16 pins

    /* Read port to clear mismatch on change notice pins */
    value = PORTD;

    mCNClearIntFlag(); // Clear the CN interrupt flag status bit

    mCNIntEnable(1); // Enable Change Notice interrupts
}

#endif

/*********************************************************************
 * Function:        void _CN_Interrupt_ISR(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None.
 *
 * Overview:        This is the interrupt handler for push button 1 & 2 to 
 *                  wake up the controller in Sleep State.
 ********************************************************************/
#if defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24H__)
    void _ISRFAST __attribute__((interrupt, auto_psv)) _CNInterrupt(void) 
    {
        IFS1bits.CNIF = 0;
        IEC1bits.CNIE = 0;          // disable pin change notification interrupt
        #ifdef ENABLE_SLEEP
            if( PUSH_BUTTON_1 == 0 )
            {
                PUSH_BUTTON_1_pressed = TRUE;
            }
            else if( PUSH_BUTTON_2 == 0 )
            {
                PUSH_BUTTON_2_pressed = TRUE;
            }
        #endif
    }
#elif defined(__PIC32MX__)

void __ISR(_CHANGE_NOTICE_VECTOR, ipl5) _CN_Interrupt_ISR(void)
{
    /* Disable Watchdog Timer to make sure that reset doesn't happen */
    DisableWDT();
    
    {
        unsigned int value1;

        value1 = PORTD; // Read PORTD to clear CN15/16 mismatch condition

        if(value1 & 0x80 )
        {
           PUSH_BUTTON_1_pressed = TRUE;
        }
        else
        {
           PUSH_BUTTON_2_pressed = TRUE;
        }
    }

    mCNIntEnable(0); // Disable the CN Interrupt 

    CNCON = 0x0000; // Disable CN Module 

    CNEN = 0x0000;
    
    mCNClearIntFlag(); // Clear the CN interrupt flag status bit

}

#endif

