#include "thread.h"
#include <jsoncpp/json/json.h>
#include <sys/socket.h>
#include <iostream>

Thread::Thread(QObject *parent, int _socket, std::string _topic, std::string _flag): QThread(parent)
{
    this->socket = _socket;
    this->topic = _topic;
    this->flag = _flag;
    this->timestamp = 0;
    this->stopped = false;
}

void Thread::run()
{
    stopped = false;
    while (!stopped) {
        double temperature = (double) (-20 + (rand() % 70));
        Json::Value jdata;
        jdata["value"] = temperature;
        jdata["timestamp"] = timestamp;
        Json::Value obj;
        obj["command"] = "PUBLISH";
        obj["topic"] = this->topic;
        obj["data"] = jdata;
        obj["flag"] = this->flag;
        Json::StyledWriter styledWriter;
        std::string jsonString = styledWriter.write(obj);
        char json[1024];
        memset(json, 0, 1024);
        strcpy(json, jsonString.c_str());
        send(this->socket, json, sizeof(json), 0);
        timestamp++;
        emit SendMessage(QString::fromStdString(jsonString));
    }
    return;
}
