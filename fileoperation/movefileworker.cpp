#include <QDebug>

#include "fileoperation.h"

#include "movefileworker.h"

MoveFileWorker::MoveFileWorker(const QString &source, const QString &dest,
                               QObject *parent) :
    AbstractFileWorker(source, dest, parent)
{
}

void MoveFileWorker::performWork()
{
    setResult(false);

    // try rename first
    FileOperation fileOp(source());

    if (fileOp.open())
    {
        if (fileOp.rename(dest()))
        {
            qDebug() << "MoveFileWorker: Rename succeeded";

            emit valueChanged(100);

            setResult(true);

            return;
        }

        fileOp.close();
    }

    qDebug() << "MoveFileWorker: Rename failed, try copy and remove";

    // try copy and remove

    fileOp.setDest(dest());

    if (fileOp.open())
    {
        qint64 copied = 0;
        qint64 totalCopied = 0;
        qint64 totalSize = fileOp.size();

        while (!wasCanceled() && totalCopied < totalSize)
        {
            copied = fileOp.copy();

            if (copied == -1)
                return;

            totalCopied += copied;

            emit valueChanged(totalCopied * 100 / totalSize);
        }

        if (!wasCanceled())
        {
            if (!fileOp.remove())
                return;

            setResult(true);
        }
    }

    if (wasCanceled())
        fileOp.abort();
}
