#include "subscriber.h"
#include "./ui_subscriber.h"
#include <QMessageBox>
#include <QTimer>
#include <QCloseEvent>
#include "thread.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <jsoncpp/json/json.h>
#include <iostream>

#define PORT 8080

subscriber::subscriber(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::subscriber)
{
    ui->setupUi(this);

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
    connect(thread, SIGNAL(NewMessage(QString)), this, SLOT(onNewMessage(QString)));
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
//            QMessageBox *alert = new QMessageBox(
//                QMessageBox::Information,
//                "CONNECTION SUCCESS",
//                "CONNECTACK received"
//            );
//            alert->show();
        }
    }
}

subscriber::~subscriber()
{
    delete ui;
}

void subscriber::closeEvent(QCloseEvent *event) {
    subscriber::thread->stopped = true;
    Json::Value obj;
    obj["command"] = "QUIT";
    Json::StyledWriter styledWriter; std::string jsonString = styledWriter.write(obj);
    char json[1024] = {0};
    strcpy(json, jsonString.c_str());
    send(sock, json, sizeof(json), 0);
    subscriber::thread->exit();
    ::close(sock); event->accept();
}

void subscriber::onNewMessage(QString message) {
    ui->label_4->setText(message);
}

void subscriber::on_pushButton_clicked()
{
    if (ui->lineEdit->text() != "") {
        subscriber::thread->topic = ui->lineEdit->text().toStdString();
        subscriber::thread->start();
    }
}

void subscriber::on_pushButton_2_clicked()
{
    subscriber::thread->stopped = true;
    subscriber::thread->exit();
}

void subscriber::on_pushButton_3_clicked()
{

}
