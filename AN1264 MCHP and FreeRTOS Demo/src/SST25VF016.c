/*****************************************************************************
 *
 * Basic access to SPI Flash SST25VF016 located on 
 * Graphics LCD Controller PICtail Plus SSD1926 Board.
 *
 *****************************************************************************
 * FileName:        SST25VF016.c
 * Dependencies:    SST25VF016.h
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
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Anton Alkhimenok		01/07/09	...
 *****************************************************************************/

// Standard includes
#include "HardwareProfile.h"
#include "Graphics\Graphics.h"
#include "SST25VF016.h"

// semaphores are used by this module to guard access to the EEPROM
// so we add the FreeRTOS includes to support this
#include "FreeRTOS.h"
#include "Semphr.h"
#include "taskUART.h"

#if (GRAPHICS_PICTAIL_VERSION == 3)

// Internal pointer to address being written
DWORD dwWriteAddr;

///////////////////////////////////////////////////////////////////
// Semaphores used to control access to SPI2 and FLASH
extern xSemaphoreHandle SPI2Semaphore;

/************************************************************************
* Function: SST25Init                                                  
*                                                                       
* Overview: this function setup SPI and IOs connected to SST25
*                                                                       
* Input: none                                                          
*                                                                       
* Output: none
*                                                                       
************************************************************************/
void SST25Init()
{	
    SST25ResetWriteProtection();
}

/************************************************************************
* Function SPI2Put(BYTE data)                                                       
*                                                                       
* Overview:  this function sends a byte                     
*                                                                       
* Input: byte to be sent
*                                                                       
* Output: none
*                                                                       
************************************************************************/           
void SPI2Put(BYTE data)
{
#ifdef __PIC32MX
    // Wait for free buffer
    while(!SPI2STATbits.SPITBE);
    SPI2BUF = data;
    // Wait for data byte
    while(!SPI2STATbits.SPIRBF);
#else
    // Wait for free buffer
    while(SPI2STATbits.SPITBF);
    SPI2BUF = data;
    // Wait for data byte
    while(!SPI2STATbits.SPIRBF);
#endif
}

/************************************************************************
* Macros SPI2Get()                                                       
*                                                                       
* Overview:  this macros gets a byte from SPI                      
*                                                                       
* Input: none
*                                                                       
* Output: none
*                                                                       
************************************************************************/           
#define SPI2Get() SPI2BUF

/************************************************************************
* Function: void SST25WriteByte(DWORD address, BYTE data)                                           
*                                                                       
* Overview: this function writes a byte to the address specified
*                                                                       
* Input: data to be written and address
*                                                                       
* Output: none                                 
*                                                                       
************************************************************************/
void SST25WriteByte(DWORD address, BYTE data)
{
    SST25WriteEnable();

	xSemaphoreTake(SPI2Semaphore, portMAX_DELAY);	
    SST25CSLow();

    SPI2Put(SST25_CMD_WRITE);
    SPI2Get();

    SPI2Put(((DWORD_VAL)address).v[2]);
    SPI2Get();

    SPI2Put(((DWORD_VAL)address).v[1]);
    SPI2Get();

    SPI2Put(((DWORD_VAL)address).v[0]);
    SPI2Get();

    SPI2Put(data);
    SPI2Get();

	dwWriteAddr++;
	
    SST25CSHigh();
    xSemaphoreGive(SPI2Semaphore);

    // Wait for write end
    while(SST25IsWriteBusy());
    
}

/************************************************************************
* Function: BYTE SST25ReadByte(DWORD address)             
*                                                                       
* Overview: this function reads a byte from the address specified         
*                                                                       
* Input: address                                                     
*                                                                       
* Output: data read
*                                                                       
************************************************************************/
BYTE SST25ReadByte(DWORD address)
{
	BYTE temp;

	xSemaphoreTake(SPI2Semaphore, portMAX_DELAY);

    SST25CSLow();

    SPI2Put(SST25_CMD_READ);
    SPI2Get();

    SPI2Put(((DWORD_VAL)address).v[2]);
    SPI2Get();

    SPI2Put(((DWORD_VAL)address).v[1]);
    SPI2Get();

    SPI2Put(((DWORD_VAL)address).v[0]);
    SPI2Get();

    SPI2Put(0);
    temp = SPI2Get();

    SST25CSHigh();    
    xSemaphoreGive(SPI2Semaphore);
    
    return temp;
}

