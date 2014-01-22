#include "fileoperation.h"

#include "renamefileworker.h"

RenameFileWorker::RenameFileWorker(const QString& source, const QString& dest,
                                   QObject *parent) :
    AbstractFileWorker(source, dest, parent)
{

}

void RenameFileWorker::performWork()
{
    FileOperation fileOp(source());

    setResult(fileOp.open() && fileOp.rename(dest()));
}
