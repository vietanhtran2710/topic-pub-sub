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
            std::cout << topicSubscriber[topicName]->size() << std::endl;
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
    std::cout << "Got new subscriber" << std::endl;
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
    std::cout << "Got new topic" << std::endl;
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
    topicPublishers[topicName]--;
    QStandardItem *findResult = model->findItems(topicName)[0];
    if (topicPublishers[topicName] == 0) {
        model->removeRow(findResult->row());
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

void server::onSubscriberQuit(int socket) {

}



