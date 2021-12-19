#ifndef THREAD_H
#define THREAD_H

#include<QThread>

class Thread : public QThread
{
    Q_OBJECT
public:
    explicit Thread(QObject *parent = 0, int socket = 0, std::string topic = "");
    bool stopped;
    std::string topic; int socket;
    void run();
signals:
    void NewMessage(QString, QString);
};

#endif // THREAD_H
