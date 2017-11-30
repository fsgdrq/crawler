#ifndef H_HTTP
#define H_HTTP

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#define BUFSIZE 45920                      //1448*40
#define WHOLE_PAGE_SIZE 1000000
#define URLSIZE 256
#define INVALID_SOCKET -1

char* HttpHeadCreate(const char* strUrl);
char* GetHostAddrFromUrl(const char* strUrl);
char* GetIPFromUrl(const char* strUrl);
char* GetParamFromUrl(const char* strUrl);
char* HttpDataTransmit(const int iSockFd);

#endif