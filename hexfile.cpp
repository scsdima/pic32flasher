#include "hexfile.h"

HexFile::HexFile(){}

HexFile::~HexFile(){}

/****************************************************************************
 * Open file
 *
 * \param f_name    file name to open
 * \param option    options line
 * \param mode		modes to open file
 *      FilePreload  - file will be preloaded to memory
 *      FileReadDirectly - directly read from file
 * \return        true = file is opened successfully
 *****************************************************************************/
bool HexFile::open(const char *f_name,const char *option, HexFileMode f_mode ){
    if(this->f_p != NULL ){
        close();
    }
    if(!isOpened()) {
    this->f_p = NULL;
    this->f_opened = false;
    switch (mode) {
    case FilePreload:
        this->f_p = fopen(f_name,option);
        this->f_opened = (this->f_p != NULL);
        preloadFile();
        break;
    case FileReadDirectly:
        this->f_p = fopen(f_name,option);
        this->f_opened = (this->f_p != NULL);
        break;
    default:
        break;
    }
    return isOpened();
    }
    return false;
}

/****************************************************************************
 * read line from file
 *
 * \param data     Pointer to a buffer of \a data_len bytes.
 * \param maxsize	max	number of bytes to read
 * \return         if ok returns data
 *****************************************************************************/
char *HexFile::gets(char *data,size_t maxsize){
    char * result = NULL;
    if(isOpened()){
        switch (mode) {
        case FilePreload:
            {
            /* reading one line of yext from memory*/
                size_t pos = this->f_pos;
                unsigned char ch;
                while( ch != '\r' && ch != '\n'
                       && pos < this->f_size && maxsize !=0 ){
                    data[pos] = this->f_buf[pos];
                    pos++;
                    maxsize--;
                }
                while( ch == '\r' || ch == '\n'
                       && pos < this->f_size){
                    pos++;
                }
                this->f_pos += pos+2;
            }
            break;
        case FileReadDirectly:
            result= fgets(data, maxsize, this->f_p);
            break;
        default:
            break;
        }
    }
    return result;
}

/****************************************************************************
 * change posistion in file
 *
 * \param offset  offset in file
 * \param origin form where to make offset
 * \return        if ok returns 0
 *****************************************************************************/
int HexFile::seek(long int offset ,int origin){
    int result=-1;
    if(isOpened()){
        switch (mode) {
        case FilePreload:
        {
            size_t new_pos=-1;
            switch(origin){
                case SEEK_SET:
                    new_pos = offset;
                    break;
                case SEEK_END:
                    new_pos = f_size-offset;
                    break;
                case SEEK_CUR:
                    new_pos =(this->f_pos +offset);
                    break;
                default:break;
            }
            if( new_pos < this->f_size
                    && (new_pos >= 0))  {
                this->f_pos =new_pos;
                result = 0;
            }
        }
            break;
        case FileReadDirectly:
            result =  fseek(this->f_p,offset,origin);
            break;
        default:
            break;
        }
    }
    return result;
}

/****************************************************************************
 * close the file
 *
 * \param
 * \return  int if ok returns 0
 *****************************************************************************/
int HexFile::close(){
    int result = -1;
    if(isOpened()){
        switch (mode) {
        case FilePreload:
            delete [] this->f_buf;
            result = fclose(this->f_p);
            break;
        case FileReadDirectly:
            result = fclose(this->f_p);
            break;
        default:
            break;
        }
    }
    this->f_p  = NULL;
    this->f_opened = true;
    return result;
}

/****************************************************************************
 * end of file
 *
 * \param
 * \return  int if ok returns 0
 *****************************************************************************/
int HexFile::eof(){
    int result;
    switch (mode) {
    case FilePreload:
        break;
    case FileReadDirectly:
        result = feof(this->f_p);
        break;
    default:
        break;
    }
    return result;
}
/****************************************************************************
 * is file opened
 *
 * \param
 * \return    if file is currently opend
 *****************************************************************************/
bool HexFile::isOpened(){
    return f_opened;
}

/****************************************************************************
 * preloads data from file to memory
 *
 * \param
 * \return  int if ok returns 0
 *****************************************************************************/
int HexFile::preloadFile(){
    int result = -1;
    fseek (this->f_p, 0, SEEK_END);// goto end
    this->f_size = ftell(this->f_p);
    this->f_buf = new unsigned char(this->f_size);
    fread(this->f_buf, this->f_size,1, this->f_p);
    fclose(this->f_p);
    this->f_pos = 0;
    return result;
}
