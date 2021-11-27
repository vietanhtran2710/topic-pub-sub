#include "thread.h"
#include <jsoncpp/json/json.h>
#include <sys/socket.h>
#include <iostream>
#include <QDateTime>

Thread::Thread(QObject *parent, int _socket, std::string _topic, std::string _flag): QThread(parent)
{
    this->socket = _socket;
    this->topic = _topic;
    this->flag = _flag;
    this->timestamp = 0;
    this->stopped = false;
    this->paused = false;
    this->topicRegistered = false;
    this->automaticData = true;
    this->customData["data"] = "0";
}

void Thread::run()
{
    if (!topicRegistered) {
        Json::Value obj;
        obj["command"] = "START PUBLISHING";
        obj["topic"] = this->topic;
        Json::StyledWriter styledWriter;
        std::string jsonString = styledWriter.write(obj);
        char json[1024];
        memset(json, 0, 1024);
        strcpy(json, jsonString.c_str());
        send(this->socket, json, sizeof(json), 0);
        this->topicRegistered = true;
    }
    while (!stopped && !paused) {
        Json::Value jdata, obj;
        jdata["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        if (this->automaticData) {
            jdata["value"] = rand() % 1000;
        }
        else {
            for(std::map<QString, QString>::iterator iter = customData.begin(); iter != customData.end(); ++iter) {
                QString key =  iter->first;
                QString value = iter->second;
                jdata[key.toStdString()] = value.toStdString();
            }
        }
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
        QThread::msleep(100);
        timestamp++;
        emit SendMessage(QString::fromStdString(jsonString));
    }
    return;
}
