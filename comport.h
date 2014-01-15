#pragma once

#ifdef __WIN32__
#include "windows.h"
#endif

#ifdef __UNIX__
#include <termios.h>
#include <sys/ioctl.h>
typedef int HANDLE;
#endif

#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef enum{
    Baud115200 = 115200,
    Baud57600 = 57600,
    Baud19200 = 19200,
    Baud9600 = 9600,
    Baud2400 = 2400,
}BaudRate;

class ComPort
{
private:
    std::string     portname;
    BaudRate     baudrate;
    uint32_t    bytes_read    ;     // Number of bytes read from port
    uint32_t    bytes_written;
    HANDLE      comPortHandle; 	// Handle COM port
    uint16_t 	RxCount;

public:
    ComPort(void)	{
        comPortHandle = 0;
    }

    ~ComPort(void)	{
        // if comport is already opened close it.
        if(comPortHandle) {
            CloseComPort();
        }
    }

    bool IsEnabled(void);
    bool OpenComPort(const std::string &portname, BaudRate baudrate);
    bool OpenComPort(void );
    void setBaudRate(BaudRate baudrate) {this->baudrate = baudrate;}
    void setPortName(const std::string &portname) { this->portname =portname;}
    void CloseComPort(void);
    void SendComPort(char *data, size_t datasize);
    bool GetComPortOpenStatus(void);
    int ReadComPort(char*data, size_t datasize);
    int bytesAvailable(void);
    void flush(void);

};
