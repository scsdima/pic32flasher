#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdio.h>

#include "ComPort.h"
#include "Hex.h"
#include "BootLoader.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static const char THIS_FILE[] = __FILE__;
#endif

// Virtual Flash.
#define KB      (1024)
#define MB      (KB*KB)

// 5 MB flash
static unsigned char VirtualFlash[5*MB];
static unsigned char virtual_file[5*MB];

#define BOOT_SECTOR_BEGIN   (0x9FC00000)
#define APPLICATION_START   (0x9D000000)
#define PA_TO_VFA(x)        (x - APPLICATION_START)
#define PA_TO_KVA0(x)       (x | 0x80000000)

#define DATA_RECORD 		(0)
#define END_OF_FILE_RECORD 	(1)
#define EXT_SEG_ADRS_RECORD (2)
#define EXT_LIN_ADRS_RECORD (4)

#define READ_FILE_TO_MEMORY  (1)

static char Ascii[1000];

static HexFile hex_file;
/**
 * Static table used for the table_driven implementation.
 *****************************************************************************/
static const unsigned short crc_table[16] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
};

/****************************************************************************
 * Loads hex file
 *
 * \param  
 * \param   
 * \param 
 * \return  true if hex file loads successfully      
 *****************************************************************************/
bool HexManager::LoadHexFile(const std::string &fname)
{
	char HexRec[255];
    HexFileNamePath =fname;
    HexFile::HexFileMode mode = HexFile::FileBin;
    if(fname.find(".hex")!= string::npos){
        mode = HexFile::FileHex;
    }
    if(hex_file.open(HexFileNamePath.c_str(),mode)==false)	{
        return false;
	}
	else	{
		HexTotalLines = 0;
        /*calculating number of lines in file*/
        while(!hex_file.eof()) {
            hex_file.gets(&HexRec[0], (size_t)sizeof(HexRec));
			HexTotalLines++;
		}
	}
	return true;
}


/****************************************************************************
 * Gets next hex record from the hex file
 *
 * \param  HexRec: Pointer to HexRec.
 * \param  BuffLen: Buffer Length 
 * \param 
 * \return Length of the hex record in bytes.  
 *****************************************************************************/

unsigned short HexManager::GetNextHexRecord(char *HexRec, unsigned int BuffLen)
{
	unsigned short len = 0;
    if(!hex_file.eof())
	{
        hex_file.gets(Ascii, BuffLen);
		if(Ascii[0] != ':')		{
			// Not a valid hex record.
			return 0;
		}
		// Convert rest to hex.
		len = ConvertAsciiToHex((void *)&Ascii[1], (void *)HexRec);
		HexCurrLineNo++;		
	}	
	return len;
}

/****************************************************************************
 * Resets hex file pointer.
 *
 * \param  
 * \param   
 * \param 
 * \return  True if resets file pointer.     
 *****************************************************************************/
bool HexManager::ResetHexFilePointer(void){
	// Reset file pointer.
    if(hex_file.seek( 0, 0) == 0){
		HexCurrLineNo = 0;
		return true;
	}
    return false;
}


/****************************************************************************
 * Converts ASCII to hex.
 *
 * \param  VdAscii: Hex Record in ASCII format.
 * \param  VdHexRec: Hex record in Hex format.
 * \param 
 * \return  Number of bytes in Hex record(Hex format)    
 *****************************************************************************/
unsigned short HexManager::ConvertAsciiToHex(void *VdAscii, void *VdHexRec){
    char temp[5] = {'0','x',0, 0, 0};
	unsigned int i = 0;
	char *Ascii;
	char *HexRec;
	Ascii = (char *)VdAscii;
	HexRec = (char *)VdHexRec;
	while(1)	{
		temp[2] = Ascii[i++];
		temp[3] = Ascii[i++];
        if((temp[2] == 0) || (temp[3] == 0)){
            i -= 2;// Not a valid ASCII. Stop conversion and break.
			break;			
		}
		else
		{			
            sscanf(temp, "%x", HexRec);// Convert ASCII to hex.
			HexRec++;			
		}
	}

	return (i/2); // i/2: Because, an representing Hex in ASCII takes 2 bytes.
}


/****************************************************************************
 * Verifies flash
 *
 * \param  StartAddress: Pointer to program start address
 * \param  ProgLen: Pointer to Program length in bytes 
 * \param  crc : Pointer to CRC
 * \return      
 *****************************************************************************/
