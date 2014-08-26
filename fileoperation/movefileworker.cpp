/****************************************************************************
**
** MoveFileWorker, worker for a file move
** Copyright (C) 2014 by KO Myung-Hun
** All rights reserved.
** Contact: KO Myung-Hun (komh@chollian.net)
**
** This file is part of K File Wizard.
**
** $BEGIN_LICENSE$
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** $END_LICENSE$
**
****************************************************************************/

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

    if (fileOp.rename(dest()))
    {
        qDebug() << "MoveFileWorker: Rename succeeded";

        emit valueChanged(100);

        setResult(true);

        return;
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
            fileOp.close();

            if (!fileOp.remove())
                return;

            setResult(true);
        }
    }

    if (wasCanceled())
        fileOp.abort();
}
