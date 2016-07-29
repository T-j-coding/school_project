#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>

#define RULEPORT 9999
void main()
{
printf("111\n");
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

printf("111\n");
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
printf("111\n");
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
            //rule_process(buf);
        }
    }
}
