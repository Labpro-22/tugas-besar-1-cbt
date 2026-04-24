#include "LogPopupDemo.h"

LogPopupDemo::LogPopupDemo(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}

LogPopupDemo::~LogPopupDemo()
{
    delete ui;
}

void LogPopupDemo::on_pushButton_clicked()
{
    QTime time = QTime::currentTime();
    QString timeString = time.toString("hh:mm:ss");
    ui->textEdit->appendPlainText(timeString);
}