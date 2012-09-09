#pragma once

#include "stdint.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdio.h>

typedef struct 
{
	unsigned char RecDataLen;
	unsigned int Address;
	unsigned int MaxAddress;
	unsigned int MinAddress;
	unsigned char RecType;
	unsigned char* Data;
	unsigned char CheckSum;	
	unsigned int ExtSegAddress;
	unsigned int ExtLinAddress;
}T_HEX_RECORD;


// Hex Manager class
class CHexManager
{
private:

    std::string HexFileNamePath;
	FILE *HexFilePtr;

	
	

public:
	unsigned int HexTotalLines;
	unsigned int HexCurrLineNo;
	bool ResetHexFilePointer(void);
    bool LoadHexFile(const std::string &);
	unsigned short GetNextHexRecord(char *HexRec, unsigned int BuffLen);
	unsigned short ConvertAsciiToHex(void *VdAscii, void *VdHexRec);
	void VerifyFlash(unsigned int* StartAdress, unsigned int* ProgLen, unsigned short* crc);
    static  unsigned short CalculateCrc(char *data, unsigned int len);

	//Constructor
	CHexManager()
	{
		HexFilePtr = NULL;
	}
	//Destructor
	~CHexManager()
	{
		// If hex file is open close it.
		if(HexFilePtr)
		{
			fclose(HexFilePtr);
		}
	}


};
