#include "hexfile.h"

HexFile::HexFile(const string &physical_filename)
{
    this->phy_filename =physical_filename;
    preloadPhyFile();
}

HexFile::~HexFile(){}

typedef enum {Filepreload,FileReadDirectly} FileMode;
HexFile::open(){

}

HexFile::gets(){

}

HexFile::seek(){

}

HexFile::read(){

}

HexFile::close(){

}

HexFile::preloadPhyFile(){
    switch(mode ){
    case Filepreload:
    break;
    case FileReadDirectly:
    break;
    default:
        break;
    }
}
