#pragma once
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include "stdint.h"

typedef int HANDLE;
static const char *txtBaud[] = {"115200","57600","19200","9600","2400"};
static const int lstBaud[]={B115200,B57600,B19200,B9600,B2400};

enum{
	baud115200,
	baud57600,
	baud19200,
	baud9600,
	baud2400,
	BAUD_TXT_MAX
};

class CComPort
{
private:
    std::string portname;
	uint8_t baudrate_n;
    int bytes_read    ;     // Number of bytes read from port
    int     bytes_written;

	HANDLE      comPortHandle; 	// Handle COM port 
	uint16_t 	RxCount;
	

public:
// Constructor
	CComPort(void)
	{
        comPortHandle = 0;
	}


// Destructor
	~CComPort(void)
	{
		// if comport is already opened close it.
		if(comPortHandle)
		{
			CloseComPort();
		}
	}

	
    bool OpenComPort(std::string &pname, unsigned int baud);
    bool OpenComPort(void );
    void setBaudRate(uint8_t br) {baudrate_n =br;}
    void setPortName(const std::string &pname) { portname =pname;}
	void CloseComPort(void);
	void SendComPort(char*, uint16_t );
	bool GetComPortOpenStatus(void);	
	int ReadComPort(char*, uint16_t);
    int bytesAvailable(void);

};
