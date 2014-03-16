/****************************************************************************
**
** FileOperation, class for file operations
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

#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QObject>
#include <QString>
#include <QFile>

class FileOperation : public QObject
{
    Q_OBJECT

public:
    FileOperation(const QString& source = QString(),
                  const QString& dest = QString(),
                  QObject* parent = 0);

    ~FileOperation();

    const QString& source() const { return _source; }
    const QString& dest() const { return _dest; }

    void setSource(const QString& source) { _source = source; }
    void setDest(const QString& dest) { _dest = dest; }

    enum { DefaultChunkSize = 1024 * 4 };

    bool open();
    void close();

    void abort();

    qint64 size() const;

    qint64 copy(qint64 chunkSize = DefaultChunkSize);
    bool remove();
    bool rename(const QString& newName);
    bool mkdir();
    bool rmdir();

private:
    QString _source;
    QString _dest;

    QFile _sourceFile;
    QFile _destFile;
};

#endif // FILEOPERATION_H
