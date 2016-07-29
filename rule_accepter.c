#include "rule_accepter.h"
#include "down_string_match.h"
extern ACSMX_STRUCT* N;
extern ACSMX_STRUCT* M;
extern struct unicode_to_string1 utos[100000]; 
extern  int utos_length ;
void* startlisten(void *arg)
{
    int server_sockfd,client_sockfd;
    int len;
    struct sockaddr_in my_addr;
    struct sockaddr_in remote_addr;
    int sin_size;
    char buf[1024];
    memset(&my_addr,0,sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(RULEPORT);
    
    if((server_sockfd = socket(PF_INET,SOCK_STREAM,0))<0)
    {
	perror("socket");
	return;
    }
    if(bind(server_sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr))<0)
    {
	perror("bind");
	return;
    }
    listen(server_sockfd,5);
printf("Rule Accepter, start listen\n");
    while(1){
	sin_size = sizeof(struct sockaddr_in);
	if((client_sockfd = accept(server_sockfd, (struct sockaddr*)&remote_addr,&sin_size))<0)
	{
	    perror("accept");
	    continue;
	}
	printf("accept client %s\n", inet_ntoa(remote_addr.sin_addr));
	len = send(client_sockfd, "connection established\n", 23, 0);

	if((len = recv(client_sockfd, buf, 1024, 0))>0){
	    buf[len]='\0';
	    rule_process(buf);
	}
    }
}
void rule_process(char* rule)
{   
    printf("Receive Rule:%s\n",rule);
    char * configid;
    char * ctime;//config time
    char * stime;//start time
    char * etime;//end time
    char * state;//1 for enable ,or 0 for disable
    char * webtype;
    char * end;
    char * workmode;//1 for monitor, 2 for gk
    char * up_or_down;// 1 for up traffic, 2 for down gizp traffic
    if('1' == *rule)
    {
	// example "1|111|keywords|2015-6-8|stime|etime|1|00|2|"
	char * keyword;
	printf("rule type: 1\n");
	configid = strchr(rule, '|') + 1;
	keyword = strchr(configid, '|') + 1;
	ctime = strchr(keyword, '|') + 1;
	stime = strchr(ctime, '|') + 1;
	etime = strchr(stime, '|') + 1;
	state = strchr(etime, '|') + 1;
	webtype = strchr(state, '|') + 1;
	workmode = strchr(webtype,'|') + 1;
        up_or_down = strchr(workmode,'|')+1;
	end = strchr(up_or_down, '|');
	//printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",configid,keyword,ctime,stime,etime,state,webtype,up_or_down);
	char rule1[256];
	bzero(rule1, 256*sizeof(char));
	memcpy(rule1, keyword, ctime-keyword-1);
	printf("rule1:|%s|\n",rule1);
printf("mode:%c\n",*workmode);

	if('2' == *workmode)
	{
	    printf("rule mode: gk\n");
	    if('1'==*state)
	    {
		if (*up_or_down == '1')
			add_query_rule(rule1);
		if (*up_or_down == '2')
		{
	        	add_str(rule1);
			char key[1024];
			if(!strtounicode(rule1,key)){
				add_str(key);
				memcpy(utos[utos_length].str,rule1,strlen(rule1));
				memcpy(utos[utos_length].code,key,strlen(key));
				utos_length++;
				printf("utos_length == %d\n",utos_length);	
			}
		}
	    }
	    else if('0'==*state)
	    {
		del_query_rule(rule1);
                del_str(rule1);
	    }
	}
	else if('1'== *workmode)
	{
	    printf("rule mode: monitor\n");
	    if('1'==*state)
	    {
		printf("add_query_rule_N from rule_accepter.c\n");
		add_query_rule_N(rule1);
		if (*up_or_down == '2')
		{
	        	add_str(rule1);
			char key[1024];
			if(!strtounicode(rule1,key))
				add_str(key);
		}
	    }
	    else if('0'==*state)
	    {
		del_query_rule_N(rule1);

	    }
	    
	}
	
    }
    else if('2' == *rule)
    {
	// example "2|222|userid|5|2015-6-8|stime|etime|1|00|"
	char * userid;
	char * action;
	printf("rule type: 2\n");

	configid = strchr(rule, '|') + 1;
	userid = strchr(configid, '|') + 1;
	action = strchr(userid, '|') + 1;
	ctime = strchr(action, '|') + 1;
	stime = strchr(ctime, '|') + 1;
	etime = strchr(stime, '|') + 1;
	state = strchr(etime, '|') + 1;
	webtype = strchr(state, '|') + 1;
	end = strchr(webtype, '|');
	//printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",configid,userid,action,ctime,stime,etime,state,webtype);
	char rule2[256];
	bzero(rule2, 256*sizeof(char));
	memcpy(rule2, userid, action-userid-1);
	rule2[strlen(rule2)] = ' ';
	memcpy(rule2+strlen(rule2), action, ctime-action-1);
	rule2[strlen(rule2)] = ' ';
	rule2[strlen(rule2)] = '*';
	printf("rule2:|%s|\n",rule2);
	if('1'==*state)
     
   {
            insert_for_userid(rule2);
        }
        else if('0'==*state)
        {
            del(rule2);
        }
    }
    else if('3' == *rule)
    {
	// example "3|222|resourceid|5|2015-6-8|stime|etime|1|00|"
	char * resourceid;
	char * action;
	printf("type3\n");
	configid = strchr(rule, '|') + 1;
	resourceid = strchr(configid, '|') + 1;
	action = strchr(resourceid, '|') + 1;
	ctime = strchr(action, '|') + 1;
	stime = strchr(ctime, '|') + 1;
	etime = strchr(stime, '|') + 1;
	state = strchr(etime, '|') + 1;
	webtype = strchr(state, '|') + 1;
	end = strchr(webtype, '|');
 
        // sunpy: SHARE:13
        /*if (*action == '1' && *(action + 1) == '3') {
            char url[512];
            memset(url, 0, 512 * sizeof(char));
            memcpy(url, resourceid, action - resourceid - 1);
            printf("##### accept URL in share: %s\n", url);     
            addurl(url);
        } else {i*/
	    char rule3[256];
	    bzero(rule3, 256*sizeof(char));
	    rule3[0] = '*';
	    rule3[1] = ' ';
	    memcpy(rule3+strlen(rule3), action, ctime-action-1);
	    rule3[strlen(rule3)] = ' ';
	    memcpy(rule3+strlen(rule3), resourceid, action-resourceid-1);
	    printf("rule3:|%s|\n",rule3);
	    if('1'==*state)
	    {
	        insert_for_resourceid(rule3);
	    }
	    else if('0'==*state)
	    {
	        del(rule3);
	    }
        //}
    }
    else if('4' == *rule)
    {//url , todo

	//sunpy added
        char * keyword;
        printf("rule type: 4\n");
        configid = strchr(rule, '|') + 1;
        keyword = strchr(configid, '|') + 1;
        ctime = strchr(keyword, '|') + 1;
        stime = strchr(ctime, '|') + 1;
        etime = strchr(stime, '|') + 1;
        state = strchr(etime, '|') + 1;
        webtype = strchr(state, '|') + 1;
        end = strchr(webtype, '|');

        char rule1[256];
        bzero(rule1, 256*sizeof(char));
        memcpy(rule1, keyword, ctime-keyword-1);
        printf("rule1:|%s|\n",rule1);
        if('1'==*state)
        {
            addurl(rule1);
        }
        else if('0'==*state)
        {
            delurl(rule1);
        }     

    }
    return;
}
