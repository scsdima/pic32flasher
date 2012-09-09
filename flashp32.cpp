#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include "BootLoader.h"

using namespace std;


void showInfo()
{
	cout<<"*****************************************\n"
		<<"* Microchip PIC32 SerialPort bootloader *\n"
		<<"*****************************************\n"
		<<"use: fp32 <COMMAND> [-p<SerialPort>] [-b<BAUDRATE>]  [-f<FILENAME>] "<<endl;
}

int main( int argc, char *argv[] ) { 

    int i,cmd=-1;
	int baud=0;
	string filename,portname;
	char *str;

	CBootLoader bootloader;
	if(argc<=1)
		{
			showInfo();
			return 0;
		}

		for(int ci=1;ci<argc;ci++)
		{
			str=argv[ci];
			if(str[0]!='-')
			{//command
				for(i=0;i<CMD_TXT_MAX;i++)
				{
                    if(strcmp(str,(const char*)txtCmd[i])==0)
					{

                        cmd=i;
						break;
					}
				}	
			}
			else 
				{
					if(str[1]=='f')
					{//file
						filename=string(&str[2]);
					}
					else if(str[1]=='b')
					{//baud
						for(i=0;i<BAUD_TXT_MAX;i++)
						{
							if(strstr(&str[2],(const char*)txtBaud[i])) {
								baud=i;
								break;
							}
						}	
					}
					else if(str[1]=='p')
					{//serial port
						portname=string(&str[2]);	
					}
					
				}//else
				
		}//for
        if(cmd>=0)
        bootloader.job(cmd,baud,filename,portname);
        else cout <<"command not recognized"<<std::endl;
	return 1;
}
