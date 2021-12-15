#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <QMainWindow>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <QStringListModel>
#include <unistd.h>
#include "thread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class subscriber; }
QT_END_NAMESPACE

class subscriber : public QMainWindow
{
    Q_OBJECT
public slots:
    void onNewMessage(QString message);

public:
    subscriber(QWidget *parent = nullptr);
    ~subscriber();
    void closeEvent(QCloseEvent *event);

private slots:
    void on_startButton_clicked();

    void on_stopButton_clicked();

    void on_refreshButton_clicked();

private:
    Ui::subscriber *ui;
    int sock = 0;
    struct sockaddr_in serv_addr;
    bool displaying;
    std::string currentTopic;
    Thread *thread;
    QStringListModel *model;
};
#endif // SUBSCRIBER_H
