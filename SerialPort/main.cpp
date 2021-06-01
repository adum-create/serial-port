#include "SerialPort.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SerialPort w;
    w.show();
    return a.exec();
}
