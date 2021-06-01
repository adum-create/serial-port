#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SerialPort.h"

class SerialPort : public QMainWindow
{
    Q_OBJECT

public:
    SerialPort(QWidget *parent = Q_NULLPTR);

private:
    Ui::SerialPortClass ui;
};
