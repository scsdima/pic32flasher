#include "simple_crypt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void crypt_simple(char *inp, unsigned long inplen, char* key , unsigned long keylen )
{
    //we will consider size of sbox 256 bytes
    //(extra byte are only to prevent any mishep just in case)
    char Sbox[257], Sbox2[257];
    unsigned long i, j, t, x;    
    char temp , k;
    temp = i = j = k = t =  x = 0;

    //always initialize the arrays with zero
    memset(Sbox,0, sizeof(Sbox));
    memset(Sbox2,0, sizeof(Sbox2));

    //initialize sbox i
    for(i = 0; i < 256U; i++)
    {
        Sbox[i] = (char)i;
    }

    j = 0;
    //whether user has sent any inpur key
    if(! keylen) return ;
   
        //initialize the sbox2 with user key
        for(i = 0; i < 256U ; i++)
        {
            if(j == keylen)
            {
                j = 0;
            }
            Sbox2[i] = key[j++];
        }    
   

    j = 0 ; //Initialize j
    //scramble sbox1 with sbox2
    for(i = 0; i < 256; i++)
    {
        j = (j + (unsigned long) Sbox[i] + (unsigned long) Sbox2[i]) % 256U ;
        temp =  Sbox[i];                    
        Sbox[i] = Sbox[j];
        Sbox[j] =  temp;
    }

    i = j = 0;
    for(x = 0; x < inplen; x++)
    {
        //increment i
        i = (i + 1U) % 256U;
        //increment j
        j = (j + (unsigned long) Sbox[i]) % 256U;

        //Scramble SBox #1 further so encryption routine will
        //will repeat itself at great interval
        temp = Sbox[i];
        Sbox[i] = Sbox[j] ;
        Sbox[j] = temp;

        //Get ready to create pseudo random  byte for encryption key
        t = ((unsigned long) Sbox[i] + (unsigned long) Sbox[j]) %  256U ;

        //get the random byte
        k = Sbox[t];

        //xor with the data and done
        inp[x] = (inp[x] ^ k);
    }    
}
