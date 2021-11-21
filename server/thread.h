#include<QThread>

#ifndef THREAD_H
#define THREAD_H

class Thread : public QThread
{
    Q_OBJECT
public:
    explicit Thread(QObject *parent = 0, int _socket = 0);
    int socket;
    void run();
    bool stopped;
signals:
    void SendMessage(QString);
public slots:
};
#endif // THREAD_H
