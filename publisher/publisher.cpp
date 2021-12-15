#include "publisher.h"
#include "./ui_publisher.h"
#include <QTimer>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <QMessageBox>
#include <QTimer>
#include <QDateTime>
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
    ui->pauseButton->setDisabled(true);
    ui->stopButton->setDisabled(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeaderItem(0)->setText("data");
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
    ui->checkBox_2->setChecked(true);
    ui->tableWidget->setItem(0, 0, new QTableWidgetItem("0"));
}

publisher::~publisher()
{
    delete ui;
}

void publisher::closeEvent(QCloseEvent *event) {
    if (!thread->stopped) ui->stopButton->click();
    thread->stopped = true;
    thread->exit();
    thread->terminate();
    Json::Value obj;
    obj["command"] = "QUIT";
    Json::StyledWriter styledWriter; std::string jsonString = styledWriter.write(obj);
    char json[1024] = {0};
    strcpy(json, jsonString.c_str());
    send(sock, json, sizeof(json), 0);
    ::close(sock); event->accept();
}

void publisher::sectionClicked(int index) {
    QString oldName = ui->tableWidget->verticalHeaderItem(index)->text();
    bool ok;
    QString text = QInputDialog::getText(this, tr("Change field name"),
                                         tr("Field name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        ui->tableWidget->verticalHeaderItem(index)->setText(text);
        QString oldValue = publisher::thread->customData[oldName];
        publisher::thread->customData.erase(oldName);
        publisher::thread->customData[text] = oldValue;
    }
}

void publisher::onSendMessage(QString message) {
    this->ui->label_3->setText(message);
}

void publisher::on_startButton_clicked()
{
    ui->pauseButton->setDisabled(false);
    ui->stopButton->setDisabled(false);
    ui->groupBox_2->setDisabled(true);
    ui->onceButton->setDisabled(true);
    ui->startButton->setDisabled(true);
    ui->lineEdit->setDisabled(true);
    thread->stopped = thread->paused = false;
    thread->topic = ui->lineEdit->text().toStdString();
    if (ui->checkBox->isChecked()) thread->flag = "retain";
    else thread->flag = "";
    publisher::thread->start();
}

void publisher::on_stopButton_clicked()
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
    ui->groupBox_2->setDisabled(false);
    ui->startButton->setDisabled(false);
    ui->onceButton->setDisabled(false);
    ui->lineEdit->setDisabled(false);
    ui->stopButton->setDisabled(true);
    ui->pauseButton->setDisabled(true);
}

void publisher::on_deleteFieldButton_clicked()
{
    publisher::thread->customData.erase(ui->tableWidget->verticalHeaderItem(ui->tableWidget->currentRow())->text());
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
}

void publisher::on_newFieldButton_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Create new data field"),
                                         tr("Field name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        int rowPos = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow( ui->tableWidget->rowCount() );
        ui->tableWidget->setVerticalHeaderItem(rowPos, new QTableWidgetItem(text));
        publisher::thread->customData[ui->tableWidget->verticalHeaderItem(rowPos)->text()] = "";
    }
}

void publisher::on_pauseButton_clicked()
{
    thread->paused = true;
    ui->startButton->setDisabled(false);
    ui->onceButton->setDisabled(false);
    ui->groupBox_2->setDisabled(false);
    ui->pauseButton->setDisabled(true);
}

void publisher::on_checkBox_stateChanged(int arg1)
{
    ui->tableWidget->setDisabled(ui->checkBox_2->isChecked());
    ui->newFieldButton->setDisabled(ui->checkBox_2->isChecked());
    ui->deleteFieldButton->setDisabled(ui->checkBox_2->isChecked());
    publisher::thread->automaticData = ui->checkBox_2->isChecked();
}

void publisher::on_tableWidget_cellChanged(int row, int column)
{
    publisher::thread->customData[ui->tableWidget->verticalHeaderItem(row)->text()] = ui->tableWidget->item(row, 0)->text();
}

void publisher::on_onceButton_clicked()
{
    ui->startButton->setDisabled(false);
    if (ui->checkBox->isChecked()) thread->flag = "retain";
    else thread->flag = "";
    if (thread->topicRegistered && thread->topic != ui->lineEdit->text().toStdString()) {
        thread->stopped = true;
        Json::Value obj;
        obj["command"] = "STOP PUBLISHING";
        obj["topic"] = thread->topic;
        Json::StyledWriter styledWriter;
        std::string jsonString = styledWriter.write(obj);
        char json[1024];
        memset(json, 0, 1024);
        strcpy(json, jsonString.c_str());
        send(sock, json, sizeof(json), 0);
        thread->topicRegistered = false;
    }
    thread->topic = ui->lineEdit->text().toStdString();
    if (!thread->topicRegistered) {
        Json::Value obj;
        obj["command"] = "START PUBLISHING";
        obj["topic"] = thread->topic;
        Json::StyledWriter styledWriter;
        std::string jsonString = styledWriter.write(obj);
        char json[1024];
        memset(json, 0, 1024);
        strcpy(json, jsonString.c_str());
        send(sock, json, sizeof(json), 0);
        thread->topicRegistered = true;
    }
    Json::Value jdata, obj;
    jdata["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    if (thread->automaticData) {
        jdata["value"] = rand() % 1000;
    }
    else {
        for(std::map<QString, QString>::iterator iter = thread->customData.begin(); iter != thread->customData.end(); ++iter) {
            QString key =  iter->first;
            QString value = iter->second;
            jdata[key.toStdString()] = value.toStdString();
        }
    }
    obj["command"] = "PUBLISH";
    obj["topic"] = thread->topic;
    obj["data"] = jdata;
    obj["flag"] = thread->flag;
    Json::StyledWriter styledWriter;
    std::string jsonString = styledWriter.write(obj);
    char json[1024];
    memset(json, 0, 1024);
    strcpy(json, jsonString.c_str());
    send(sock, json, sizeof(json), 0);
    this->ui->label_3->setText(QString::fromStdString(jsonString));
}
