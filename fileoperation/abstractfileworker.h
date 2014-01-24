/****************************************************************************
**
** AbstractFileWorker, the abstract class for file operation workers
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

#ifndef ABSTRACTFILEWORKER_H
#define ABSTRACTFILEWORKER_H

#include <QObject>

class AbstractFileWorker : public QObject
{
    Q_OBJECT
public:
    explicit AbstractFileWorker(const QString& source, const QString& dest,
                                QObject *parent = 0);

    virtual ~AbstractFileWorker();

    void cancel() { _canceled = true; }
    bool wasCanceled() const { return _canceled; }

    const QString& source() const { return _source; }
    const QString& dest() const { return _dest; }

    bool result() const { return _result; }

signals:
    void valueChanged(int value);

public slots:
    void perform();

protected:
    virtual void performWork() = 0;

    void setResult(bool result) { _result = result; }

private:
    QString _source;
    QString _dest;

    bool _canceled;
    bool _result;
};

#endif // ABSTRACTFILEWORKER_H
