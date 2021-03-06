#include "thread.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <jsoncpp/json/json.h>

#define SERVER_PORT 8080

Thread::Thread(QObject *parent, int _type, int _socket): QThread(parent)
{
    this->type = _type;
    this->stopped = false;
    this->connSocket = _socket;
}

void Thread::run() {
    if (this->type == 0) {
        int listenPort = SERVER_PORT;
        int cliSock;
        struct sockaddr_in servSockAddr, cliAddr;
        int servSockLen, cliAddrLen;

        this->servSock = socket(AF_INET, SOCK_STREAM, 0);
        bzero(&servSockAddr, sizeof(servSockAddr));
        servSockAddr.sin_family = AF_INET;
        servSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servSockAddr.sin_port = htons(listenPort);

        servSockLen = sizeof(servSockAddr);

        if (bind(this->servSock, (struct sockaddr*) &servSockAddr, servSockLen) < 0)
        {
            perror("bind");
            exit(1);
        }
        if (listen(this->servSock, 10) < 0)
        {
            perror("listen");
            exit(1);
        }
        while (!this->stopped) {
            std::cout << "Waiting for a client ..." << std::endl;
            cliSock = accept(this->servSock, (struct sockaddr *) &cliAddr, (socklen_t *) &cliAddrLen);
            std::cout << "Received a connection from a client " << inet_ntoa(cliAddr.sin_addr) << std::endl;
            emit this->NewClient(cliSock);
        }
        close(this->servSock);
        return;
    }
    else {
        char readBuffer[1024], sendBuffer[1024];
        while(!this->stopped) {
            memset(readBuffer, 0, 1024);
            recv(this->connSocket, readBuffer, sizeof(readBuffer), 0);
            if (strncmp(readBuffer, "{\n", 2)==0)
            {
                Json::Value data; Json::Reader read;
                std::string json(readBuffer);
                read.parse(json, data);
                if (data["command"] == "START PUBLISHING") {
                    emit NewTopic(QString::fromStdString(data["topic"].asString()));
                }
                else if (data["command"] == "STOP PUBLISHING") {
                    emit QuitTopic(QString::fromStdString(data["topic"].asString()));
                }
                else if (data["command"] == "PUBLISH") {
                    Json::StyledWriter styledWriter;
                    std::string jsonString = styledWriter.write(data["data"]);
                    QString message = QString::fromStdString(jsonString);
                    QString topic = QString::fromStdString(data["topic"].asString());
                    QString retain = QString::fromStdString((data["flag"].asString()));
                    emit this->NewMessage(topic, message, retain);
                }
                else if (data["command"] == "QUIT") {
                    emit NodeQuit();
                    break;
                }
                else if (data["command"] == "CONNECT") {
                    Json::Value obj;
                    obj["status"] = "CONNECTACK";
                    Json::StyledWriter styledWriter; std::string jsonString = styledWriter.write(obj);
                    strcpy(sendBuffer, jsonString.c_str());
                    send(this->connSocket, sendBuffer, strlen(sendBuffer), 0);
                }
                else if (data["command"] == "SUBSCRIBE") {
                    emit this->NewSubscriber(QString::fromStdString(data["topic"].asString()), this->connSocket);
                }
                else if (data["command"] == "STOP SUBSCRIBING") {
                    QString topic = QString::fromStdString(data["topic"].asString());
                    emit this->SubscriberQuit(this->connSocket, topic);
                }
                else if (data["command"] == "GET ALL TOPICS") {
                    emit this->GetTopic(this->connSocket);
                }
            }
        }
        close(this->connSocket);
        return;
    }
}
