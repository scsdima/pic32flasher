#include "config.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include "BootLoader.h"



using namespace std;

BootLoader::Jobs  findJob(const char str[]){
    BootLoader::Jobs job_n = BootLoader::jobNothing;
    uint8_t i=0;
    for(; i<BootLoader::JobsCount; i++)    {
        if(strcmp(str, (const char*)txtCmd[i]) == 0)        {
            job_n = (BootLoader::Jobs)i;
            return job_n;
        }
    }
}

BaudRate_t findBaudRate(const char str[]){
    BaudRate_t baudrate= (BaudRate_t)0;
    uint8_t i=0;
    for(; i < BAUD_TXT_MAX; i++)    {
        if(strstr(str, (const char*)Baud_txt[i])) {
            baudrate = (BaudRate_t)i;
            return baudrate;
        }
    }
}

int main( int argc, char *argv[] ) { 

    BootLoader::Jobs job = BootLoader::jobNothing;
    BaudRate_t baudrate = B19200;
    char *str;
    string filename ="";
    string portname ="COM1";

    if(argc <= 1)    {
        cout<<"*****************************************\n"
           <<"* Microchip PIC32 SerialPort bootloader *\n"
          <<"*****************************************\n"
         <<"use: fp32 <COMMAND> [-p<SerialPort>] [-b<BAUDRATE>]  [-f<FILENAME>] "
        <<"version,erase,check,program,run,startbl\n" <<endl;
        return 0;
    }

    for(uint8_t ci=1; ci < argc; ci++)    {
        str = argv[ci];
        if(str[0] != '-')/* if no '-' means that this is command */ {
            job = findJob(&str[0]);
            break;
        }
        else
        {
            switch (str[1]){
            case 'f':/* file */
                filename = string(&str[2]);
                break;
            case 'b':/* baudrate dafault =19200*/
                baudrate= findBaudRate(&str[2]);
                break;
            case 'p':/* portname*/
                portname = string(&str[2]);
                break;
            default:
                break;
            }
        }//else

    }//for

    if(job == BootLoader::jobNothing){
        cout << "Command not recognized" << endl;
        return -1;
    }
    BootLoader bootloader;
    bootloader.runJob(job, baudrate, filename, portname);

    return 0;
}
