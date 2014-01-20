#include <QtGui>

#include "fileoperation.h"

#include "copyfileworker.h"

CopyFileWorker::CopyFileWorker(const QString &source, const QString &dest,
                               QObject *parent)
    : AbstractFileWorker(source, dest, parent)
{
}

void CopyFileWorker::performWork()
{
    FileOperation fileOp(source(), dest());

    setResult(false);

    if (fileOp.open())
    {
        qint64 copied = 0;
        qint64 totalCopied = 0;
        qint64 totalSize = fileOp.size();

        while (!wasCanceled() && totalCopied < totalSize)
        {
            copied = fileOp.copy();

            if (copied == -1)
                break;

            totalCopied += copied;

            emit valueChanged(totalCopied * 100 / totalSize);
        }

        setResult(copied != -1);
    }
}
