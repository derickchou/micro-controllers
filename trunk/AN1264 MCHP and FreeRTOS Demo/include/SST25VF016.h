/*****************************************************************************
 *
 * Basic access to SPI Flash SST25VF016 located on 
 * Graphics LCD Controller PICtail Plus SSD1926 Board.
 *
 *****************************************************************************
 * FileName:        SST25VF016.h
 * Dependencies:    Graphics.h
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

#ifndef _SST25VF016_H
#define _SST25VF016_H

#include "Graphics\Graphics.h"

/************************************************************************
* SST25 Commands                                                       
************************************************************************/
#define SST25_CMD_READ     (unsigned)0x03
#define SST25_CMD_WRITE    (unsigned)0x02
#define SST25_CMD_WREN     (unsigned)0x06
#define SST25_CMD_RDSR     (unsigned)0x05
#define SST25_CMD_ERASE    (unsigned)0x60
#define SST25_CMD_EWSR     (unsigned)0x50
#define SST25_CMD_WRSR     (unsigned)0x01
#define SST25_CMD_SER      (unsigned)0x20

#define SST25_FLASH_SECTOR_SIZE 		(4096ul)
#define SST25_FLASH_SECTOR_MASK			(SST25_FLASH_SECTOR_SIZE - 1)


/************************************************************************
* Macro: SST25CSLow()                                                   
*                                                                       
* Preconditions: CS IO must be configured as output
*                                                                       
* Overview: this macro pulls down CS line                    
*                                                                       
* Input: none                                                          
*                                                                       
* Output: none                                                         
*                                                                       
************************************************************************/
#define SST25CSLow()      SST25_CS_LAT=0;

/************************************************************************
* Macro: SST25CSHigh()
*                                                                       
* Preconditions: CS IO must be configured as output
*                                                                       
* Overview: this macro set CS line to high level
*                                                                       
* Input: none                                                          
*                                                                       
* Output: none
*                                                                       
************************************************************************/
#define SST25CSHigh()     SST25_CS_LAT=1;

/************************************************************************
* Function: SST25Init()
*                                                                       
* Overview: this function setups SPI and IOs connected to SST25
*                                                                       
* Input: none
*                                                                       
* Output: none  
*                                                                       
************************************************************************/
void SST25Init();

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
BYTE SST25IsWriteBusy();

/************************************************************************
* Function: void SST25WriteByte(DWORD address, BYTE data)                                           
*                                                                       
* Overview: this function writes a byte to the address specified
*                                                                       
* Input: byte to be written and address
*                                                                       
* Output: none
*                                                                       
************************************************************************/
void SST25WriteByte(DWORD address, BYTE data);

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
BYTE SST25ReadByte(DWORD address);

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
void SST25WriteWord(DWORD address, WORD data);

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
WORD SST25ReadWord(DWORD address);

/************************************************************************
* Function: SST25WriteEnable()                                       
*
* Overview: this function allows writing into SST25. Must be called
*           before every write/erase command
*
* Input: none
*            
* Output: none
*
************************************************************************/
void SST25WriteEnable();

/************************************************************************
* Function: BYTE SST25WriteArray(DWORD address, BYTE* pData, nCount)
*                                                                       
* Overview: this function writes data array at the address specified
*                                                                       
* Input: flash memory address, pointer to the data array, data number
*                                                                       
* Output: return 1 if the operation was successfull
*                                                                     
************************************************************************/
BYTE SST25WriteArray(DWORD address, BYTE* pData, WORD nCount);

/************************************************************************
* Function: void SST25ReadArray(DWORD address, BYTE* pData, nCount)
*                                                                       
* Overview: this function reads  data into buffer specified
*                                                                       
* Input: flash memory address, pointer to the buffer, data number
*                                                                       
************************************************************************/
void SST25ReadArray(DWORD address, BYTE* pData, WORD nCount);

/************************************************************************
* Function: void SST25ChipErase(void)
*                                                                       
* Overview: chip erase
*                                                                       
* Input: none
*                                                                       
************************************************************************/
void SST25ChipErase(void);

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
void SST25ResetWriteProtection();

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
void SST25SectorErase(DWORD address);

/***************************************************************************
 * Start a write operation by recording the start address
 ***************************************************************************/
void SST25BeginWrite(DWORD dwAddr);

/******************************************************************
 *  SST25WriteIncrementalArray(BYTE* vData, WORD wLen);
 * write an array erasing the sector if required
 *****************************************************************/
void SST25WriteIncrementalArray(BYTE* vData, WORD wLen);

#endif //_SST25VF016_H

