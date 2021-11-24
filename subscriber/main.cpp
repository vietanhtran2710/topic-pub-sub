#include "subscriber.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    subscriber w;
    w.show();
    return a.exec();
}
