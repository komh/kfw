#include "kfilewizard.h"

#include <QApplication>

#include "ftpfileengine/ftpfileenginehandler.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    FtpFileEngineHandler ftpHandler;

    KFileWizard w;
    w.show();

    return a.exec();
}
