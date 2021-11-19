#include "publisher.h"
#include "./ui_publisher.h"
#include <QTimer>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <QMessageBox>
#include <QTimer>
#include <iostream>

#define PORT 8080

publisher::publisher(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::publisher)
{
    ui->setupUi(this);
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        QMessageBox *alert = new QMessageBox(
            QMessageBox::Warning,
            "Error",
            "Invalid address or Address not supported."
        );
        alert->show();
        QTimer::singleShot(0, this, SLOT(close()));
    }

    if (::connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        QMessageBox *alert = new QMessageBox(
            QMessageBox::Warning,
            "Error",
            "Connection Failed."
        );
        alert->show();
        QTimer::singleShot(0, this, SLOT(close()));
    }
    publisher::thread = new Thread(this, sock, "", "");
    connect(thread, SIGNAL(SendMessage(QString)), this, SLOT(onSendMessage(QString)));
}

publisher::~publisher()
{
    delete ui;
}

void publisher::onSendMessage(QString message) {
    this->ui->label_3->setText(message);
}

void publisher::on_pushButton_clicked()
{
    thread->topic = ui->lineEdit->text().toStdString();
    if (ui->checkBox->isChecked()) thread->flag = "retain";
    else thread->flag = "";
    publisher::thread->start();
}

void publisher::on_pushButton_2_clicked()
{
    thread->stopped = true;
}
