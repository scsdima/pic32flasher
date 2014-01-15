#include "config.h"

#include "ComPort.h"

#ifdef __UNIX__
    #include <termios.h>
    #include <sys/ioctl.h>
#endif
#ifndef _POSIX_SOURCE
    #define _POSIX_SOURCE
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

bool CComPort::OpenComPort()
{    

#ifdef __UNIX__
     struct termios newtio;
    memset (&newtio, 0, sizeof newtio);

     bytes_read    = 0;     // Number of bytes read from port
     bytes_written = 0;    // Number of bytes written to the port
     comPortHandle = open(portname.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

    //tcgetattr(comPortHandle,&newtio);
    newtio.c_cflag = CS8;
    cfsetospeed(&newtio,lstBaud[baudrate_n]);
    cfsetispeed(&newtio,lstBaud[baudrate_n]);
    tcflush(comPortHandle, TCIFLUSH);
    tcsetattr(comPortHandle,TCSANOW,&newtio);

    fcntl(comPortHandle, F_SETFL, 0);

      if (comPortHandle  <0)
      {
       
        CloseComPort();
        std::cerr<<"open_port: Unable to open "<<portname<<std::endl;
        return false;
        }
      return true;
#endif

#ifdef __WIN32__

      int   bStatus;
      DCB          comSettings;          // Contains various port settings
      COMMTIMEOUTS CommTimeouts;      
      std::wstring wportname;
      wportname = std::wstring(portname.begin(), portname.end());
      comPortHandle =  CreateFile((WCHAR*)wportname.c_str(),                // open com5:
                  GENERIC_READ | GENERIC_WRITE, // for reading and writing
                  0,                            // exclusive access
                  NULL,                         // no security attributes
                  OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL,
                  NULL);

      // Open COM port
      if (comPortHandle== INVALID_HANDLE_VALUE)
      {
          // error processing code goes here
          std::cerr<<"Not able to open com port!"<<portname<<std::endl;
          comPortHandle = NULL;

      }
      else
      {          
                 // Set timeouts in milliseconds
          CommTimeouts.ReadIntervalTimeout         = 0;
          CommTimeouts.ReadTotalTimeoutMultiplier  = 0;
          CommTimeouts.ReadTotalTimeoutConstant    = 5; // Read time out 5ms.
          CommTimeouts.WriteTotalTimeoutMultiplier = 1;
          CommTimeouts.WriteTotalTimeoutConstant   = 500; // Write time out 500ms.

          (void)SetCommTimeouts(comPortHandle,&CommTimeouts);
          // Set Port parameters.
          // Make a call to GetCommState() first in order to fill
          // the comSettings structure with all the necessary values.
          // Then change the ones you want and call SetCommState().
          GetCommState(comPortHandle, &comSettings);


          comSettings.BaudRate = Baud_val[baudrate_n];
          comSettings.StopBits = ONESTOPBIT;
          comSettings.ByteSize = 8;
          comSettings.Parity   = NOPARITY;   // No Parity
          comSettings.fParity  = FALSE;
          comSettings.fRtsControl = RTS_CONTROL_ENABLE; // Keep the RTS ON, to trigger bootloader enter bootload mode.
          bStatus = SetCommState(comPortHandle, &comSettings);
          if (bStatus == 0)
          {
              // error processing code goes here
              std::cerr<<"open_port: Unable to open "<<portname<<std::endl;
              CloseComPort();
          }
        else return true;

      }
      return false;
#endif

}

/****************************************************************************
 * Opens com port
 *
 * \param comPort  Com port to be opened.
 * \param baud     Baud rate
 * \param 
 * \return         Opens the com port
 *****************************************************************************/
bool CComPort::OpenComPort(std::string &pname, unsigned int baud)
{    
	portname = pname;
    baudrate_n = baud;
    return OpenComPort();
}



/****************************************************************************
 * Close com port
 *
 * \param   
 * \param      
 * \param 
 * \return         
 *****************************************************************************/
void CComPort::CloseComPort(void)
{

    #ifdef __UNIX__
        close(comPortHandle);
    #endif

    #ifdef __WIN32__
        CloseHandle(comPortHandle);
    #endif
         comPortHandle= NULL;
}


/****************************************************************************
 * Send com port
 *
 * \param  buffer: Data buffer
 * \param  bufflen: Buffer Length    
 * \param 
 * \return         
 *****************************************************************************/
void CComPort::SendComPort(char *buffer, uint16_t bufflen)
{
#ifdef __UNIX__
    fcntl(comPortHandle, F_SETFL, 0);
	bytes_written = write(comPortHandle,buffer,bufflen);
#endif
#ifdef __WIN32__
    (void)WriteFile(comPortHandle,              // Handle
               &buffer[0],      // Outgoing data
               bufflen,              // Number of bytes to write
               &bytes_written,  // Number of bytes written
               NULL);
#endif

}

/****************************************************************************
 * Reads com port
 *
 * \param  buffer: Data buffer
 * \param  MaxLen: Max possible length
 * \param 
 * \return  Number of bytes read.       
 *****************************************************************************/
int CComPort::ReadComPort(char* buffer, uint16_t MaxLen)
{

#ifdef __UNIX__
    bytes_read = read (comPortHandle, buffer, MaxLen);
    if(bytes_read<0)
        return 	0;
    return bytes_read;
#endif
#ifdef __WIN32__
    (void)ReadFile(comPortHandle,   // Handle
                buffer,            // Incoming data
                MaxLen,                  // Number of bytes to read
                (unsigned long *)&RxCount,          // Number of bytes read
                NULL);

        return RxCount;
#endif

	
}

int CComPort::bytesAvailable(void)
{
#ifdef __UNIX__
    int bytesAv;
    if(ioctl(comPortHandle, FIONREAD, &bytesAv)<0) bytesAv=0;
    return bytesAv;
#endif
#ifdef __WIN32__
    COMSTAT stat;
      DWORD dwErrors;
      if (!ClearCommError(comPortHandle, &dwErrors, &stat))
      {
        return 0;
      }
    return stat.cbInQue;
#endif
}

void CComPort::flush()
{
    #ifdef __UNIX__
        tcflush(comPortHandle, TCIFLUSH);
    #endif
    #ifdef __WIN32__
        FlushFileBuffers(comPortHandle);
    #endif
}

/****************************************************************************
 * Gets com port status
 *
 * \param  
 * \param   
 * \param 
 * \return  true if com port is up and running      
 *****************************************************************************/
bool CComPort::GetComPortOpenStatus(void)
{
    return (comPortHandle != 0);

}

/************End of file******************************************************/
