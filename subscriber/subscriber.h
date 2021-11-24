#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class subscriber; }
QT_END_NAMESPACE

class subscriber : public QMainWindow
{
    Q_OBJECT

public:
    subscriber(QWidget *parent = nullptr);
    ~subscriber();

private:
    Ui::subscriber *ui;
};
#endif // SUBSCRIBER_H
