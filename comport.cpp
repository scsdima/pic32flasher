#include "ComPort.h"


bool ComPort::OpenComPort(void)
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
      if (comPortHandle== INVALID_HANDLE_VALUE)      {
          // error processing code goes here
          std::cerr<<"Can't open serial port!"<<portname<<std::endl;
          comPortHandle = NULL;

      }
      else      {          
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


          comSettings.BaudRate = (DWORD)baudrate;
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
bool ComPort::OpenComPort(const std::string &portname, BaudRate baudrate){
    this->portname = portname;
    this->baudrate = baudrate;
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
void ComPort::CloseComPort(void){
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
void ComPort::SendComPort(char *data, size_t datasize){
#ifdef __UNIX__
    fcntl(comPortHandle, F_SETFL, 0);
    bytes_written = write(comPortHandle,data,datasize);
#endif
#ifdef __WIN32__
    (void)WriteFile(comPortHandle,              // Handle
               &data[0],      // Outgoing data
               datasize,              // Number of bytes to write
               (PDWORD)&bytes_written,  // Number of bytes written
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
int ComPort::ReadComPort(char* data, size_t datasize){
#ifdef __UNIX__
    bytes_read = read (comPortHandle, data, datasize);
    if(bytes_read<0)
        return 	0;
    return bytes_read;
#endif
#ifdef __WIN32__
    (void)ReadFile(comPortHandle,   // Handle
                &data[0],            // Incoming data
                datasize,                  // Number of bytes to read
                (PDWORD)&RxCount,          // Number of bytes read
                NULL);

        return RxCount;
#endif

	
}

int ComPort::bytesAvailable(void){
#ifdef __UNIX__
    int bytesAv;
    if(ioctl(comPortHandle, FIONREAD, &bytesAv)<0) bytesAv=0;
    return bytesAv;
#endif
#ifdef __WIN32__
    COMSTAT stat;
      DWORD dwErrors;
      if (!ClearCommError(comPortHandle, &dwErrors, &stat))      {
        return 0;
      }
    return stat.cbInQue;
#endif
}

/****************************************************************************
 * Flush unsent data
 *
 * \param
 * \param
 * \param
 * \return  void
 *****************************************************************************/
void ComPort::flush(void){
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
bool ComPort::GetComPortOpenStatus(void){
    return (comPortHandle != 0);

}

/************End of file******************************************************/
