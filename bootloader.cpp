#include <string>

#include "comport.h"
#include "hex.h"
#include "bootloader.h"
#include "simple_crypt.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SOH 01
#define EOT 04
#define DLE 16

#ifdef __UNIX___
    #define SLEEP(_X_) usleep((_X_)*1000)
#endif
#ifdef __WIN32__
    #define SLEEP(_X_) Sleep(_X_)
#endif

char Buff[1000];

using namespace std;
/****************************************************************************
 *  Receive Task
 *
 * \param
 * \param
 * \param
 * \return
 *****************************************************************************/
bool BootLoader::ReceiveTask(void)
{
    bool res=false;
    unsigned short BuffLen;
    char Buff[255];
    RxFrameValid=false;
    BuffLen = ReadPort((unsigned char*)Buff, (sizeof(Buff) - 10));
    if(BuffLen) BuildRxFrame((unsigned char*)Buff, BuffLen);
    if(RxFrameValid)    {
        // Valid frame is received.
        // Disable further retries.
        RxFrameValid = false;
        // Handle Response
        HandleResponse();
        res=true;
    }
    else    {
        // Retries exceeded. There is no reponse from the device.
        if(NoResponseFromDevice)        {
            // Reset flags
            NoResponseFromDevice = false;
            RxFrameValid = false;
            // Handle no response situation.
            HandleNoResponse();
        }
    }
    return res;
}


/****************************************************************************
 *  Handle no response situation
 *
 * \param
 * \param
 * \param
 * \return
 *****************************************************************************/
void BootLoader::HandleNoResponse(void){
    // Handle no response situation depending on the last sent command.
    switch(LastSentCommand) {
    case READ_BOOT_INFO:
        cout<<endl<<"version:";
        break;
    case ERASE_FLASH:
        cout<<endl<<"erase:";
        break;
    case PROGRAM_FLASH:
        cout<<endl<<"program:";
        break;
    case READ_CRC:
        cout<<endl<<"verify(crc):";
        break;
    }
    cout<<endl<<" doesn't responding";
}

/****************************************************************************
 *  Handle Response situation
 *
 * \param
 * \param
 * \param
 * \return
 *****************************************************************************/
void BootLoader::HandleResponse(void){
    uint8_t cmd = RxData[0];
    int8_t majorVer = RxData[3];
    int8_t minorVer = RxData[4];
    switch(cmd)  {
    case READ_BOOT_INFO:        
        cout<<endl<<"ver:"<<(int)majorVer<<"."<<(int)minorVer;
        break;
    case ERASE_FLASH:
        cout<<endl<<"flash erased";
        break;
    case READ_CRC:
        // Notify main window that command received successfully.
        cout<<endl<<"verified" ;
        break;
    case PROGRAM_FLASH:
        break;
    }
}


/****************************************************************************
 *  Builds the response frame
 *
 * \param  buff: Pointer to the data buffer
 * \param  buffLen: Buffer length
 * \param
 * \return
 *****************************************************************************/
void BootLoader::BuildRxFrame(unsigned char *buff, unsigned short buffLen){
    static bool Escape = false;
    unsigned short crc,crcCalc;

    while((buffLen > 0) && (RxFrameValid == false))    {
        buffLen --;
        if(RxDataLen >= (sizeof(RxData)-2))        {
            RxDataLen = 0;
        }
        switch(*buff)  {

        case SOH: //Start of header
            if(Escape)  {
                // Received byte is not SOH, but data.
                RxData[RxDataLen++] = *buff;
                // Reset Escape Flag.
                Escape = false;
            }
            else  {
                // Received byte is indeed a SOH which indicates start of new frame.
                RxDataLen = 0;
            }
            break;

        case EOT: // End of transmission
            if(Escape)  {
                // Received byte is not EOT, but data.
                RxData[RxDataLen++] = *buff;
                // Reset Escape Flag.
                Escape = false;
            }
            else  {
                // Received byte is indeed a EOT which indicates end of frame.
                // Calculate CRC to check the validity of the frame.
                if(RxDataLen > 1)  {
                    crc = (RxData[RxDataLen-2]) & 0x00ff;
                    crc = crc | ((RxData[RxDataLen-1] << 8) & 0xFF00);
                    crcCalc = HexManager::CalculateCrc((char*)RxData, (RxDataLen-2));
                    if((crcCalc == crc) && (RxDataLen > 2))  {
                        // CRC matches and frame received is valid.
                        RxFrameValid = true;
                    }
                }
            }
            break;


        case DLE: // Escape character received.
            if(Escape)  {
                // Received byte is not ESC but data.
                RxData[RxDataLen++] = *buff;
                // Reset Escape Flag.
                Escape = false;
            }
            else  {
                // Received byte is an escape character. Set Escape flag to escape next byte.
                Escape = true;
            }
            break;

        default: // Data field.
            RxData[RxDataLen++] = *buff;
            // Reset Escape Flag.
            Escape = false;
            break;

        }
        // Increment the pointer.
        buff++;
    }
}


