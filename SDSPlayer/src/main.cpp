#include "sdsplayer.h"

#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SDSPlayer w;
    w.show();
    return a.exec();
}
