#include "subscriber.h"
#include "./ui_subscriber.h"
#include <QMessageBox>
#include <QTimer>
#include <QCloseEvent>
#include <QStringListModel>
#include "thread.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <map>

#define PORT 8080

subscriber::subscriber(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::subscriber)
{
    ui->setupUi(this);
    availableTopicsModel = new QStringListModel(this);
    ui->availableTopicsList->setModel(availableTopicsModel);
    ui->availableTopicsList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    receivedTopicsModel = new QStringListModel(this);
    ui->receivedTopicsList->setModel(receivedTopicsModel);
    ui->receivedTopicsList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->stopButton->setDisabled(true);
    subscriber::currentTopic = "";

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        QMessageBox *alert = new QMessageBox(
            QMessageBox::Warning,
            "Error",
            "Invalid address or Address not supported."
        );
        alert->show();
        QTimer::singleShot(0, this, SLOT(close()));
    }

    if (::connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        QMessageBox *alert = new QMessageBox(
            QMessageBox::Warning,
            "Error",
            "Connection Failed."
        );
        alert->show();
        QTimer::singleShot(0, this, SLOT(close()));
    }
    subscriber::thread = new Thread(this, sock, "");
    connect(thread, SIGNAL(NewMessage(QString, QString)), this, SLOT(onNewMessage(QString, QString)));
    Json::Value obj;
    obj["command"] = "CONNECT";
    Json::StyledWriter styledWriter; std::string jsonString = styledWriter.write(obj);
    char json[1024] = {0};
    strcpy(json, jsonString.c_str());
    send(sock, json, sizeof(json), 0);
    char buffer[1024] = {0};
    recv(sock, buffer, sizeof(buffer), 0);
    std::cout << buffer << std::endl;
    if (strncmp(buffer, "{\n", 2)==0)
    {
        Json::Value data; Json::Reader read;
        std::string json(buffer);
        read.parse(json, data);
        if (data["status"] == "CONNECTACK") {
            ui->refreshButton->click();
        }
    }
    subscriber::displaying = false;
}

subscriber::~subscriber()
{
    delete ui;
}

void subscriber::closeEvent(QCloseEvent *event) {
    if (!thread->stopped) ui->stopButton->click();
    subscriber::thread->stopped = true;
    thread->stopped = true;
    thread->exit();
    thread->terminate();
    Json::Value obj;
    obj["command"] = "QUIT";
    Json::StyledWriter styledWriter; std::string jsonString = styledWriter.write(obj);
    char json[1024] = {0};
    strcpy(json, jsonString.c_str());
    send(sock, json, sizeof(json), 0);
    subscriber::thread->exit();
    ::close(sock); event->accept();
}

void subscriber::onNewMessage(QString topic, QString message) {
    topic.remove(0, 1); topic.remove(topic.length() - 2, 1);
    message.remove(0, 1); message.remove(message.length() - 2, 1);
    if (receivedMessages.find(topic) == receivedMessages.end()) {
        subscriber::receivedTopicsModel->insertRow(receivedTopicsModel->rowCount());
        std::cout << receivedTopicsModel->rowCount() << std::endl;
        QModelIndex index = receivedTopicsModel->index(receivedTopicsModel->rowCount()-1);
        subscriber::receivedTopicsModel->setData(index, topic);
    }
    subscriber::receivedMessages[topic] = message;
    if (topic == selectedTopic) {
        while(displaying);
        displaying = true;
        ui->label_4->setText(message.replace("\\n","\n").replace("\\\"", "\""));
        displaying = false;
    }
}

void subscriber::on_startButton_clicked()
{
    if (ui->lineEdit->text() != "") {
        subscriber::currentTopic = ui->lineEdit->text().toStdString();
    }
    else {
        QString newTopic = ui->availableTopicsList->currentIndex().data(Qt::DisplayRole).toString();
        if (newTopic != "") {
            subscriber::currentTopic = newTopic.toStdString();
        }
    }
    subscriber::thread->topic = subscriber::currentTopic;
    subscriber::thread->stopped = false;
    ui->refreshButton->setDisabled(true);
    ui->startButton->setDisabled(true);
    ui->stopButton->setDisabled(false);
    subscriber::thread->start();
}

void subscriber::on_stopButton_clicked()
{
    subscriber::thread->stopped = true;
    subscriber::thread->terminate();
    Json::Value obj;
    obj["command"] = "STOP SUBSCRIBING";
    obj["topic"] = subscriber::currentTopic;
    subscriber::currentTopic = "";
    Json::StyledWriter styledWriter; std::string jsonString = styledWriter.write(obj);
    char json[1024] = {0};
    strcpy(json, jsonString.c_str());
    send(sock, json, sizeof(json), 0);
    ui->startButton->setDisabled(false);
    ui->refreshButton->setDisabled(false);
    ui->stopButton->setDisabled(true);
}

void subscriber::on_refreshButton_clicked()
{
    Json::Value obj;
    obj["command"] = "GET ALL TOPICS";
    Json::StyledWriter styledWriter; std::string jsonString = styledWriter.write(obj);
    char json[1024] = {0};
    strcpy(json, jsonString.c_str());
    send(sock, json, sizeof(json), 0);
    char buffer[1024] = {0};
    recv(sock, buffer, sizeof(buffer), 0);
    std::cout << buffer << std::endl;
    std::string str(buffer);
    QString result = QString::fromStdString(str);
    QStringList topics = result.split(";");
    subscriber::availableTopicsModel->setStringList(topics);
}

void subscriber::on_receivedTopicsList_clicked(const QModelIndex &index)
{
    QModelIndex _index = ui->receivedTopicsList->currentIndex();
    subscriber::selectedTopic = _index.data(Qt::DisplayRole).toString();
    while(displaying);
    displaying = true;
    ui->label_4->setText(subscriber::receivedMessages[selectedTopic]);
    displaying = false;
}
