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
#include <QStandardItemModel>
#include <QStandardItem>
#include <algorithm>

server::server(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::server)
{
    ui->setupUi(this);
    model = new QStandardItemModel(0, 3, this);
    QStringList labels; labels << "Topic" << "Number of Publishers" << "Number of Subscriber";
    model->setHorizontalHeaderLabels(labels);
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    server::threads = new std::vector<Thread*>(0);
    server::clientCount = 0;
    server::acceptThread = new Thread(this, 0, 0);
    connect(acceptThread, SIGNAL(NewClient(int)), this, SLOT(onNewClient(int)));
    server::acceptThread->start();
}
;
server::~server()
{
    delete ui;
}

void server::closeEvent(QCloseEvent *event) {
    server::acceptThread->stopped = true;
    server::acceptThread->exit();
    for (int i = 0; i < server::threads->size(); i++)
        server::threads->at(i)->exit();
    event->accept();
}

void server::onNewClient(int socket) {
    server::threads->push_back(new Thread(this, 1, socket));
    connect(this->threads->at(threads->size() - 1), SIGNAL(NewMessage(QString, QString, QString)), this, SLOT(onNewMessage(QString, QString, QString)));
    connect(this->threads->at(threads->size() - 1), SIGNAL(NewSubscriber(QString, int)), this, SLOT(onNewSubscriber(QString, int)));
    connect(this->threads->at(threads->size() - 1), SIGNAL(NewTopic(QString)), this, SLOT(onNewTopic(QString)));
    connect(this->threads->at(threads->size() - 1), SIGNAL(QuitTopic(QString)), this, SLOT(onQuitTopic(QString)));
    connect(this->threads->at(threads->size() - 1), SIGNAL(NodeQuit()), this, SLOT(onNodeQuit()));
    connect(this->threads->at(threads->size() - 1), SIGNAL(SubscriberQuit(int, QString)), this, SLOT(onSubscriberQuit(int, QString)));
    connect(this->threads->at(threads->size() - 1), SIGNAL(GetTopic(int)), this, SLOT(onGetTopic(int)));
    this->threads->back()->start();
    server::clientCount++;
    ui->label_2->setText(QString::number(server::clientCount));
}

void server::onNewMessage(QString topicName, QString message, QString retainFlag) {
    if (topicSubscriber.find(topicName) != topicSubscriber.end()) {
        if (topicSubscriber[topicName]->size() == 0) {
            if (retainFlag == "retain") {
                retainedMessage[topicName] = message;
            }
        }
        else {
            for (int i = 0; i < topicSubscriber[topicName]->size(); i++) {
                int socket = topicSubscriber[topicName]->at(i);
                char sendBuffer[1024] = {0};
                strcpy(sendBuffer, message.toLocal8Bit().data());
                send(socket, sendBuffer, strlen(sendBuffer), 0);
            }
        }
    }
}

void server::onNewSubscriber(QString topicName, int socket) {
    if (topicSubscriber.find(topicName) == topicSubscriber.end() ) {
        topicSubscriber[topicName] = new std::vector<int>(1, socket);
        if (topicPublishers.find(topicName) == topicPublishers.end()) {
            QList<QStandardItem *> items;
            items.append(new QStandardItem(topicName));
            items.append(new QStandardItem("0"));
            items.append(new QStandardItem("1"));
            model->appendRow(items);
        }
        else {
            QStandardItem *findResult = model->findItems(topicName)[0];
            model->setItem(findResult->row(), findResult->column() + 2, new QStandardItem("1"));
        }
    } else {
        topicSubscriber[topicName]->push_back(socket);
        QStandardItem *findResult = model->findItems(topicName)[0];
        model->setItem(findResult->row(), findResult->column() + 2, new QStandardItem(QString::number(topicSubscriber[topicName]->size())));
    }
}

void server::onNewTopic(QString topicName) {
    if (topicPublishers.find(topicName) == topicPublishers.end()) {
        if (topicSubscriber.find(topicName) == topicSubscriber.end()) {
            topicPublishers[topicName] = 1;
            QList<QStandardItem *> items;
            items.append(new QStandardItem(topicName));
            items.append(new QStandardItem("1"));
            items.append(new QStandardItem("0"));
            model->appendRow(items);
        }
        else {
            topicPublishers[topicName] = 1;
            QStandardItem *findResult = model->findItems(topicName)[0];
            model->setItem(findResult->row(), findResult->column() + 1, new QStandardItem("1"));
        }
    }
    else {
        topicPublishers[topicName]++;
        QStandardItem *findResult = model->findItems(topicName)[0];
        model->setItem(findResult->row(), findResult->column() + 1, new QStandardItem(QString::number(topicPublishers[topicName])));
    }
}

void server::onQuitTopic(QString topicName) {
    std::cout << "Quit topic" << std::endl;
    std::cout << topicPublishers[topicName] << std::endl;
    topicPublishers[topicName]--;
    std::cout << topicPublishers[topicName] << std::endl;
    QStandardItem *findResult = model->findItems(topicName)[0];
    if (topicPublishers[topicName] == 0) {
        if (topicSubscriber.find(topicName) == topicSubscriber.end())
            model->removeRow(findResult->row());
        else
            model->setItem(findResult->row(), findResult->column() + 1, new QStandardItem(QString::number(topicPublishers[topicName])));
        topicPublishers.erase(topicName);
    }
    else {
        model->setItem(findResult->row(), findResult->column() + 1, new QStandardItem(QString::number(topicPublishers[topicName])));
    }
}

void server::onNodeQuit() {
    server::clientCount--;
    ui->label_2->setText(QString::number(server::clientCount));
}

void server::onSubscriberQuit(int socket, QString topic) {
    if (topicSubscriber.find(topic) == topicSubscriber.end()) return;
    std::vector<int>::iterator it = std::find(topicSubscriber[topic]->begin(), topicSubscriber[topic]->end(), socket);
    if (it == topicSubscriber[topic]->end()) {
        return;
    }
    topicSubscriber[topic]->erase(std::remove(topicSubscriber[topic]->begin(), topicSubscriber[topic]->end(), socket), topicSubscriber[topic]->end());
    QStandardItem *findResult = model->findItems(topic)[0];
    if (topicSubscriber[topic]->size() == 0) {
        topicSubscriber.erase(topic);
        if (topicPublishers.find(topic) == topicPublishers.end())
            model->removeRow(findResult->row());
        else
            model->setItem(findResult->row(), findResult->column() + 2, new QStandardItem(QString::number(0)));
    }
    else
        model->setItem(findResult->row(), findResult->column() + 2, new QStandardItem(QString::number(topicSubscriber[topic]->size())));
}

void server::onGetTopic(int _socket) {
    std::string topics = "";
    if (topicPublishers.size() == 0) {
        topics = "NO TOPIC AVAILABLE";
    }
    else {
        for(std::map<QString, int>::iterator iter = topicPublishers.begin(); iter != topicPublishers.end(); ++iter) {
            QString key =  iter->first;
            topics += key.toStdString() + ";";
        }
    }
    char sendBuffer[1024] = {0};
    strcpy(sendBuffer, topics.c_str());
    std::cout << topics << std::endl;
    send(_socket, sendBuffer, strlen(sendBuffer), 0);
}



