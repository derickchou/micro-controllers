/*****************************************************************************
 *
 * Basic access to parallel FLASH SST39VF040.
 *
 *****************************************************************************
 * FileName:        SST39VF040.c
 * Dependencies:    SST39VF040.h
 * Processor:       PIC24, PIC32
 * Compiler:       	MPLAB C30 V3.00
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
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Anton Alkhimenok		01/14/08	...
 *****************************************************************************/

// Standard includes
#include "HardwareProfile.h"
#include "Graphics\Graphics.h"
#include "SST39VF040.h"

#if (GRAPHICS_PICTAIL_VERSION < 3)

// Internal pointer to address being written
DWORD dwWriteAddr;

#define SST39PMPWaitBusy()  while(PMMODEbits.BUSY);

/////////////////////////////////////////////////////////////////////////
// Local prototypes
/////////////////////////////////////////////////////////////////////////
void SST39SetAddress(DWORD address);

/************************************************************************
* Function: SST39Init                                                  
*                                                                       
* Overview: this function initializes IOs and PMP
*                                                                       
* Input: none                                                          
*                                                                       
* Output: none
*                                                                       
************************************************************************/
void SST39Init()
{   
   // disable FLASH
    SST39_CS_LAT = 1;
   // set FLASH CS pin as output
    SST39_CS_TRIS = 0;
 
    // upper address bits
    SST39_A18_LAT = 0;
    SST39_A17_LAT = 0;
    SST39_A16_LAT = 0;
    SST39_A18_TRIS = 0;
    SST39_A17_TRIS = 0;
    SST39_A16_TRIS = 0;
    
    // PMP Setup is performed in main.c because the display is
    // also connected to this peripheral
}

/************************************************************************
* Function: void SST39SetAddress(DWORD address)
*                                                                       
* Overview: this function sets address
*                                                                       
* Input: address                                                     
*                                                                       
* Output: none
*                                                                       
************************************************************************/
void SST39SetAddress(DWORD address){

    // mask upper address bits
    if(((DWORD_VAL)address).w[1]&0x01)
        SST39_A16_LAT = 1;
    else
        SST39_A16_LAT = 0;

    if(((DWORD_VAL)address).w[1]&0x02)
        SST39_A17_LAT = 1;
    else
        SST39_A17_LAT = 0;

    if(((DWORD_VAL)address).w[1]&0x04)
        SST39_A18_LAT = 1;
    else
        SST39_A18_LAT = 0;

    // low address part
    PMADDR=((DWORD_VAL)address).w[0];
}

/************************************************************************
* Function: BYTE SST39WriteByte(DWORD address, BYTE data)
*                                                                       
* Overview: this function writes one byte
*                                                                       
* Input: address and byte to be written
*                                                                       
* Output: non-zero if it's successful
*
* Notes: none
*                                                                       
************************************************************************/
BYTE SST39WriteByte(DWORD address, BYTE data)
{
	BYTE d1, d2;
	
    SST39SetAddress(0x00005555);
    SST39_CS_LAT = 0;
    PMDIN1 = 0xAA;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;

    SST39SetAddress(0x00002AAA);
    SST39_CS_LAT = 0;
    PMDIN1 = 0x55;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;
    
    SST39SetAddress(0x00005555);
    SST39_CS_LAT = 0;
    PMDIN1 = 0xA0;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;
    
    SST39SetAddress(address);
    SST39_CS_LAT = 0;
    PMDIN1 = data;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;
    
	dwWriteAddr++;
    SST39WaitProgram();

	d1 = SST39ReadByte(address);
	d2 = SST39ReadByte(address);
	if ((d1 == data) && (d2 == data))
		return 1;
 
    return 0; // failed
}

