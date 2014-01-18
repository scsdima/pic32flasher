#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include "bootloader.h"



using namespace std;

/*alter*/
static const char *const operation_list[] = {"version","erase","program","run","startbl","setpsw","setid"};

BootLoader::Jobs  findJob(const char str[]){
    BootLoader::Jobs job_n = BootLoader::jobNothing;
    uint8_t i=0;
    for(; i<BootLoader::JobsCount; i++)    {
        if(strcmp(str, (const char*)operation_list[i]) == 0)        {
            job_n = (BootLoader::Jobs)i;            
        }
    }
    return job_n;
}

int main( int argc, char *argv[] ) { 

    BootLoader::Jobs job = BootLoader::jobNothing;
    BaudRate baudrate = Baud19200;
    const char *str;
    string filename ="";
    string portname ="COM1";
    uint32_t value;
    void *data = NULL;
    bool data_set= false;

    if(argc <= 1)    {
        cout<<"*****************************************\n"
           <<"* Microchip PIC32 SerialPort bootloader *\n"
          <<"*****************************************\n"
         <<"use: fp32 <COMMAND> [-p<SerialPort>] [-b<BAUDRATE>]  [-f<FILENAME>] "
        <<"version,erase,program,run,startbl\n" <<endl;
        return 0;
    }

    for(uint8_t ci=1; ci < argc; ci++)    {
        str = argv[ci];
        if(str[0] != '-')/* if no '-' means that this is command */ {
            job = findJob(&str[0]);
        }
        else
        {
            switch (str[1]){
            case 'f':/* file */
                filename = string(&str[2]);
                data = (void*)&filename;
                break;
            case 'b':/* baudrate dafault =19200*/                                
                sscanf(&str[2],"%d",&baudrate);
                break;
            case 'p':/* portname*/                
                portname = string(&str[2]);
                break;
            case 'd':/* data data*/
                data_set =true;
                value = 0;
                sscanf(&str[2],"%d",&value);                
                data = (void*)&value;
                break;
            default:
                break;
            }
        }//else

    }//for

    if(job == BootLoader::jobNothing){
        cout << endl<<"Command not recognized" << endl;
        return -1;
    }    
    if(job == BootLoader::jobWriteId ||job == BootLoader::jobWritePassword){
        if(!data_set) {
            cout <<endl<<"data not set"<<endl;
            return (-1);
        }
    }
    BootLoader bootloader;        
    bootloader.runJob(job, baudrate, data, portname);

    return 0;
}
