#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <QMainWindow>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "thread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class publisher; }
QT_END_NAMESPACE

class publisher : public QMainWindow
{
    Q_OBJECT
public slots:
    void onSendMessage(QString result);

public:
    publisher(QWidget *parent = nullptr);
    ~publisher();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::publisher *ui;
    int timestamp;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    Thread *thread;
};
#endif // PUBLISHER_H