/************************************************************************
* Function: BYTE SST39WriteArray(DWORD address, BYTE* pData, nCount)
*                                                                       
* Overview: this function writes data array at the address specified
*                                                                       
* Input: flash memory address, pointer to the data array, data number
*                                                                       
* Output: return 1 if the operation was successfull
*
* Notes: none
*                                                                       
************************************************************************/
BYTE SST39WriteArray(DWORD address, BYTE* pData, WORD nCount)
{
	WORD    counter;
	BYTE*   pD;
	DWORD   addr;

    pD = pData;
    addr = address;

    // write
    for(counter=0; counter<nCount; counter++){
       while(0 == SST39WriteByte(addr,*pD));
       addr++; pD++;
    }

    pD = pData;
    addr = address;

    // verify
    for(counter=0; counter<nCount; counter++){
        if(*pD++ != SST39ReadByte(addr++))
            return 0;
    }

    return 1;
}

/************************************************************************
* Function: void SST39ReadArray(DWORD address, BYTE* pData, nCount)
*                                                                       
* Overview: this function reads data array from the address specified
*                                                                       
* Input: flash memory address, pointer to the buffer receiving data, data number
*                                                                       
* Output: none
*                                                                       
************************************************************************/

void SST39ReadArray(DWORD address, BYTE* pData, WORD nCount)
{
	WORD counter;
	BYTE temp;

    SST39SetAddress(address); 
    // enable
    SST39_CS_LAT = 0;

    // run dummy cycle to get data
    temp = PMDIN1;

	for(counter=0; counter<nCount; counter++)
    {
        SST39PMPWaitBusy();

  		if(PMADDR == 0){
			// set upper address bits
            address+=0x010000;
    
			if(((DWORD_VAL)address).v[2]&0x01)
				SST39_A16_LAT = 1;
			else
				SST39_A16_LAT = 0;

			if(((DWORD_VAL)address).v[2]&0x02)
				SST39_A17_LAT = 1;
			else
				SST39_A17_LAT = 0;

			if(((DWORD_VAL)address).v[2]&0x04)
				SST39_A18_LAT = 1;
			else
				SST39_A18_LAT = 0;

		}
        
        *pData++ = PMDIN1;

    }
    SST39PMPWaitBusy();
    // disable
    SST39_CS_LAT = 1;

}

/************************************************************************
* Function: BYTE SST39ReadByte(DWORD address)
*                                                                       
* Overview: this function reads a byte from address specified         
*                                                                       
* Input: address                                                     
*                                                                       
* Output: data read
*                                                                       
************************************************************************/
BYTE SST39ReadByte(DWORD address)
{
	BYTE temp;

    SST39SetAddress(address); 

    // enable
    SST39_CS_LAT = 0;

    temp = PMDIN1;
    // wait for reading cycle is completed
    SST39PMPWaitBusy();
    // disable
    SST39_CS_LAT = 1;
    // read result    
    temp = PMDIN1;
    // wait for dummy reading cycle is completed
    SST39PMPWaitBusy();

    return temp;
}

/************************************************************************
* Function: WORD SST39ReadWord(DWORD address)             
*                                                                       
* Overview: this function reads a 16-bit word from address specified         
*                                                                       
* Input: address                                                     
*                                                                       
* Output: data read
*                                                                       
************************************************************************/
WORD SST39ReadWord(DWORD address)
{
	WORD_VAL temp;

    temp.v[0] = SST39ReadByte(address);
    temp.v[1] = SST39ReadByte(address+1);

    return temp.Val;
}

/************************************************************************
* Function: void SST39WaitProgram()
*                                                                       
* Overview: this function waits for program/erase operation completion
*                                                                       
* Input: none                                                          
*                                                                       
* Output: 1 if it was successfull 0 - if not
*                                                                       
************************************************************************/
void SST39WaitProgram()
{
	SST39STATUS stat;
	SST39STATUS prevStat;

    // enable
    SST39_CS_LAT = 0;

    prevStat.Val = PMDIN1;
    SST39PMPWaitBusy();

    prevStat.Val = PMDIN1;
    SST39PMPWaitBusy();

    stat.Val = PMDIN1;
    SST39PMPWaitBusy();


    while(stat.Bits.TOGGLE != prevStat.Bits.TOGGLE){

        prevStat.Val = stat.Val;
        stat.Val = PMDIN1;
        SST39PMPWaitBusy();
        
    }// end of while

    // disable
    SST39_CS_LAT = 1;
}