/************************************************************************
* Function: void SST25WriteWord(DWORD address, WORD data)                                           
*                                                                       
* Overview: this function writes a 16-bit word to the address specified
*                                                                       
* Input: data to be written and address
*                                                                       
* Output: none                                                         
*                                                                       
************************************************************************/
void SST25WriteWord(DWORD address, WORD data)
{
    SST25WriteByte(address, ((WORD_VAL)data).v[0]);
    SST25WriteByte(address + 1, ((WORD_VAL)data).v[1]);
}

/************************************************************************
* Function: WORD SST25ReadWord(DWORD address)             
*                                                                       
* Overview: this function reads a 16-bit word from the address specified         
*                                                                       
* Input: address                                                     
*                                                                       
* Output: data read
*                                                                       
************************************************************************/
WORD SST25ReadWord(DWORD address){
	WORD_VAL temp;

    temp.v[0] = SST25ReadByte(address);
    temp.v[1] = SST25ReadByte(address+1);

    return temp.Val;
}

/************************************************************************
* Function: SST25WriteEnable()                                         
*                                                                       
* Overview: this function allows write/erase SST25. Must be called  
* before every write/erase command.                                         
*                                                                       
* Input: none                                                          
*                                                                       
* Output: none                                 
*                                                                       
************************************************************************/
void SST25WriteEnable(){
	xSemaphoreTake(SPI2Semaphore, portMAX_DELAY);
    SST25CSLow();
    SPI2Put(SST25_CMD_WREN);
    SPI2Get();
    SST25CSHigh();
    xSemaphoreGive(SPI2Semaphore);
}

/************************************************************************
* Function: BYTE SST25IsWriteBusy(void)  
*                                                                       
* Overview: this function reads status register and chek BUSY bit for write operation
*                                                                       
* Input: none                                                          
*                                                                       
* Output: non zero if busy
*                                                                       
************************************************************************/
BYTE SST25IsWriteBusy(void)
{
	BYTE temp;

	xSemaphoreTake(SPI2Semaphore, portMAX_DELAY);
    SST25CSLow();
    SPI2Put(SST25_CMD_RDSR);
    SPI2Get();

    SPI2Put(0);
    temp = SPI2Get();
    SST25CSHigh();
	xSemaphoreGive(SPI2Semaphore);

    return (temp & 0x01);
}

/************************************************************************
* Function: BYTE SST25WriteArray(DWORD address, BYTE* pData, nCount)
*                                                                       
* Overview: this function writes a data array at the address specified
*                                                                       
* Input: flash memory address, pointer to the data array, data number
*                                                                       
* Output: return 1 if the operation was successfull
*                                                                     
************************************************************************/
BYTE SST25WriteArray(DWORD address, BYTE* pData, WORD nCount)
{
	DWORD     addr;
	BYTE*     pD;
	WORD      counter;


    addr = address;
    pD   = pData;

    // WRITE
    for(counter=0; counter<nCount; counter++)
    {
        SST25WriteByte(*pD++, addr++);
    }

    // VERIFY
    for(counter=0; counter<nCount; counter++)
    {              
        if(*pData != SST25ReadByte(address))
            return 0;
        pData++;
        address++;
    }

    return 1;
}

