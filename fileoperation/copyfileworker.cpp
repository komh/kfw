/****************************************************************************
**
** CopyFileWorker, worker for a file copy operation
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

#include <QtGui>

#include "fileoperation.h"
#include "pathcomp.h"

#include "copyfileworker.h"

CopyFileWorker::CopyFileWorker(const QString &source, const QString &dest,
                               QObject *parent)
    : AbstractFileWorker(source, dest, parent)
{
}

void CopyFileWorker::performWork()
{
    setResult(false);

    if (QFileInfo(source()).isDir())
    {
        FileOperation fileOp(dest());

        if (QDir(dest()).exists() || fileOp.mkdir())
        {
            emit valueChanged(100);
            setResult(true);
        }

        return;
    }

    FileOperation fileOp(source(), dest());

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

    if (wasCanceled())
        fileOp.abort();
}
