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

#include <QStringList>
#include <QUrl>

#include "urllistmimedata.h"

UrlListMimeData::UrlListMimeData(Action action, QObject *parent) :
    QMimeData()
  , _action(action)
{
}

QStringList UrlListMimeData::formats() const
{
    return QStringList() << format(_action);
}

void UrlListMimeData::setList(const QList<QUrl> &urlList)
{
    QStringList list;

    foreach(QUrl url, urlList)
        list << url.toString();

    setData(format(_action), list.join("\n").toUtf8());
}

QList<QUrl> UrlListMimeData::listFrom(const QMimeData *mimeData, Action action)
{
    QString oneLineList(QString::fromUtf8(mimeData->data(format(action))));

    QStringList list(oneLineList.split("\n"));

    QList<QUrl> urlList;

    foreach(QString entry, list)
        urlList << entry;

    return urlList;
}

QString UrlListMimeData::format(Action action)
{
    return action == CopyAction ? "text/kfw-copy-url-list" :
                                  "text/kfw-cut-url-list";
}
