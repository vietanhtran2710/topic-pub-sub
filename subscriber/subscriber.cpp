#include "subscriber.h"
#include "./ui_subscriber.h"

subscriber::subscriber(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::subscriber)
{
    ui->setupUi(this);
}

subscriber::~subscriber()
{
    delete ui;
}

