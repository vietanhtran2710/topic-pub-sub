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
#include <jsoncpp/json/json.h>
#include <QCloseEvent>
#include <QDir>
#include <QInputDialog>

#define PORT 8080

publisher::publisher(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::publisher)
{
    ui->setupUi(this);
    ui->pushButton_5->setDisabled(true);
    ui->pushButton_2->setDisabled(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect( ui->tableWidget->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sectionClicked(int)));
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

void publisher::closeEvent(QCloseEvent *event) {
    thread->stopped = true;
    Json::Value obj;
    obj["command"] = "QUIT";
    Json::StyledWriter styledWriter; std::string jsonString = styledWriter.write(obj);
    char json[1024] = {0};
    strcpy(json, jsonString.c_str());
    send(sock, json, sizeof(json), 0);
    ::close(sock); event->accept();
}

void publisher::sectionClicked(int index) {
    std::cout << index << " " << ui->tableWidget->rowCount() << std::endl;
    bool ok;
    QString text = QInputDialog::getText(this, tr("Change field name"),
                                         tr("Field name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty())
        ui->tableWidget->verticalHeaderItem(index)->setText(text);
}

void publisher::onSendMessage(QString message) {
    this->ui->label_3->setText(message);
}

void publisher::on_pushButton_clicked()
{
    ui->pushButton_5->setDisabled(false);
    ui->pushButton_2->setDisabled(false);
    ui->pushButton_6->setDisabled(true);
    ui->pushButton->setDisabled(true);
    ui->lineEdit->setDisabled(true);
    thread->stopped = thread->paused = false;
    thread->topic = ui->lineEdit->text().toStdString();
    if (ui->checkBox->isChecked()) thread->flag = "retain";
    else thread->flag = "";
    publisher::thread->start();
}

void publisher::on_pushButton_2_clicked()
{
    thread->stopped = true;
    Json::Value obj;
    obj["command"] = "STOP PUBLISHING";
    obj["topic"] = ui->lineEdit->text().toStdString();
    Json::StyledWriter styledWriter;
    std::string jsonString = styledWriter.write(obj);
    char json[1024];
    memset(json, 0, 1024);
    strcpy(json, jsonString.c_str());
    send(sock, json, sizeof(json), 0);
    thread->topicRegistered = false;
    ui->pushButton->setDisabled(false);
    ui->pushButton_6->setDisabled(false);
    ui->lineEdit->setDisabled(false);
}

void publisher::on_pushButton_4_clicked()
{
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
}

void publisher::on_pushButton_3_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Create new data field"),
                                         tr("Field name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        int rowPos = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow( ui->tableWidget->rowCount() );
        ui->tableWidget->setVerticalHeaderItem(rowPos, new QTableWidgetItem(text));
    }
}

void publisher::on_pushButton_5_clicked()
{
    thread->paused = true;
    ui->pushButton->setDisabled(false);
    ui->pushButton_6->setDisabled(false);
}
