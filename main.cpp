#include "kfilewizard.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    KFileWizard w;
    w.show();

    return a.exec();
}
