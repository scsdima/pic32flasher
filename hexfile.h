#ifndef HEXFILE_H
#define HEXFILE_H


class HexFile{
public:
    HexFile(const string &physical_filename);
    ~HexFile();
    typedef enum {Filepreload,FileReadDirectly} FileMode;
    open();
    gets();
    seek();
    read();
    close();
private:
    FileMode mode;
    string phy_filename;
    size_t phy_size;
    size_t phy_position;
    preloadPhyFile();
};
#endif // HEXFILE_H
