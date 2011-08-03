#include <QtGui/QApplication>
#include "remotecv.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    remoteCV w;
    w.show();

    return a.exec();
}
