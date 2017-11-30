#ifndef H_CRAWLER
#define H_CRAWLER

#include "http.h"
#include "ACstates.h"
#include "Hash.h"
#include <string>
#include <string.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
#include <queue>
#include <map>
#include <pthread.h>
#include <unistd.h>
static Node* pRoot;
void cb_read(evutil_socket_t fd,short sig,void *arg);
void cb_write(evutil_socket_t fd,short sig,void *arg);
inline int checkHost(char* URL);
void rebuildUrlLink();
struct cmp
{
	bool operator()(const char* s1,const char* s2)
	{
		return strcmp(s1,s2)<0;
	}
};

struct struRead
{
	evutil_socket_t fd=0;
	Node* pCurNode=NULL;
	bool bInit=false;
	int nURLlength;
	struct event *eventRead;
	char* URL;
};

struct struWrite
{
	char* URL = NULL;
	struct event *eventWrite;
};

#endif