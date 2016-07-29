#include "log.h"

void printlog(struct loginfo* log)
{
    printf("################## This is Loginfo ####################\n");
    printf("# configid:%ld, logtime:%s\n",log->configid,log->logtime);
    printf("# processorid:%d, website:%s\n",log->processorid,log->website);
    printf("# sport:%u, sip:%u\n",log->serverport,log->serverip);
    printf("# cport:%u, cip:%u\n",log->clientport,log->clientip);
    printf("#\n# actiontype:%d, userid:%s, resourceid:%s\n",log->actiontype,log->userID,log->resourceID);
    printf("# originUID:%s, originRID:%s\n",log->originUID,log->originRID);
    printf("# URL:%s\n",log->url);
    printf("# keywords:%s\n",log->keywords);
    printf("# fulltext: %s\n", log->fulltext);
    printf("##################  Loginfo  end   ####################\n");
    return;
}

int sendlog(struct loginfo * log)
{
    //printlog(log);
    char buf[1200];
    bzero(buf,1200*sizeof(char));
    //make transmission string

//    sprintf(buf, "%d|",log->configid);
    sprintf(buf+strlen(buf), "%s|", log->logtime);
    sprintf(buf+strlen(buf), "%d|", log->processorid);
    sprintf(buf+strlen(buf), "%s|", log->website);
    sprintf(buf+strlen(buf), "%u|", log->serverip);
    sprintf(buf+strlen(buf), "%u|", log->serverport);
    sprintf(buf+strlen(buf), "%u|", log->clientip);
    sprintf(buf+strlen(buf), "%u|", log->clientport);
    sprintf(buf+strlen(buf), "%d|", log->actiontype);
    sprintf(buf+strlen(buf), "%s|", log->userID);
    sprintf(buf+strlen(buf), "%s|", log->resourceID);
    sprintf(buf+strlen(buf), "%s|", log->originUID);
    sprintf(buf+strlen(buf), "%s|", log->originRID);
    sprintf(buf+strlen(buf), "%s|", log->keywords);
    sprintf(buf+strlen(buf), "%d|", log->gk_type);
    sprintf(buf+strlen(buf), "%s|end|", log->fulltext);
    printf("log string:\n%s\n",buf);
    buf[strlen(buf)] = '\n';//for telnet, last char '\n' means end of transmission
    
    printf("log buf:%s\n", buf);

    //send log string
    int sockfd,n;
    struct sockaddr_in servaddr,cliaddr;
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    if (gk_method == 5)
    {
	servaddr.sin_port=htons(LOGPORT2);
    }
    else
    {
	servaddr.sin_port=htons(LOGPORT);
    }   
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))<0)
    {
	perror("connect fail\n");
	printf("\n!!!log server connect fail!!!\n\n");
	return 0;
    }
    int sendbytes;
    sendbytes = send(sockfd, buf, strlen(buf), 0);
    //sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    printf(" bufbytes=%d\nsendbytes=%d\n",strlen(buf),sendbytes);
    printf("log send done!\n");
    char recvbuf[100];
    recv(sockfd, recvbuf, 100, 0);
    printf("receive:%s|\n",recvbuf);
    return 1;
}

int getlogtime(struct loginfo * log)
{
    //2001-05-10 12:13:14.567
    time_t rawtime; 
    struct tm * timeinfo; 
    time ( &rawtime ); 
    timeinfo = localtime ( &rawtime ); 
    timeinfo->tm_mon++;

    struct timeval tv;
    gettimeofday(&tv,NULL);

    char* mytime = (char*)malloc(25*sizeof(char));
    bzero(mytime,25);
    sprintf(mytime,"%d",timeinfo->tm_year+1900);
    if(timeinfo->tm_mon < 10)
    {	sprintf(mytime+6,"%d",timeinfo->tm_mon);
	*(mytime+5)='0';    }else
    {	sprintf(mytime+5,"%d",timeinfo->tm_mon);    }
    if(timeinfo->tm_mday < 10)
    {	sprintf(mytime+9,"%d",timeinfo->tm_mday);
	*(mytime+8)='0';    }else
    {	sprintf(mytime+8,"%d",timeinfo->tm_mday);   }
    if(timeinfo->tm_hour < 10)
    {	sprintf(mytime+12,"%d",timeinfo->tm_hour);
	*(mytime+11)='0';   }else
    {	sprintf(mytime+11,"%d",timeinfo->tm_hour);  }
    if(timeinfo->tm_min < 10)
    {	sprintf(mytime+15,"%d",timeinfo->tm_min);
	*(mytime+14)='0';   }else
    {	sprintf(mytime+14,"%d",timeinfo->tm_min);   }
    if(timeinfo->tm_sec < 10)
    {	sprintf(mytime+18,"%d",timeinfo->tm_sec);
	*(mytime+17)='0';   }else
    {	sprintf(mytime+17,"%d",timeinfo->tm_sec);   }

    *(mytime+4) = *(mytime+7) = '-';
    *(mytime+10) = ' ';
    *(mytime+13) = *(mytime+16) = ':';

    //for ms   
    *(mytime+19) = '.';
    sprintf(mytime+20,"%d",tv.tv_usec/100000);
    sprintf(mytime+21,"%d",(tv.tv_usec%100000)/10000);
    sprintf(mytime+22,"%d",(tv.tv_usec%10000)/1000);

//    printf("time:|%s|\n",mytime);
    memcpy(log->logtime,mytime,strlen(mytime));
    free(mytime);
    return 1;
}

void initlog(struct loginfo * log, struct tcp_stream * a_tcp)
{
    bzero(log, sizeof(struct loginfo));
    getlogtime(log);
    log->processorid = PROCESSORID;//different in each worknode
    memcpy(log->website,WEBSITE,strlen(WEBSITE));

    //tuple4
//    log->serverport = a_tcp->addr.dest;
//    log->serverip  =  a_tcp->addr.daddr;
//    log->clientport = a_tcp->addr.source;
//    log->clientip  =  a_tcp->addr.saddr;
    log->serverport = a_tcp->addr.dest;
    log->serverip  =  a_tcp->addr.daddr;
    log->clientport = a_tcp->addr.source;
    log->clientip  =  a_tcp->addr.saddr;
    return;
}

void log_addsocialinfo(struct loginfo* log, struct connection_info* c_info)
{
    //social info
    log->actiontype = c_info->p_type;
//url?????
    memcpy(log->userID     , c_info->user_id , strlen(c_info->user_id));
    memcpy(log->resourceID , c_info->r_id    , strlen(c_info->r_id));
    memcpy(log->originUID  , c_info->s_id    , strlen(c_info->s_id));
//    memcpy(log->fulltext  , c_info->comment , strlen(c_info->comment));
    return;
}


void log_set_full_text(struct loginfo* log, char* fulltext)
{
    memcpy(log->fulltext, fulltext, strlen(fulltext));
}


void log_addkeywords(struct loginfo* log, char* kw[], int count)
{
    int n;
    for(n=0; n<count; n++)
    {
	strcat(log->keywords, kw[n]);
	strcat(log->keywords, "&");
    }
    return;
}


void log_set_gk_type(struct loginfo* log, int gk_type)
{
   log->gk_type = gk_type; 
}
