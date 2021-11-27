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
    void closeEvent(QCloseEvent *event);

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void sectionClicked(int);

    void on_pushButton_4_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_5_clicked();

    void on_checkBox_2_stateChanged(int arg1);

    void on_tableWidget_cellChanged(int row, int column);

private:
    Ui::publisher *ui;
    int timestamp;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    Thread *thread;
};
#endif // PUBLISHER_H
