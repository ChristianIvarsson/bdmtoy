#ifndef UTILS_H_
#define UTILS_H_
#ifdef __cplusplus 
extern "C" {
#endif
#include <stdint.h>
#include <stdio.h>

#define WordSwap(a)      ((((a)&0xffff)<<16)|((((a)&0xFFFF0000)>>16)))
#define ByteSwap(a)      (((a)&0xff)<<24|(((a)>>8)&0xff)<<16|(((a)>>16)&0xff)<<8|(((a)>>24)&0xff))
#define ByteSwapWord(a)  ((((a)>>16)&0xff)<<24|(((a)>>24)&0xff)<<16|((a)&0xff)<<8|(((a)>>8)&0xff))
#define BIT_REVERSE(W,B) (1<<(((W)-1)-(B)))

uint32_t *utl_md5Hash        (const void     *data , const uint32_t  noBytes); // Hash of local buffer
uint32_t *utl_md5FinalTarget (const uint32_t *keys , const uint32_t  noBytes); // Finalize keys from remote target
uint32_t  utl_compareMD5     (const uint32_t *keysA, const uint32_t *keysB  ); // Compare two arrays of keys. 1 = not the same
uint32_t  utl_checksum       (const void     *data ,       uint32_t  noBytes); // Hash a checksum-32 (or 8/16 if you AND it)
uint32_t utl_checksum32block (const void     *data ,       uint32_t  noBytes); // Hash a checksum where every addition is four bytes instead of one
#ifdef __cplusplus 
}
#endif
#endif
