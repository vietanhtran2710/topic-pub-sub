#include<QThread>
#include<map>

#ifndef THREAD_H
#define THREAD_H

class Thread : public QThread
{
    Q_OBJECT
public:
    explicit Thread(QObject *parent = 0, int socket = 0, std::string topic = "", std::string flag = "");
    int socket, timestamp; std::string topic, flag;
    void run();
    bool stopped, paused, topicRegistered, automaticData;
    std::map<QString, QString> customData;
signals:
    void SendMessage(QString);
public slots:
};
#endif // THREAD_H
