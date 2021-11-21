#include <QCoreApplication>
#include <iostream>
#include <iterator>
#include <map>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "thread.h"

#define SERVER_PORT 8080;

using namespace std;

class Broker: QObject {
public:
    Broker() {
        this->threads = new vector<Thread*>(0);
    }

    ~Broker() {

    }

    void createThread(int socket) {
        this->threads->push_back(new Thread(this, socket));
        this->threads->back()->run();
    }

public slots:
    void onMessageReceive(QString topicName, Thread &thread) {

    }

    void onCancelSubscription(QString topicName) {

    }

    void onNewSubscriber(QString topicName, int socket) {

    }

private:
    map<QString, QString> retainedMessage;
    map<QString, vector<int>> topicSubscriber;
    vector<Thread*> *threads;

private slots:


};

int main(int argc, char *argv[])
{
    Broker *broker = new Broker();
    int listenPort = SERVER_PORT;
    int servSock, cliSock;
    struct sockaddr_in servSockAddr, cliAddr;
    int servSockLen, cliAddrLen;

    servSock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servSockAddr, sizeof(servSockAddr));
    servSockAddr.sin_family = AF_INET;
    servSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servSockAddr.sin_port = htons(listenPort);

    servSockLen = sizeof(servSockAddr);

    if (bind(servSock, (struct sockaddr*) &servSockAddr, servSockLen) < 0)
    {
        perror("bind");
        exit(1);
    }
    if (listen(servSock, 10) < 0)
    {
        perror("listen");
        exit(1);
    }
    while (true) {
        std::cout << "Waiting for a client ..." << std::endl;
        cliSock = accept(servSock, (struct sockaddr *) &cliAddr, (socklen_t *) &cliAddrLen);
        std::cout << "Received a connection from a client %s\n", inet_ntoa(cliAddr.sin_addr) << std::endl;
        broker->createThread(cliSock);
    }
    return 0;
}
