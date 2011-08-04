#include <QtGui/QApplication>
#include "remotecv.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RemoteCV w;
    w.show();

    return a.exec();
}
