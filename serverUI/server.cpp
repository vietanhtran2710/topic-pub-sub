#include "server.h"
#include "./ui_server.h"
#include <iostream>
#include <iterator>
#include <map>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "thread.h"
#include <QCloseEvent>

server::server(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::server)
{
    ui->setupUi(this);
    server::threads = new std::vector<Thread*>(0);
    server::acceptThread = new Thread(this, 0, 0);
    connect(acceptThread, SIGNAL(NewClient(int)), this, SLOT(onNewClient(int)));
    server::acceptThread->start();
}

server::~server()
{
    delete ui;
}

void server::closeEvent(QCloseEvent *event) {
    server::acceptThread->stopped = true;
    event->accept();
}

void server::onNewClient(int socket) {
    server::threads->push_back(new Thread(this, 1, socket));
    connect(this->threads->at(threads->size() - 1), SIGNAL(NewMessage(QString, QString, QString)), this, SLOT(onNewMessage(QString, QString, QString)));
    connect(this->threads->at(threads->size() - 1), SIGNAL(NewSubscriber(QString, int)), this, SLOT(onNewSubscriber(QString, int)));
    this->threads->back()->start();
}

void server::onNewMessage(QString topicName, QString message, QString retainFlag) {
    std::cout << "message.toStdString()" << std::endl;
    std::cout << message.toStdString() << std::endl;
    if (topicSubscriber[topicName]->size() == 0) {
        if (retainFlag == "retain") {
            retainedMessage[topicName] = message;
        }
    }
    else {
        std::cout << topicSubscriber[topicName]->size() << std::endl;
        for (int i = 0; i < topicSubscriber[topicName]->size(); i++) {
            int socket = topicSubscriber[topicName]->at(i);
            char sendBuffer[1024] = {0};
            strcpy(sendBuffer, message.toLocal8Bit().data());
            send(socket, sendBuffer, strlen(sendBuffer), 0);
        }
    }
}

void server::onNewSubscriber(QString topicName, int socket) {
    std::cout << "Got new subscriber" << std::endl;
    if (topicSubscriber.find(topicName) == topicSubscriber.end() ) {
        topicSubscriber[topicName] = new std::vector<int>(1, socket);
    } else {
        topicSubscriber[topicName]->push_back(socket);
    }
}



