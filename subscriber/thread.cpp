#include "thread.h"
#include <jsoncpp/json/json.h>
#include <sys/socket.h>
#include <iostream>

Thread::Thread(QObject *parent, int _socket, std::string _topic): QThread(parent)
{
    this->socket = _socket;
    this->topic = _topic;
    this->stopped = false;
}

void Thread::run() {
    Json::Value obj; Json::StyledWriter styledWriter;
    obj["command"] = "SUBSCRIBE"; obj["topic"] = this->topic;
    std::string jsonString = styledWriter.write(obj);
    char sendBuffer[1024] = {0};
    strcpy(sendBuffer, jsonString.c_str());
    send(this->socket, sendBuffer, sizeof(sendBuffer), 0);
    char buffer[1024] = {0};
    while(!stopped) {
        memset(buffer, 1024, 0);
        recv(this->socket, buffer, sizeof(buffer), 0);
        Json::Value data; Json::Reader read;
        std::string json(buffer);
        read.parse(json, data);
        Json::Value obj;
        QString topic = QString::fromStdString(data["topic"].toStyledString());
        QString message = QString::fromStdString(data["message"].toStyledString());
        emit NewMessage(topic, message);
    }
    return;
}
