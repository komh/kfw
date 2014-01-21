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
