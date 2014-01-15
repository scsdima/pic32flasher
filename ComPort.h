#pragma once

#ifdef __UNIX__
#include <termios.h>
#endif

#ifdef __WIN32__
#include "windows.h"
#endif

#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <stdint.h>

#ifdef __UNIX__
typedef int HANDLE;
#endif



typedef enum{
    B115200=115200,
    B57600=57600,
    B19200=19200,
    B9600=9600,
    B2400=2400,
    BAUD_TXT_MAX
}BaudRate_t;

static const char *Baud_txt[] = {"115200","57600","19200","9600","2400"};
static const int Baud_val[]={B115200,B57600,B19200,B9600,B2400};


class CComPort
{
private:
    std::string portname;
    uint8_t baudrate_n;
    DWORD bytes_read    ;     // Number of bytes read from port
    DWORD     bytes_written;

    HANDLE      comPortHandle; 	// Handle COM port
    uint16_t 	RxCount;


public:
    CComPort(void)	{
        comPortHandle = 0;
    }
    ~CComPort(void)	{
        // if comport is already opened close it.
        if(comPortHandle)		{
            CloseComPort();
        }
    }

    bool IsEnabled();
    bool OpenComPort(std::string &pname, unsigned int baud);
    bool OpenComPort(void );
    void setBaudRate(uint8_t br) {baudrate_n =br;}
    void setPortName(const std::string &pname) { portname =pname;}
    void CloseComPort(void);
    void SendComPort(char*, uint16_t );
    bool GetComPortOpenStatus(void);
    int ReadComPort(char*, uint16_t);
    int bytesAvailable(void);
    void flush();

};
