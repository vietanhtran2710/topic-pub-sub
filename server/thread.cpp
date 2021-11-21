#include "thread.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <jsoncpp/json/json.h>
#include <iostream>

Thread::Thread(QObject *parent, int _socket)
{
    this->socket = _socket;
}

void Thread::run() {
    char buffer[1024];
    while(true) {
        memset(buffer, 0, 1024);
        recv(this->socket, buffer, sizeof(buffer), 0);
        if (strncmp(buffer, "{\n", 2)==0)
        {
            Json::Value data; Json::Reader read;
            std::string json(buffer);
            read.parse(json, data);
            std::cout << data["command"] << std::endl;
            if (data["command"] == "QUIT") break;
        }
        else std::cout << "Not JSON\n";
    }
    return;
}
