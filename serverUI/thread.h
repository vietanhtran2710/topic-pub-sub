#include<QThread>

#ifndef THREAD_H
#define THREAD_H


class Thread : public QThread
{
    Q_OBJECT
public:
    explicit Thread(QObject *parent = 0, int type = 0, int socket = 0);
    void run();
    bool stopped;
private:
    int type, connSocket;
signals:
    void NewClient(int);

    void NewMessage(QString, QString, QString);

    void NewSubscriber(QString, int);

    void NewTopic(QString);

    void QuitTopic(QString);

    void NodeQuit();
};

#endif // THREAD_H