/************************************************************************
* Function: void SST39ChipErase(void)                                 
*
* Overview: erase whole chip
*
* Input: none
*            
* Output: none
*
************************************************************************/
void SST39ChipErase(void)
{
    SST39SetAddress(0x00005555);
    SST39_CS_LAT = 0;
    PMDIN1=0xaa;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;

    SST39SetAddress(0x00002AAA);
    SST39_CS_LAT = 0;
    PMDIN1=0x55;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;
    
    SST39SetAddress(0x00005555);
    SST39_CS_LAT = 0;
    PMDIN1=0x80;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;

    SST39SetAddress(0x00005555);
    SST39_CS_LAT = 0;
    PMDIN1=0xaa;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;

    SST39SetAddress(0x00002AAA);
    SST39_CS_LAT = 0;
    PMDIN1=0x55;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;
    
    SST39SetAddress(0x00005555);
    SST39_CS_LAT = 0;
    PMDIN1=0x10;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;

    SST39WaitProgram();
}

/************************************************************************
* Function: void SST39SectorErase(DWORD address)                                 
*
* Overview: erase whole chip
*
* Input: none
*            
* Output: none
*
************************************************************************/
void SST39SectorErase(DWORD address)
{
    SST39SetAddress(0x00005555);
    SST39_CS_LAT = 0;
    PMDIN1=0xaa;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;

    SST39SetAddress(0x00002AAA);
    SST39_CS_LAT = 0;
    PMDIN1=0x55;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;
    
    SST39SetAddress(0x00005555);
    SST39_CS_LAT = 0;
    PMDIN1=0x80;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;

    SST39SetAddress(0x00005555);
    SST39_CS_LAT = 0;
    PMDIN1=0xaa;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;

    SST39SetAddress(0x00002AAA);
    SST39_CS_LAT = 0;
    PMDIN1=0x55;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;
    
    // set the sector address
    SST39SetAddress(address);
    SST39_CS_LAT = 0;
    PMDIN1=0x30;
    SST39PMPWaitBusy();
    SST39_CS_LAT = 1;

    SST39WaitProgram();
}

/************************************************************************
* Function: void SST39WriteWord(DWORD address, BYTE data)                                           
*                                                                       
* Overview: this function writes a 16-bit word to the address specified
*                                                                       
* Input: data to be written and address
*                                                                       
* Output: none                                                         
*                                                                       
************************************************************************/
void SST39WriteWord(DWORD address, WORD data)
{
    SST39WriteByte(address, ((WORD_VAL)data).v[0]);
    SST39WriteByte(address+1, ((WORD_VAL)data).v[1]);
}

/***************************************************************************
 * Start a write operation by recording the start address
 ***************************************************************************/
void SST39BeginWrite(DWORD dwAddr)
{
    dwWriteAddr = dwAddr;
}

/******************************************************************
 *  SST39WriteIncrementalArray(BYTE* vData, WORD wLen);
 * write an array erasing the sector if required
 *****************************************************************/
void SST39WriteIncrementalArray(BYTE* vData, WORD wLen)
{
	while (wLen > 0) {
		// wait for any existing transaction to finish
		SST39WaitProgram();
		
		// clear the sector if on a sector boundary
		if ((dwWriteAddr & SST39_FLASH_SECTOR_MASK) == 0) {
			SST39SectorErase(dwWriteAddr);
			SST39WaitProgram();
		}
			
		// write the byte of data
		SST39WriteByte(dwWriteAddr, *vData);
		// dwWriteAddr is incremented in WriteByte
		vData++;
		wLen--;
	}
}

#endif
