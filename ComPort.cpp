#include "ComPort.h"
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

#ifndef _POSIX_SOURCE
    #define _POSIX_SOURCE
#endif

bool CComPort::OpenComPort()
{    

     struct termios newtio;
    memset (&newtio, 0, sizeof newtio);

     bytes_read    = 0;     // Number of bytes read from port
     bytes_written = 0;    // Number of bytes written to the port
     comPortHandle = open(portname.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    tcgetattr(comPortHandle,&newtio);
    newtio.c_cflag =  CS8 ;
    cfsetospeed(&newtio,lstBaud[baudrate_n]);
    cfsetispeed(&newtio,lstBaud[baudrate_n]);
    tcflush(comPortHandle, TCIFLUSH);
    tcsetattr(comPortHandle,TCSANOW,&newtio);

      if (comPortHandle  <0)
      {
       
        CloseComPort();
        std::cout<<"open_port: Unable to open "<<portname<<std::endl;
        return false;
        }
      return true;

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

			fcntl(comPortHandle, F_SETFL, 0);		
                        comPortHandle = 0;
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
	bytes_written = write(comPortHandle,buffer,bufflen);

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

    bytes_read = read (comPortHandle, buffer, MaxLen);
    if(bytes_read<0)
        return 	0;
    return bytes_read;
	
}

int CComPort::bytesAvailable(void)
{
    int bytesAv;
    if(ioctl(comPortHandle, FIONREAD, &bytesAv)<0) bytesAv=0;
    return bytesAv;
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
    return comPortHandle!=0;

}

/************End of file******************************************************/
