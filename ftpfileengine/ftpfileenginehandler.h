#ifndef FTPFILEENGINEHANDLER_H
#define FTPFILEENGINEHANDLER_H

#include <QAbstractFileEngineHandler>

class FtpFileEngineHandler : public QAbstractFileEngineHandler
{
public:
    QAbstractFileEngine* create(const QString &fileName) const;
};
#endif // FTPFILEENGINEHANDLER_H
