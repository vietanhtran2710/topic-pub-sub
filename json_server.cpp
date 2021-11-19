#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <jsoncpp/json/json.h>
using namespace std;
#define MAX 1024
#define PORT 8080
#define SA struct sockaddr
   
static void *protocol(void* arg)
{
	int sockfd = *((int*) arg);
	pthread_detach(pthread_self());
    char buff[MAX] = {0};
    
    for (;;) {
        recv(sockfd, buff, sizeof(buff),0);
		// printf("CLIENT:\n%s\n", buff);
        
        if (strncmp(buff, "{\n", 2)==0)
        {
            Json::Value data;
            Json::Reader read;
            string json(buff);
            read.parse(json, data);
            cout << data["data"] << endl;
        }
        else 
        {
            // send(sockfd, invalid, sizeof(invalid), 0);
            cout << "Not JSON\n";
        }
        memset(buff, 0, MAX);
    }
    close(sockfd);
    return NULL;
}
   

int main()
{
	pthread_t tid;
    int sockfd, connfd;
    int len;
    int* iptr;
    struct sockaddr_in servaddr, cli;
    struct sockaddr_in cliaddr;
    int addrlen = sizeof(servaddr);
   
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("Socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");
   
    
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening...\n");
    len=sizeof(cli);
    
   while(0==0)
   {
	   connfd = accept(sockfd, (SA*)&cli, (socklen_t*)&len);
	   printf("Server accepted a client...\n");
	   pthread_create(&tid, NULL, &protocol, (void*)&connfd);
   }
}