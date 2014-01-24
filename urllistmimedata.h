/****************************************************************************
**
** UrlListMimeData, a class to share url lists via clipboard
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

#ifndef URLLISTMIMEDATA_H
#define URLLISTMIMEDATA_H

#include <QMimeData>
#include <QUrl>

class UrlListMimeData : public QMimeData
{
    Q_OBJECT
public:
    enum Action { CopyAction = 1, CutAction = 2 };

    explicit UrlListMimeData(Action action = CopyAction);

    QStringList formats() const;

    void setList(const QList<QUrl>& urlList);

    static QList<QUrl> listFrom(const QMimeData *mimeData,
                                Action action = CopyAction);

    static QString format(Action action = CopyAction);

private:
    Action _action;
};

#endif // URLLISTMIMEDATA_H