/****************************************************************************
 *  Send Command
 *
 * \param		cmd:  Command
 * \param		data: Pointer to data buffer if any
 * \param 		dataLen: Data length
 * \param		retries: Number of retries allowed
 * \param		retryDelayInMs: Delay between retries in milisecond
 * \return
 *****************************************************************************/


bool BootLoader::SendCommand(char cmd, unsigned short Retries, unsigned short DelayInMs){
    unsigned short crc;
    unsigned int StartAddress,  Len;
    unsigned short BuffLen ;
    unsigned short HexRecLen;
    unsigned short totalRecords ;
    NoResponseFromDevice=true;
    while(Retries--)    {
        if(!Retries) this->NoResponseFromDevice=false;
        // Store for later use.

        LastSentCommand = cmd;
        BuffLen=0;
        totalRecords = 10;

        switch((unsigned char)cmd)
        {
        case READ_BOOT_INFO:
            Buff[BuffLen++] = cmd;
            TxRetryDelay = DelayInMs/100; // in ms
            break;

        case ERASE_FLASH:
            Buff[BuffLen++] = cmd;
            TxRetryDelay = DelayInMs/100; // in ms
            break;

        case JMP_TO_APP:
            Buff[BuffLen++] = cmd;
            Retries = 0;
            TxRetryDelay = 10; // in ms
            break;

        case PROGRAM_FLASH:
            Buff[BuffLen++] = cmd;
            HexRecLen = hex_manager.GetNextHexRecord(&Buff[BuffLen], (sizeof(Buff) - 5));
            if(HexRecLen == 0)  {
                //Not a valid hex file.
                return false;
            }

            BuffLen = BuffLen + HexRecLen;
            while(totalRecords)
            {
                HexRecLen = hex_manager.GetNextHexRecord(&Buff[BuffLen], (sizeof(Buff) - 5));
                BuffLen = BuffLen + HexRecLen;
                totalRecords--;
            }
            TxRetryDelay = DelayInMs/100; // in ms
            break;

        case READ_CRC:
            Buff[BuffLen++] = cmd;
            hex_manager.VerifyFlash((unsigned int*)&StartAddress, (unsigned int*)&Len, (unsigned short*)&crc);
            Buff[BuffLen++] = (StartAddress);
            Buff[BuffLen++] = (StartAddress >> 8);
            Buff[BuffLen++] = (StartAddress >> 16);
            Buff[BuffLen++] = (StartAddress >> 24);
            Buff[BuffLen++] = (Len);
            Buff[BuffLen++] = (Len >> 8);
            Buff[BuffLen++] = (Len >> 16);
            Buff[BuffLen++] = (Len >> 24);
            Buff[BuffLen++] =  (char)crc;
            Buff[BuffLen++] =  (char)(crc >> 8);
            TxRetryDelay = DelayInMs/100; // in ms
            break;

        default:
            return false;
            break;

        }//switch
        // Calculate CRC for the frame.        
        crc = HexManager::CalculateCrc(Buff, BuffLen);
        Buff[BuffLen++] = (char)crc;
        Buff[BuffLen++] = (char)(crc >> 8);

        TxPacketLen=0;
        // SOH: Start of header
        TxPacket[TxPacketLen++] = SOH;

        // Form TxPacket. Insert DLE in the data field whereever SOH and EOT are present.
        for(int i = 0; i < BuffLen; i++)
        {
            if((Buff[i] == EOT) || (Buff[i] == SOH)
                    || (Buff[i] == DLE))
            {
                TxPacket[TxPacketLen++] = DLE;
            }
            TxPacket[TxPacketLen++] = Buff[i];
        }
        // EOT: End of transmission
        TxPacket[TxPacketLen++] = EOT;
        WritePort(TxPacket,TxPacketLen);
        int msstep = TxRetryDelay;
        while(!com_port.bytesAvailable() && msstep--) SLEEP(100);        

       if(ReceiveTask())
            return true;
    }//while(Retries)

    return false;
}


/****************************************************************************
 *  Gets locally calculated CRC
 *
 * \param
 * \param
 * \param
 * \return 16 bit CRC
 *****************************************************************************/
unsigned short BootLoader::CalculateFlashCRC(void){
    unsigned int StartAddress,  Len;
    unsigned short crc;
    hex_manager.VerifyFlash((unsigned int*)&StartAddress, (unsigned int*)&Len, (unsigned short*)&crc);
    return crc;
}

/****************************************************************************
 *  Open communication port (USB/COM/Eth)
 *
 * \param Port Type	(USB/COM)
 * \param	com port
 * \param 	baud rate
 * \param   vid
 * \param   pid
 * \return
 *****************************************************************************/
void BootLoader::OpenPort(PortType port){
    port_selected = port;
    com_port.OpenComPort();
}

/****************************************************************************
 *  Get communication port status.
 *
 * \param
 * \return true: Port opened.
           false: Port closed.
 *****************************************************************************/
bool BootLoader::isPortOpen(PortType port){
    bool result;
    result = com_port.GetComPortOpenStatus();
    return result;
}

/****************************************************************************
 *  Closes the communication port (USB/COM/ETH)
 *
 * \param
 * \return
 *****************************************************************************/
