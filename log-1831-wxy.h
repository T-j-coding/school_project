#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "nids.h"
#include "include.h"
#include "get_data.h"
#include "loop.h"

#define PROCESSORID 43
#define WEBSITE "renren"
#define LOGPORT 8888
#define LOGPORT2 8889

struct loginfo{
    long configid;
    char logtime[25]; //format: "yyyy-mm-dd hh:mm:ss"
    int processorid; //id of worknode
    char website[7]; //"renren" or "weibo"
    //tuple4
    unsigned short serverport;
    unsigned int serverip;
    unsigned short clientport;
    unsigned int clientip;
/*    char serverport[6];
    char serverip[16];
    char clientport[6];
    char clientip[16];
*/
    //social info
    int actiontype;
    char userID[20];
    char resourceID[20];
    char originUID[20];
    char originRID[20];
    char url[512];
    char keywords[300];//format: "kw1&kw2&kw3"
    int gk_type;
    char fulltext[1000];
};
void printlog(struct loginfo*);
int sendlog(struct loginfo*);
void initlog(struct loginfo *, struct tcp_stream *);
void log_addkeyword(struct loginfo* log, char* kw);
void log_addsocialinfo(struct loginfo*, struct connection_info*);
void log_addkeywords(struct loginfo*, char*[], int);

#endif
