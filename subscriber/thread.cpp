#include "thread.h"
#include <jsoncpp/json/json.h>
#include <sys/socket.h>

Thread::Thread(QObject *parent, int _socket, std::string _topic): QThread(parent)
{
    this->socket = _socket;
    this->topic = _topic;
}

void Thread::run() {
    this->stopped = false;
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
        emit NewMessage(QString::fromUtf8(buffer));
    }
    return;
}
