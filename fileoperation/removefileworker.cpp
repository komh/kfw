#include "fileoperation.h"

#include "removefileworker.h"

RemoveFileWorker::RemoveFileWorker(const QString& source, QObject *parent) :
    AbstractFileWorker(source, QString(), parent)
{
}

void RemoveFileWorker::performWork()
{
    FileOperation fileOp(source());

    setResult(fileOp.open() && fileOp.remove());
}
