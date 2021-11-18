#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <jsoncpp/json/json.h>
#define MAX 1024
#define PORT 8080
#define SA struct sockaddr
using namespace std;
void protocol(int sockfd)
{
    char buff[1024] = {0};
    int fSize = 0;
    int n = 0;
    int timestamp = 0; 
    for (;;) {
		char command[20];
        char topic[30];
        char flag[10];
        char data[100];
        double temperature = (double) (-20 + (rand() % 70));
        char json[1024];
        cout << "Enter command: ";
        cin.getline(command, 20);
        cout << "Enter topic: ";
        cin.getline(topic, 30);
        cout << "Enter flag: ";
        cin.getline(flag, 10);
        //cout << "Enter temperature: "; cin >> temperature;
        //cout << "Enter timestamp: "; cin >> timestamp;

        //sprintf(data, "\"data\":\n{\n\"value\":\"%f\",\n\"timestamp\":\"%d\"\n},\n", temperature, timestamp);
        //sprintf(json, "{\n\"command\":\"%s\",\n\"topic\":\"%s\",\n%s\"flag\":\"%s\"\n}", command, topic, data, flag);
        Json::Value jdata;
        jdata["value"] = temperature;
        jdata["timestamp"] = timestamp;
        Json::Value obj;
        obj["command"] = command;
        obj["topic"] = topic;
        obj["data"] = jdata;
        obj["flag"] = flag;
        Json::StyledWriter sw;
        string j = sw.write(obj);
        strcpy(json, j.c_str());
		send(sockfd, json, sizeof(json), 0);
        printf("JSON:\n%s\n", json);
        recv(sockfd, buff, sizeof(buff), 0);
		printf("SERVER: %s\n", buff);
        memset(buff, 1024, 0);
        memset(command, 20,0);
        memset(topic, 30,0);
        memset(flag, 10,0);
        memset(data, 100, 0);	
        timestamp++;
    }
}
   
int main()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    char addr[20];
   
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    printf("Enter server IP address: ");
    std::cin.getline(addr, sizeof(addr));
	
	if (inet_pton(AF_INET, addr, &servaddr.sin_addr)<=0)
	{
		printf("Invalid Address!");
		exit(0);
	}
   
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("Connection to server failed...\n");
        exit(0);
    }
    else
        printf("Connected to server..\n");
   
    // server-client chat function
    protocol(sockfd);
    return 0;
}