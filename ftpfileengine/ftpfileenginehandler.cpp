#include <QUrl>

#include "ftpfileengine.h"

#include "ftpfileenginehandler.h"

QAbstractFileEngine*
FtpFileEngineHandler::create(const QString &fileName) const
{
    return QUrl(fileName).scheme() == "ftp" ? new FtpFileEngine(fileName) : 0;
}
