#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <map>
#include "thread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class server; }
QT_END_NAMESPACE

class server : public QMainWindow
{
    Q_OBJECT
public slots:
    void onNewClient(int socket);

    void onNewMessage(QString topicName, QString message, QString retainFlag);

    void onNewSubscriber(QString topicName, int socket);

public:
    server(QWidget *parent = nullptr);
    ~server();
    void closeEvent(QCloseEvent*);

private slots:


private:
    Ui::server *ui;
    std::map<QString, QString> retainedMessage;
    std::map<QString, std::vector<int>*> topicSubscriber;
    std::vector<Thread*> *threads;
    Thread* acceptThread;

};
#endif // SERVER_H
