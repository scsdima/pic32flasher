#ifndef SIMPLE_CRYPT_H
#define SIMPLE_CRYPT_H

	#ifdef __cplusplus
		extern "C" {
	#endif
 
		void crypt_simple(char *inp, unsigned long inplen, char* key , unsigned long keylen );
	#ifdef __cplusplus
		}
	#endif

#endif
