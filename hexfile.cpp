#include "hexfile.h"
#include "simple_crypt.h"

HexFile::HexFile(){
    this->f_opened=false;
    this->f_p=NULL;
}

HexFile::~HexFile(){
if(this->f_buf != NULL)
    delete this->f_buf;
}

/****************************************************************************
 * Open file
 *
 * \param f_name    file name to open
 * \param option    options line
 * \param mode		modes to open file
 *      FileBin  - file will be preloaded to memory
 *      FileHex - directly read from file
 * \return        true = file is opened successfully
 *****************************************************************************/
bool HexFile::open(const char *f_name, HexFileMode f_mode ){
    if(this->f_p != NULL ){
        close();
    }
    this->mode = f_mode;
    if(!isOpened()) {
    switch (mode) {
    case FileBin:
        this->f_p = fopen(f_name,"r");
        this->f_opened = preloadFile();
        break;

    case FileHex:
        this->f_p = fopen(f_name,"rb");
        this->f_opened = (this->f_p != NULL);
        break;

    default:
        break;
    }    
    }
    return isOpened();

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

        case FileBin:
            {            
            /* reading one line of yext from memory*/
                long int new_pos = (long int)this->f_pos;
                long int di=0;
                while( new_pos < (long int)this->f_size && maxsize !=0 ){
                    unsigned char ch = this->f_buf[new_pos];
                    if(ch == '\r' || ch == '\n') break;
                    data[di++] = ch;
                    new_pos++;
                    maxsize--;
                }
                data[di] = 0;
                while(new_pos < (long int)this->f_size){
                    unsigned char ch = this->f_buf[new_pos];
                    if(ch != '\r'  && ch != '\n')  break;
                    new_pos++;
                }
                this->f_pos = new_pos;
            }
            break;

        case FileHex:
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

        case FileBin: {
            long int new_pos=-1;
            switch(origin){
                case SEEK_SET:
                    new_pos = offset;
                    break;
                case SEEK_END:
                    new_pos = (long int)f_size-offset;
                    break;
                case SEEK_CUR:
                    new_pos =(this->f_pos +offset);
                    break;
                default:break;
            }
            if( new_pos < (long int)this->f_size
                    && (new_pos >= 0))  {
                this->f_pos = new_pos;
                result = 0;
            }
        }
            break;

        case FileHex:
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
        case FileBin:
            delete [] this->f_buf;
            this->f_buf=NULL;
            result = 0;
            break;

        case FileHex:
            result = fclose(this->f_p);
            break;

        default:
            break;
        }
    }    
    this->f_opened = false;
    return result;
}

/****************************************************************************
 * end of file
 *
 * \param
 * \return  int if ok returns 0
 *****************************************************************************/
int HexFile::eof(){
    int result = -1;
    switch (mode) {
    case FileBin:
        result = (this->f_pos >= (long int)this->f_size);
        break;

    case FileHex:
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
bool HexFile::isOpened() const{
    return f_opened;
}

/****************************************************************************
 * preloads data from file to memory
 *
 * \param
 * \return  int if ok returns 0
 *****************************************************************************/
bool HexFile::preloadFile(){
    bool result = false;
    unsigned char *input_data;
    size_t input_size,output_size;
    unsigned char *output_data;
    fseek (this->f_p, 0, SEEK_END);// goto end
    input_size = (size_t)ftell(this->f_p);
    if(input_size >0){
        input_data =new unsigned char[input_size];
        /* set to begining of the file*/
        fseek (this->f_p, 0, SEEK_SET);
        /* read all information from file*/
        result = ((size_t)fread(input_data, 1,input_size, this->f_p) == this->f_size);
        output_size= decode_data(&output_data, input_data, input_size);
        if(output_data != NULL && output_size >0){
            this->f_buf = output_data;
            this->f_size =  output_size;
            result |= true;
        }
        else{
            result = false;
        }
        delete [] input_data;
    }
    /* physical file we can close*/
    fclose(this->f_p);
    this->f_p = 0;
    /* set to begining of file*/
    this->f_pos = 0;
    return result;
}
