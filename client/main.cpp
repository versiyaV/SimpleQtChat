#include "mainwindow.h"
#include "myclient.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyClient client;
    client.show();
    return a.exec();
}