void HexManager::VerifyFlash(unsigned int* StartAdress, unsigned int* ProgLen, unsigned short* crc)
{
	unsigned short HexRecLen;
	char HexRec[255];
    T_HEX_RECORD HexRecordSt={0};
	unsigned int VirtualFlashAdrs;
	unsigned int ProgAddress;
	
	// Virtual Flash Erase (Set all bytes to 0xFF)
	memset((void*)VirtualFlash, 0xFF, sizeof(VirtualFlash));


	// Start decoding the hex file and write into virtual flash
	// Reset file pointer.
    hex_file.seek(0, 0);

	// Reset max address and min address.
	HexRecordSt.MaxAddress = 0;
	HexRecordSt.MinAddress = 0xFFFFFFFF;

    while((HexRecLen = GetNextHexRecord(&HexRec[0], 255)) != 0)
	{
		HexRecordSt.RecDataLen = HexRec[0];
		HexRecordSt.RecType = HexRec[3];	
		HexRecordSt.Data = (unsigned char*)&HexRec[4];

		switch(HexRecordSt.RecType)
		{

			case DATA_RECORD:  //Record Type 00, data record.
				HexRecordSt.Address = (((HexRec[1] << 8) & 0x0000FF00) | (HexRec[2] & 0x000000FF)) & (0x0000FFFF);
				HexRecordSt.Address = HexRecordSt.Address + HexRecordSt.ExtLinAddress + HexRecordSt.ExtSegAddress;
				
				ProgAddress = PA_TO_KVA0(HexRecordSt.Address);

				if(ProgAddress < BOOT_SECTOR_BEGIN) // Make sure we are not writing boot sector.
				{
					if(HexRecordSt.MaxAddress < (ProgAddress + HexRecordSt.RecDataLen))
					{
						HexRecordSt.MaxAddress = ProgAddress + HexRecordSt.RecDataLen;
					}

					if(HexRecordSt.MinAddress > ProgAddress)
					{
						HexRecordSt.MinAddress = ProgAddress;
					}
				
					VirtualFlashAdrs = PA_TO_VFA(ProgAddress); // Program address to local virtual flash address

					memcpy((void *)&VirtualFlash[VirtualFlashAdrs], HexRecordSt.Data, HexRecordSt.RecDataLen);
				}
				break;
			
			case EXT_SEG_ADRS_RECORD:  // Record Type 02, defines 4 to 19 of the data address.
				HexRecordSt.ExtSegAddress = ((HexRecordSt.Data[0] << 16) & 0x00FF0000) | ((HexRecordSt.Data[1] << 8) & 0x0000FF00);				
				HexRecordSt.ExtLinAddress = 0;
				break;
					
			case EXT_LIN_ADRS_RECORD:
				HexRecordSt.ExtLinAddress = ((HexRecordSt.Data[0] << 24) & 0xFF000000) | ((HexRecordSt.Data[1] << 16) & 0x00FF0000);
				HexRecordSt.ExtSegAddress = 0;
				break;


			case END_OF_FILE_RECORD:  //Record Type 01
			default: 
				HexRecordSt.ExtSegAddress = 0;
				HexRecordSt.ExtLinAddress = 0;
				break;
		}	
	}

	HexRecordSt.MinAddress -= HexRecordSt.MinAddress % 4;
	HexRecordSt.MaxAddress += HexRecordSt.MaxAddress % 4;

	*ProgLen = HexRecordSt.MaxAddress - HexRecordSt.MinAddress;
	*StartAdress = HexRecordSt.MinAddress;
	VirtualFlashAdrs = PA_TO_VFA(HexRecordSt.MinAddress);
    *crc = CalculateCrc((char*)&VirtualFlash[VirtualFlashAdrs], *ProgLen);
}

/****************************************************************************
 * Update the crc value with new data.
 *
 * \param crc      The current crc value.
 * \param data     Pointer to a buffer of \a data_len bytes.
 * \param len		Number of bytes in the \a data buffer.
 * \return         The updated crc value.
 *****************************************************************************/
unsigned short HexManager::CalculateCrc(char *data, unsigned int len){
    unsigned int i;
    unsigned short crc = 0;
    while(len--)    {
        i = (crc >> 12) ^ (*data >> 4);
        crc = crc_table[i & 0x0F] ^ (crc << 4);
        i = (crc >> 12) ^ (*data >> 0);
        crc = crc_table[i & 0x0F] ^ (crc << 4);
        data++;
    }

    return (crc & 0xFFFF);
}
/********************End of file************************************************/
