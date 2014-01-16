#pragma once

#include "stdint.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include "hexfile.h"

using namespace std;

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
class HexManager
{
private:

    std::string HexFileNamePath;

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
    HexManager(){
        //HexFilePtr = NULL;
	}
	//Destructor
    ~HexManager()	{
		// If hex file is open close it.

	}
};
