#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QPixmap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onDataReceived(const QByteArray& data)
{
    QPixmap image;
    if (image.loadFromData(data))
    {
        ui->centralwidget->setPixmap(image);
    }
}
