#pragma once

#include "Hex.h"
#include "ComPort.h"
#include "stdint.h"

// Notify message IDs
#define WM_USER_BOOTLOADER_RESP_OK WM_USER+1
#define WM_USER_BOOTLOADER_NO_RESP WM_USER+2

// Trasnmission states
#define FIRST_TRY 0
#define RE_TRY 1


// Main Bootloader class
class BootLoader
{



public:

    typedef enum {
        jobVersion =0,
        jobErase= 1,
        jobFlash= 2,
        jobRun =3,
        jobStartBootloader =4,
        jobWritePassword=5,
        jobWriteId =6,
        JobsCount =7,
        jobNothing =8
    }Jobs;

    typedef enum { SerialPort }PortType;

    // Commands
    typedef enum    {
        READ_BOOT_INFO = 1,
        ERASE_FLASH,
        PROGRAM_FLASH,
        READ_CRC,
        JMP_TO_APP
    }Commands;

    ComPort com_port;
		
	// Constructor
    BootLoader()	{
		// Initialization of some flags and variables
		RxFrameValid = false;
		NoResponseFromDevice = false;
		TxState = FIRST_TRY;
		RxDataLen = 0;
		ResetHexFilePtr = true;
	}

	// Destructor
    ~BootLoader()	{

	}

    bool runJob(Jobs job, BaudRate baudrate
                ,void *data,const std::string &pname);
	void TransmitTask(void);
    bool ReceiveTask(void);    
	bool SendCommand(char cmd, unsigned short Retries, unsigned short RetryDelayInMs);	    
	void BuildRxFrame(unsigned char*, unsigned short);
	void HandleResponse(void);	
	void StopTxRetries(void);
	void NotifyEvent(unsigned int lEvent);
	void GetRxData(char *buff);
	void GetProgress(int *Lower, int *Upper);
	void HandleNoResponse(void);
	unsigned short CalculateFlashCRC(void);
	bool LoadHexFile(void);
    void OpenPort(PortType  port);
    bool isPortOpen(PortType Port);
    void ClosePort(void);
    bool SendByProtocol(uint8_t adr,uint8_t command,uint32_t parameter);
    bool StartBootloader(void);
    bool StartProgramming(const string& fname);

private:
    unsigned char TxPacket[1000];
    unsigned short TxPacketLen;
    unsigned char RxData[255];
    unsigned short RxDataLen;
    unsigned short RetryCount;
    bool        RxFrameValid;
    unsigned char LastSentCommand;
    bool        NoResponseFromDevice;
    unsigned int    TxState;
    unsigned short  MaxRetry;
    unsigned short  TxRetryDelay;
    HexManager  hex_manager;
    bool        ResetHexFilePtr;
    PortType     port_selected;
    void    WritePort(unsigned char *buffer, int bufflen);
    unsigned short ReadPort(unsigned char *buffer, int bufflen);
};





