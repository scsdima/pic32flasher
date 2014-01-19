
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>


#define KEYS_USE 10
#define HEADER_SIZE sizeof(_H)

#pragma pack(push,1)
typedef struct {
    uint32_t size;
    uint8_t keys[KEYS_USE]  ;
}_H;
#pragma pack(pop)



static _H h;

long int decode_data(unsigned char **output,unsigned char *input,long int input_size){
    int key_i = 0;
    long int i ;
    long int output_size ;
    unsigned char *output_data = NULL;
    output_size = input_size -HEADER_SIZE;
    output_data = new unsigned char [output_size];
    memcpy((unsigned char*)&h,input,HEADER_SIZE);
    if(h.size == (uint32_t)input_size){
        for(i=0; i < output_size; i++){
            key_i = (key_i+1) % KEYS_USE;
            output_data[i] = input[HEADER_SIZE+i] ^(h.keys[key_i]+key_i);
        }
    }
    else{
        output_size=0;
    }
    *output = output_data;
    return output_size;
}