/************************************************************************
* Function: void SST25ReadArray(DWORD address, BYTE* pData, nCount)
*                                                                       
* Overview: this function reads data into buffer specified
*                                                                       
* Input: flash memory address, pointer to the data buffer, data number
*                                                                       
************************************************************************/
void SST25ReadArray(DWORD address, BYTE* pData, WORD nCount)
{
	xSemaphoreTake(SPI2Semaphore, portMAX_DELAY);
    SST25CSLow();

    SPI2Put(SST25_CMD_READ);
    SPI2Get();

    SPI2Put(((DWORD_VAL)address).v[2]);
    SPI2Get();

    SPI2Put(((DWORD_VAL)address).v[1]);
    SPI2Get();

    SPI2Put(((DWORD_VAL)address).v[0]);
    SPI2Get();

    while(nCount--){
        SPI2Put(0);
        *pData++ = SPI2Get();
    }

    SST25CSHigh();
    xSemaphoreGive(SPI2Semaphore);
}

/************************************************************************
* Function: void SST25ChipErase(void)
*                                                                       
* Overview: chip erase
*                                                                       
* Input: none
*                                                                       
************************************************************************/
void SST25ChipErase(void)
{
   	SST25WriteEnable();

 	xSemaphoreTake(SPI2Semaphore, portMAX_DELAY);
    SST25CSLow();

    SPI2Put(SST25_CMD_ERASE);
    SPI2Get();

    SST25CSHigh();
    xSemaphoreGive(SPI2Semaphore);

    // Wait for write end
    while(SST25IsWriteBusy());
    
}

/************************************************************************
* Function: void SST25ResetWriteProtection()
*                                                                       
* Overview: this function reset write protection bits
*                                                                       
* Input: none                                                     
*                                                                       
* Output: none
*                                                                       
************************************************************************/
void SST25ResetWriteProtection()
{
	xSemaphoreTake(SPI2Semaphore, portMAX_DELAY);

    SST25CSLow();

    SPI2Put(SST25_CMD_EWSR);
    SPI2Get();

    SST25CSHigh();

    SST25CSLow();

    SPI2Put(SST25_CMD_WRSR);
    SPI2Get();

    SPI2Put(0);
    SPI2Get();

    SST25CSHigh();
    xSemaphoreGive(SPI2Semaphore);
}
/************************************************************************
* Function: void SST25SectorErase(DWORD address)                                           
*                                                                       
* Overview: this function erases a 4Kb sector
*                                                                       
* Input: address within sector to be erased
*                                                                       
* Output: none                                 
*                                                                       
************************************************************************/
void SST25SectorErase(DWORD address)
{
    SST25WriteEnable();
    
	xSemaphoreTake(SPI2Semaphore, portMAX_DELAY);
    SST25CSLow();

    SPI2Put(SST25_CMD_SER);
    SPI2Get();

    SPI2Put(((DWORD_VAL)address).v[2]);
    SPI2Get();

    SPI2Put(((DWORD_VAL)address).v[1]);
    SPI2Get();

    SPI2Put(((DWORD_VAL)address).v[0]);
    SPI2Get();

    SST25CSHigh();
    
   	xSemaphoreGive(SPI2Semaphore);
		
    // Wait for write end
    while(SST25IsWriteBusy());
    
 }

/***************************************************************************
 * Start a write operation by recording the start address
 ***************************************************************************/
void SST25BeginWrite(DWORD dwAddr)
{
    dwWriteAddr = dwAddr;
}

/******************************************************************
 *  SST25WriteIncrementalArray(BYTE* vData, WORD wLen);
 * write an array erasing the sector if required
 *****************************************************************/
void SST25WriteIncrementalArray(BYTE* vData, WORD wLen)
{
	while (wLen > 0) {
		// wait for any existing transaction to finish
		while (SST25IsWriteBusy());
		
		// clear the sector if on a sector boundary
		if ((dwWriteAddr & SST25_FLASH_SECTOR_MASK) == 0) {
			SST25SectorErase(dwWriteAddr);
			while (SST25IsWriteBusy());
		}
			
		// write the byte of data
		SST25WriteByte(dwWriteAddr, *vData);
		// dwWriteAddr is incremented in WriteByte
		vData++;
		wLen--;
	}
}

#endif