void BootLoader::ClosePort(){
        com_port.CloseComPort();
}


/****************************************************************************
 *  Write communication port (USB/COM/ETH)
 *
 * \param Buffer, Len
 * \return
 *****************************************************************************/
void BootLoader::WritePort(unsigned char *buffer, int bufflen){
        com_port.SendComPort((char*)buffer, bufflen);
        com_port.flush();
}


/****************************************************************************
 *  Read communication port (USB/COM/ETH)
 *
 * \param Buffer, Len
 * \return
 *****************************************************************************/
unsigned short BootLoader::ReadPort(unsigned char *buffer, int bufflen){
    int bytesRead;
        bytesRead =com_port.ReadComPort((char*)buffer, bufflen);
        return (unsigned short) bytesRead;
}

/****************************************************************************
 *  Executes bootloader job
 *
 * \param
 * \return
 *****************************************************************************/
bool BootLoader::runJob(Jobs job,BaudRate baudrate, void *data,const std::string &pname){
    bool result = false;
    com_port.setBaudRate(baudrate);
    com_port.setPortName(pname);
    /*loading file if required*/

    if(com_port.OpenComPort())    {        
       switch(job)        {

        case jobVersion:
            this->SendCommand(READ_BOOT_INFO,2,200);
            result = true;
            break;

        case jobErase:
            this->SendCommand(ERASE_FLASH,3,3000);
            result = true;
            break;

        case jobStartBootloader:
            result = StartBootloader();
            break;

        case jobFlash:          
            result = StartProgramming(*((string*)data));
            break;

        case jobRun:
            this->SendCommand(JMP_TO_APP,3,500);
            result = true;
            break;

        case jobWritePassword:
            {
               uint32_t value;
               value = *((uint32_t*)data);
               result = SendByProtocol(0x01,0xfd,value);
           }
           break;

        case jobWriteId:
           {
               uint32_t value;
               value = *((uint32_t*)data);
               result = SendByProtocol(0x01,0xfc,value);
           }
            break;

        default:
            break;
        }
    }
    ClosePort();
    if(result == true){
        cout<<endl<<"OK";
    }
    else {
        cout<<endl<<"ERROR";
    }
    return true;
}

bool BootLoader::StartProgramming(const string& fname){
    bool file_loaded=false;
    cout<<fname;
    if(fname.empty())    {
        return false;
    }
    file_loaded = hex_manager.LoadHexFile(fname);

     if(file_loaded)
     {
         hex_manager.ResetHexFilePointer();
         SendCommand(ERASE_FLASH,3,3000);
         SLEEP(500);
         for(;hex_manager.HexCurrLineNo< hex_manager.HexTotalLines;)
         {
             if(!SendCommand(PROGRAM_FLASH,3,200)) break;
             /* writing to the same line*/
//              cout<<"\r"<<"flashing:\t"<<hex_manager.HexCurrLineNo<<"::"
//                    <<hex_manager.HexTotalLines;
             int progress = (int)((double)hex_manager.HexCurrLineNo)
                                    /((double)hex_manager.HexTotalLines/100);
              cout<<"\r"<<"flashing:\t"
                 <<progress<<"%";
              /* flush all character in stdout*/
              fflush(stdout);
         }
         SLEEP(100);
         SendCommand(READ_CRC,3,500);
     }
     else{
         return false;
     }
     return true;
}

bool BootLoader::StartBootloader(void){
    unsigned char buf[1000];
    int n;
    cout<<endl<<"Enter bootloader...";
    fflush(stdout);
    WritePort((unsigned char*)"init\r\n",6);
    SLEEP(500);
    printf("issuing bl\n");
    WritePort((unsigned char*)"\xaa\x55",2);
    SLEEP(500);
    WritePort((unsigned char*)"\xaa\x55",2);
    SLEEP(500);
    WritePort((unsigned char*)"\xaa\x55",2);
    fflush(stdout);
    SLEEP(2000);
    buf[0]=0;
    n = ReadPort(buf,100);
    buf[n]=0;
    cout<<(char*)buf;
    fflush(stdout);
    return n>0;
}

bool BootLoader::SendByProtocol(uint8_t adr,uint8_t command,uint32_t parameter)
{
   static unsigned char buf[8];
   buf[0]= '\xAA';
   buf[1]= adr;

   buf[3] = command;
   buf[4] = ((uint8_t*)(&parameter))[0];
   buf[5] = ((uint8_t*)(&parameter))[1];
   buf[6] = ((uint8_t*)(&parameter))[2];
   buf[7] = ((uint8_t*)(&parameter))[3];
   buf[2] = buf[4]+buf[5]+buf[6]+buf[7]+buf[3];
   WritePort(buf,8);
   Sleep(100);
   int n = ReadPort(buf,8);
   if(n>0 && buf[3] == command){
       cout<<"[1]";
       return true;
   }
   return false;
}


/*
Request:

 01 10 01 21 10 10 04                              ...!...

Answer:

 01 10 01 10 01 00 10 01 10 04 04                  ...........

  */

/*******************End of file**********************************************/

