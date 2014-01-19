#ifndef HEXFILE_H
#define HEXFILE_H
#include "iostream"
#include "stdint.h"
#include "stdio.h"
#include "unistd.h"

using namespace std;

class HexFile{

public:
    HexFile();
    ~HexFile();
    typedef enum {FileBin,FileHex} HexFileMode;
    bool open(const char *f_name,HexFileMode f_mode = FileHex);
    char *gets(char *data,size_t maxsize);
    int seek(long int offset ,int origin);
    int eof();
    int close();
    bool isOpened() const;

private:
    bool f_opened;
    FILE *f_p;
    HexFileMode mode;
    size_t f_size;
    long int f_pos;
    unsigned char *f_buf;
    bool preloadFile();
};
#endif // HEXFILE_H
