#include "crawler.h"
#include <iostream>
#include <fstream>
#include <time.h>
#include <math.h>
using namespace std;

typedef map<string,int> MAP_PAGE;

MAP_PAGE mapPage[256];
int nPageCount[256]={0};

int linknum(0);
int nWebNum(0);
int nBadWebNum(0);
int nTimeOut(0);
int nURLNum(0);
int nURLNum2(0);
int nConnNum(0);

fstream ofile;
fstream ofile2;
fstream ofileLog;

queue<char*> waitLine;
map<char*,int,cmp> mapURL;

struct event_base *base;

extern bool BloomFilter[BLOOMSIZE];

int main(int argc, char* argv[])
{
	if (argc != 4) 
	{
		printf("input error\n输入格式./crawler ip port url.txt\n");
		return 0;
	}
	int port = atoi(argv[2]);
	if(port<=0)
	{
		printf("port is illegal!\n");
		return 0;
	}
	clock_t beginTime, endTime;
	double dTimeSpent;
	beginTime = clock();
	
	pRoot = new Node;
	pRoot->failNode = pRoot;
	pRoot->nIndex = -1;
	ACStateInit();

	memset(&BloomFilter,0,sizeof(BloomFilter));
	
    ofile.open(argv[3],ios::out|ios::in|ios::trunc);
	ofileLog.open("./log.txt",ios::out|ios::trunc);
	if(!(ofile.is_open()&&ofileLog.is_open()))
	{
		cout<<"open files wrong!"<<endl;
	}
	
	base = event_base_new();

	char* URL=(char*)malloc(256*sizeof(char));
	strcpy(URL,"http://news.sohu.com/");
	
	checkIfNotExist(URL,strlen(URL));                 // 把首页放入bloomfilter
	waitLine.push(URL);
	while(!waitLine.empty())
	{
		while(nConnNum<100&&(!waitLine.empty()))
		{
			URL = waitLine.front();
			waitLine.pop();
			if(strlen(URL)>256)
			{
				cout<<"size of URL is too big."<<endl;
				continue;
			}
			struRead* pstruRead;
			pstruRead = (struRead*)malloc(sizeof(struRead));
			struWrite* pstruWrite;
			pstruWrite = (struWrite*)malloc(sizeof(struWrite));
			pstruWrite->URL = URL;
			struct sockaddr_in servaddr;
			bzero(&servaddr,sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			inet_pton(AF_INET,argv[1],&servaddr.sin_addr);
			//inet_pton(AF_INET,"10.108.86.80",&servaddr.sin_addr);
			servaddr.sin_port = htons(port);
			pstruRead->fd = socket(PF_INET,SOCK_STREAM,0);
			if(connect(pstruRead->fd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0)
			{
				printf("connect wrong:%d,fd:%d\n",errno,pstruRead->fd);
			}
			else
			{
				evutil_make_socket_nonblocking(pstruRead->fd);
				nConnNum++;
				struct timeval f = {5,0};
				struct event *sockwrite = event_new(base,pstruRead->fd,EV_WRITE|EV_PERSIST|EV_TIMEOUT,cb_write,(void *)pstruWrite);
				pstruWrite->eventWrite = sockwrite;
				struct event *sockread = event_new(base,pstruRead->fd,EV_READ|EV_PERSIST,cb_read,(void *)pstruRead);
				pstruRead->eventRead = sockread;
				pstruRead->URL = URL;
				pstruRead->bInit = false;
				event_add(sockwrite,&f);
				event_add(sockread,0);
			}
		}
		event_base_loop(base,0);
		cout<<"a round"<<endl;
	}

	printf("----------------------end----------------------------\n");
	event_base_free(base);
	cout<<"URLnum: "<<nURLNum<<endl;
	cout<<"URLnum2: "<<nURLNum2<<endl;
	cout<<"BadURLnum: "<<nBadWebNum<<endl;
	cout<<"nWebNum: "<<nWebNum<<endl;
	cout<<"nTimeOut: "<<nTimeOut<<endl;
	cout<<"linknum: "<<linknum<<endl;

	rebuildUrlLink();

	endTime = clock();
	dTimeSpent = (double)(endTime - beginTime) / CLOCKS_PER_SEC;
	printf("Time spent: %.9lf seconds.\n", dTimeSpent);

	ofile.close();
	ofileLog.close();
}	

void rebuildUrlLink()
{
	ofile2.open("./log.txt",ios::in);
	char buffer[512];
	char* web1 = (char*)calloc(256,sizeof(char));
	char* web2 = (char*)calloc(256,sizeof(char));
	map<char*,int>::iterator key1;
	map<char*,int>::iterator key2;
	while(!ofile2.eof())
	{
		ofile2.getline(buffer,510);
		sscanf(buffer,"%s - %s",web1,web2);
		if(strlen(web1)==0||strlen(web2)==0)
		{
			continue;
		}
		key1 = mapURL.find(web1);
		key2 = mapURL.find(web2);
		if(key1!=mapURL.end()&&key2!=mapURL.end())
		{
			ofile<<key1->second<<'-'<<key2->second<<endl;
		}
	}
	free(web1);
	free(web2);
	ofile2.close();
}

void cb_write(evutil_socket_t fd,short sig,void *arg)
{
	struct struWrite *pstu = (struct struWrite*)arg;
	char* cHttpReq = HttpHeadCreate(pstu->URL);
	if(cHttpReq!=NULL)
	{
		int ret;
		if(sig&EV_WRITE) 
		{
			ret=send(fd,cHttpReq,strlen(cHttpReq)+1,MSG_DONTWAIT);
		}
		else if(sig&EV_TIMEOUT)
		{
			ret=send(fd,cHttpReq,strlen(cHttpReq)+1,MSG_DONTWAIT);
		}
	}
	event_del(pstu->eventWrite);
	event_free(pstu->eventWrite);
	if(cHttpReq!=NULL)
	{
		free(cHttpReq);
		cHttpReq=NULL;
	}
	if(pstu!=NULL)
	{
		pstu->URL=NULL;
		free(pstu);
		pstu=NULL;
	}
}

void cb_read(evutil_socket_t fd,short sig,void *arg)
{
	struct struRead* psock = (struct struRead*)arg;
	if(sig&EV_READ)
	{	
		char* http_return=HttpDataTransmit(fd);
		if(http_return==NULL)
		{
			//ofileLog<<"------------------------------malloc goes wrong-------------------------------\n";
			return;
		}
		if(!(psock->bInit))
		{
			psock->pCurNode = pRoot;
			psock->bInit = true;
			psock->nURLlength = 0;
			//psock->mapCount=0;
			char* check = strstr(http_return,"HTTP/1.1 200 OK");
			if(check!=NULL)
			{
				nURLNum2++;
				ofile<<psock->URL<<' '<<nURLNum2<<endl;
				mapURL.insert(pair<char*,int>(psock->URL,nURLNum2));
			}
			else
			{
				//psock->mapPage.clear();
				mapPage[fd].clear();
				nPageCount[fd]=0;
				event_del(psock->eventRead);
				event_free(psock->eventRead);
				if(psock!=NULL)
				{
					if(psock->URL!=NULL)
					{
						free(psock->URL);
						psock->URL=NULL;
					}
					free(psock);
					psock=NULL;
				}
				if(http_return!=NULL)
				{
					free(http_return);
					http_return=NULL;
				}
				close(fd);
				nConnNum--;
				nBadWebNum++;
				return;
			}
		}
		int count = strlen(http_return);
		if(count==0)
		{
			nConnNum--;
			event_del(psock->eventRead);
			event_free(psock->eventRead);
			nWebNum++;
			close(fd);
			mapPage[fd].clear();
			nPageCount[fd]=0;
			if(psock!=NULL)
			{
				free(psock);
				psock=NULL;
			}
			if(http_return!=NULL)
			{
				free(http_return);
				http_return=NULL;
			}
			return;
		}
		
		int index = 0;
		int ret;
		char* URL = NULL;
		map<string,int>::iterator key;
		while(http_return[index])
		{
			if(http_return[index]=='\n')
			{
				index++;
				continue;
			}
			nextStep(&psock->pCurNode,http_return[index]);
			ret = checkIfMatch(&psock->pCurNode,&psock->nURLlength);
			if(ret>8&&ret<256)
		    {
				URL = (char*)malloc(256*sizeof(char));
				memset(URL, 0x0, sizeof(URL));
			    if(URL==NULL)
				{
					free(http_return);
					return;
				}
				snprintf(URL, ret, "%s", http_return + index - ret + 1);
				if(strlen(URL)!=0)
				{
					if(checkHost(URL))
					{
						string sURL = URL;
						key = mapPage[fd].find(sURL);
						if(key==mapPage[fd].end())
						{
							mapPage[fd].insert(pair<string,int>(sURL,++nPageCount[fd]));
							
							ofileLog<<psock->URL<<" - "<<URL<<endl;
							linknum++;
							if( checkIfNotExist(URL,strlen(URL)))
							{
								waitLine.push(URL);
								nURLNum++;
							}
							else
							{
								if(URL!=NULL)
								{
									free(URL);
									URL=NULL;
								}
							}
						}
						else
						{
							if(URL!=NULL)
							{
								free(URL);
								URL=NULL;
							}
						}
					}
					else
					{
						if(URL!=NULL)
						{
							free(URL);
							URL=NULL;
						}
					}
				}
				else
				{
					if(URL!=NULL)
					{
						free(URL);
						URL=NULL;
					}
				}
			}
			index++;
		}
		if(http_return!=NULL)
		{
			free(http_return);
			http_return=NULL;
		}
	}
	if(sig&EV_TIMEOUT)
	{
		nTimeOut++;
		mapPage[fd].clear();
		nPageCount[fd]=0;
		event_del(psock->eventRead);
		event_free(psock->eventRead);
		//ofileLog<<"-----------read timeout,fd:"<<psock->fd<<"---------------"<<endl;
		close(fd);
		nConnNum--;
		if(psock!=NULL)
		{
			if(psock->URL!=NULL)
			{
				free(psock->URL);
				psock->URL=NULL;
			}
			free(psock);
			psock=NULL;
		}
	}
}

inline int checkHost(char* URL)
{
		char* check = strstr(URL,"://news.sohu.com");
		if(check == NULL)
		{
			return 0;
		}
		else
		{
			return 1;
		}
}




