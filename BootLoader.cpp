#include "ComPort.h"
#include "Hex.h"
#include "BootLoader.h"
#include <string>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SOH 01
#define EOT 04
#define DLE 16





/****************************************************************************
 *  Receive Task
 *
 * \param
 * \param
 * \param
 * \return
 *****************************************************************************/
bool CBootLoader::ReceiveTask(void)
{
    bool res=false;
    unsigned short BuffLen;
    char Buff[255];
    RxFrameValid=false;
    BuffLen = ReadPort((char*)Buff, (sizeof(Buff) - 10));

    if(BuffLen) BuildRxFrame((unsigned char*)Buff, BuffLen);
    if(RxFrameValid)
    {
        // Valid frame is received.
        // Disable further retries.
        RxFrameValid = false;
        // Handle Response
        HandleResponse();
        res=true;
    }
    else
    {
        // Retries exceeded. There is no reponse from the device.
        if(NoResponseFromDevice)
        {
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
void CBootLoader::HandleNoResponse(void)
{
    // Handle no response situation depending on the last sent command.
    switch(LastSentCommand)
    {
    case READ_BOOT_INFO:
        std::cout<<"version:";
        break;
    case ERASE_FLASH:
        std::cout<<"erase:";
        break;
    case PROGRAM_FLASH:
        std::cout<<"program:";
        break;
    case READ_CRC:
        std::cout<<"verify(crc):";
        break;
    }
    std::cout<<" doesn't responding"<<std::endl;
}

/****************************************************************************
 *  Handle Response situation
 *
 * \param
 * \param
 * \param
 * \return
 *****************************************************************************/
void CBootLoader::HandleResponse(void)
{
    unsigned char cmd = RxData[0];
    char majorVer = RxData[3];
    char minorVer = RxData[4];


    switch(cmd)
    {
    case READ_BOOT_INFO:
        std::cout<<"version:ok" <<std::endl;
        printf("ver:%d.%d\n",majorVer,minorVer);
        break;
    case ERASE_FLASH:
        std::cout<<"erase:ok" <<std::endl;
        break;
    case READ_CRC:
        // Notify main window that command received successfully.
        std::cout<<"verify:ok" <<std::endl;
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
void CBootLoader::BuildRxFrame(unsigned char *buff, unsigned short buffLen)
{

    static bool Escape = false;
    unsigned short crc,crcCalc;


    while((buffLen > 0) && (RxFrameValid == false))
    {
        buffLen --;
        if(RxDataLen >= (sizeof(RxData)-2))
        {
            RxDataLen = 0;
        }

        switch(*buff)
        {


        case SOH: //Start of header
            if(Escape)
            {
                // Received byte is not SOH, but data.
                RxData[RxDataLen++] = *buff;
                // Reset Escape Flag.
                Escape = false;
            }
            else
            {
                // Received byte is indeed a SOH which indicates start of new frame.
                RxDataLen = 0;
            }
            break;

        case EOT: // End of transmission
            if(Escape)
            {
                // Received byte is not EOT, but data.
                RxData[RxDataLen++] = *buff;
                // Reset Escape Flag.
                Escape = false;
            }
            else
            {
                // Received byte is indeed a EOT which indicates end of frame.
                // Calculate CRC to check the validity of the frame.
                if(RxDataLen > 1)
                {
                    crc = (RxData[RxDataLen-2]) & 0x00ff;
                    crc = crc | ((RxData[RxDataLen-1] << 8) & 0xFF00);
                    crcCalc = CHexManager::CalculateCrc(RxData, (RxDataLen-2));
                    if((crcCalc == crc) && (RxDataLen > 2))
                    {
                        // CRC matches and frame received is valid.
                        RxFrameValid = true;
                    }
                }
            }
            break;


        case DLE: // Escape character received.
            if(Escape)
            {
                // Received byte is not ESC but data.
                RxData[RxDataLen++] = *buff;
                // Reset Escape Flag.
                Escape = false;
            }
            else
            {
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
char Buff[1000];

bool CBootLoader::SendCommand(char cmd, unsigned short Retries, unsigned short DelayInMs)
{


    unsigned short crc;

    unsigned int StartAddress,  Len;
    unsigned short BuffLen ;
    unsigned short HexRecLen;
    uint8_t totalRecords ;
    NoResponseFromDevice=true;

    while(Retries--)
    {
        if(!Retries) this->NoResponseFromDevice=false;
        // Store for later use.

        LastSentCommand = cmd;
        BuffLen=0;
        totalRecords = 10;

        switch((unsigned char)cmd)
        {
        case READ_BOOT_INFO:
            Buff[BuffLen++] = cmd;
            TxRetryDelay = DelayInMs; // in ms
            break;

        case ERASE_FLASH:
            Buff[BuffLen++] = cmd;
            TxRetryDelay = DelayInMs; // in ms
            break;

        case JMP_TO_APP:
            Buff[BuffLen++] = cmd;
            Retries = 0;
            TxRetryDelay = 10; // in ms
            break;

        case PROGRAM_FLASH:
            Buff[BuffLen++] = cmd;
            HexRecLen = HexManager.GetNextHexRecord(&Buff[BuffLen], (sizeof(Buff) - 5));
            if(HexRecLen == 0)
            {
                //Not a valid hex file.
                return false;
            }

            BuffLen = BuffLen + HexRecLen;
            while(totalRecords)
            {
                HexRecLen = HexManager.GetNextHexRecord(&Buff[BuffLen], (sizeof(Buff) - 5));
                BuffLen = BuffLen + HexRecLen;
                totalRecords--;
            }
            TxRetryDelay = DelayInMs; // in ms
            break;

        case READ_CRC:
            Buff[BuffLen++] = cmd;
            HexManager.VerifyFlash((unsigned int*)&StartAddress, (unsigned int*)&Len, (unsigned short*)&crc);
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
            TxRetryDelay = DelayInMs; // in ms
            break;

        default:
            return false;
            break;

        }//switch
        // Calculate CRC for the frame.
        crc = CHexManager::CalculateCrc(Buff, BuffLen);
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
        while(!ComPort.bytesAvailable() && msstep--) usleep(1000);
        usleep(10000);
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
unsigned short CBootLoader::CalculateFlashCRC(void)
{
    unsigned int StartAddress,  Len;
    unsigned short crc;
    HexManager.VerifyFlash((unsigned int*)&StartAddress, (unsigned int*)&Len, (unsigned short*)&crc);
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
void CBootLoader::OpenPort(uint8_t portType)
{

    PortSelected = portType;
    switch(portType)
    {
    case USB:

        break;

    case COM:
        ComPort.OpenComPort();
        break;

    case ETH:

        break;

    }

}

/****************************************************************************
 *  Get communication port status.
 *
 * \param
 * \return true: Port opened.
           false: Port closed.
 *****************************************************************************/
bool CBootLoader::isPortOpen(uint8_t PortType)
{
    bool result;

    switch(PortType)
    {
    case USB:
        break;

    case COM:
        result = ComPort.GetComPortOpenStatus();
        break;

    case ETH:
        break;
    }


    return result;

}

/****************************************************************************
 *  Closes the communication port (USB/COM/ETH)
 *
 * \param
 * \return
 *****************************************************************************/
void CBootLoader::ClosePort()
{

    switch(PortSelected)
    {
    case USB:
        break;

    case COM:
        ComPort.CloseComPort();
        break;

    case ETH:
        break;
    }
}


/****************************************************************************
 *  Write communication port (USB/COM/ETH)
 *
 * \param Buffer, Len
 * \return
 *****************************************************************************/
void CBootLoader::WritePort(char *buffer, int bufflen)
{

//    switch(PortSelected)
//    {
//    case USB:

//        break;

//    case COM:
        ComPort.SendComPort(buffer, bufflen);
//        break;

//    case ETH:

//        break;
//    }
}


/****************************************************************************
 *  Read communication port (USB/COM/ETH)
 *
 * \param Buffer, Len
 * \return
 *****************************************************************************/
unsigned short CBootLoader::ReadPort(char *buffer, int bufflen)
{
    int bytesRead;
//    switch(PortSelected)
//    {
//    case USB:

//        break;

//    case COM:
        bytesRead =ComPort.ReadComPort(buffer, bufflen);
//        break;

//    case ETH:

//        break;
//    }

        return (unsigned short) bytesRead;
}


bool CBootLoader::job(int &command,int &baudrate,std::string &fname,std::string &pname)
{	
    bool file_loaded;
    ComPort.setBaudRate(baudrate);
    ComPort.setPortName(pname);
    if(!fname.empty())
    {
        file_loaded = HexManager.LoadHexFile(fname);
    /*D*/ if(file_loaded) std::cout<<"file opened"<<std::endl;
    else std::cout<<"file not opened"<<std::endl;
    }
    if(ComPort.OpenComPort())
    {

        switch(command)
        {
        case cmdVersion:

            this->SendCommand(READ_BOOT_INFO,2,200);
            break;
        case cmdErase:
            this->SendCommand(ERASE_FLASH,3,1000);
            break;
        case cmdVerify:
            if(file_loaded)
                        this->SendCommand(READ_CRC,3,500);
            break;
        case cmdStartBl:
            printf("init\n");
            fflush(stdout);
            WritePort((const char*)"init\r\n",6);
            usleep(2000*1000);
            WritePort((const char*)"\xaa\x55",2);
            printf("aa 55\n");
            fflush(stdout);
            usleep(2000*1000);
            RxData[0]=0;
            RxData[ReadPort(RxData,100)]=0;
            printf(RxData);
            fflush(stdout);
            break;
        case cmdProgram:
            if(file_loaded)
            {
                HexManager.ResetHexFilePointer();
                SendCommand(ERASE_FLASH,3,1000);
                usleep(500*1000);
                for(;HexManager.HexCurrLineNo< HexManager.HexTotalLines;)
                {
                    SendCommand(PROGRAM_FLASH,3,200);
                     printf("%d/%d\r",HexManager.HexTotalLines,HexManager.HexCurrLineNo);
                     fflush(stdout);
                }
                std::cout<<std::endl;
                usleep(100*1000);
                SendCommand(READ_CRC,3,500);
            }
            else std::cout<<"file not loaded!"<<std::endl;
            break;
        case cmdRun:
            this->SendCommand(JMP_TO_APP,3,500);
            break;
        }
    }

}


/*******************End of file**********************************************/
