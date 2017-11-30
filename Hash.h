#ifndef H_HASH
#define H_HASH

#define BLOOMSIZE 240000000
//extern bool BloomFilter[];
unsigned int RSHash(const char* str,unsigned int length);
unsigned int JSHash(const char* str,unsigned int length);
unsigned int PJWHash(const char* str, unsigned int length);
unsigned int ELFHash(const char* str, unsigned int length);
unsigned int BKDRHash(const char* str, unsigned int length);
unsigned int SDBMHash(const char* str, unsigned int length);
unsigned int DJBHash(const char* str, unsigned int length);
unsigned int DEKHash(const char* str, unsigned int length);
unsigned int BPHash(const char*str ,unsigned int length);
unsigned int FNVHash(const char*str ,unsigned int length);
unsigned int APHash(const char* str, unsigned int length);
int checkIfNotExist(char* str,unsigned int length);

#endif
