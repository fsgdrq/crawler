#include <string.h>
#include <stdlib.h>

#include "http.h"

#include <iostream>
int m_iSocketFd = INVALID_SOCKET;
char* HttpHeadCreate(const char* strUrl)
{
	if(strUrl==NULL||strlen(strUrl)>BUFSIZE)
	{
		return NULL;
	}
	char* strHost = GetHostAddrFromUrl(strUrl);

	char* strParam = GetParamFromUrl(strUrl);
	if(strHost==NULL||strParam==NULL)
	{
		return NULL;
	}
	char* strHttpHead = (char*)malloc(BUFSIZE);
	memset(strHttpHead, 0, BUFSIZE);

	strcat(strHttpHead, "GET");
	strcat(strHttpHead, " /");
	strcat(strHttpHead, strParam);

	strcat(strHttpHead, " HTTP/1.1\r\n");
	strcat(strHttpHead, "Accept: */*\r\n");
	strcat(strHttpHead, "Accept-Language: cn\r\n");
	strcat(strHttpHead, "User-Agent: Mozilla/4.0\r\n");
	strcat(strHttpHead, "Host: ");
	strcat(strHttpHead, strHost);
	strcat(strHttpHead, "\r\n");
	strcat(strHttpHead, "Cache-Control: no-cache\r\n");
	strcat(strHttpHead, "Connection: Keep-Alive\r\n");
	strcat(strHttpHead, "\r\n\r\n");
	if(strHost!=NULL)
	{
		free(strHost);
		strHost=NULL;
	}
	if(strParam!=NULL)
	{
		free(strParam);
		strParam=NULL;
	}

	return strHttpHead;
}

char* HttpDataTransmit(const int iSockFd)
{
	char* buf = (char*)malloc(BUFSIZE);
	char* returnbuf = (char*)malloc(WHOLE_PAGE_SIZE);
	if(buf==NULL||returnbuf==NULL)
	{
		std::cout<<"------------------alloc memory wrong-----------------------";
		return NULL;
	}
	memset(buf, 0, BUFSIZE);
	memset(returnbuf,0,WHOLE_PAGE_SIZE);
	while (1)
	{
		int ret = recv(iSockFd, (void *)buf, BUFSIZE, MSG_DONTWAIT);
		if (ret == 0) 
		{
			if(buf!=NULL)
			{
				free(buf);
				buf=NULL;
			}

			return returnbuf;

		}
		else if (ret > 0)
		{
			strcat(returnbuf,buf);
			memset(buf,0,BUFSIZE);
		}
		else if (ret < 0) 
		{
			if (errno == EWOULDBLOCK) {
				if(buf!=NULL)
				{
					free(buf);
					buf=NULL;
				}
				return returnbuf;
			}
			else {
				memset(buf,0,BUFSIZE);
				continue;
			}
		}
	}
}

char* GetHostAddrFromUrl(const char* strUrl)
{
	char url[URLSIZE];
	memset(&url,0,URLSIZE);
	if(strUrl==NULL||strlen(strUrl)>URLSIZE)
	{
		return NULL;
	}
	strcpy(url, strUrl);

	char* strAddr = strstr(url, "http://");
	if (strAddr == NULL) {
		strAddr = strstr(url, "https://");
		if (strAddr != NULL) {
			strAddr += 8;
		}
	}
	else {
		strAddr += 7;
	}

	if (strAddr == NULL) {
		strAddr = url;
	}
	int iLen = strlen(strAddr);
	char* strHostAddr = (char*)malloc(iLen + 1);
	memset(strHostAddr, 0, iLen + 1);
	for (int i = 0; i<iLen + 1; i++) {
		if (strAddr[i] == '/') {
			break;
		}
		else {
			strHostAddr[i] = strAddr[i];
		}
	}

	return strHostAddr;
}


char* GetParamFromUrl(const char* strUrl)
{
	char url[URLSIZE] = { 0 };
	strcpy(url, strUrl);

	char* strAddr = strstr(url, "http://");
	if (strAddr == NULL) {
		strAddr = strstr(url, "https://");
		if (strAddr != NULL) {
			strAddr += 8;
		}
	}
	else {
		strAddr += 7;
	}

	if (strAddr == NULL) {
		strAddr = url;
	}
	int iLen = strlen(strAddr);
	char* strParam = (char*)malloc(iLen + 1);
	memset(strParam, 0, iLen + 1);
	int iPos = -1;
	for (int i = 0; i<iLen + 1; i++) {
		if (strAddr[i] == '/') {
			iPos = i;
			break;
		}
	}
	if (iPos == -1) {
		strcpy(strParam, "");;
	}
	else {
		strcpy(strParam, strAddr + iPos + 1);
	}
	return strParam;
}

char* GetIPFromUrl(const char* strUrl)
{
	char* strHostAddr = GetHostAddrFromUrl(strUrl);
	int iLen = strlen(strHostAddr);
	char* strAddr = (char*)malloc(iLen + 1);
	memset(strAddr, 0, iLen + 1);
	int iCount = 0;
	int iFlag = 0;
	for (int i = 0; i<iLen + 1; i++) {
		if (strHostAddr[i] == ':') {
			break;
		}

		strAddr[i] = strHostAddr[i];
		if (strHostAddr[i] == '.') {
			iCount++;
			continue;
		}
		if (iFlag == 1) {
			continue;
		}

		if ((strHostAddr[i] >= '0') || (strHostAddr[i] <= '9')) {
			iFlag = 0;
		}
		else {
			iFlag = 1;
		}
	}
	free(strHostAddr);

	if (strlen(strAddr) <= 1) {
		return NULL;
	}

	if ((iCount == 3) && (iFlag == 0)) {
		return strAddr;
	}
	else {
		struct hostent *he = gethostbyname(strAddr);
		free(strAddr);
		if (he == NULL) {
			return NULL;
		}
		else {
			struct in_addr** addr_list = (struct in_addr **)he->h_addr_list;
			for (int i = 0; addr_list[i] != NULL; i++) {
				return inet_ntoa(*addr_list[i]);
			}
			return NULL;
		}
	}
}
